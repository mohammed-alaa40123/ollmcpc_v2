// =============================================================================
// JSON Builder and Parser Implementation
// =============================================================================
//
// Lightweight JSON utilities using simple string manipulation.
// NOT a full JSON parser - designed for speed and simplicity.
//
// =============================================================================

#include "utils/json.hpp"
#include <sstream>
#include <cctype>

namespace json {

// =============================================================================
// JSON Building Functions
// =============================================================================

std::string escape(const std::string& s) {
    std::string result;
    result.reserve(s.length() + 16); // Pre-allocate for efficiency
    for (char c : s) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:   result += c;
        }
    }
    return result;
}

std::string str(const std::string& s) {
    return "\"" + escape(s) + "\"";
}

std::string num(int n) {
    return std::to_string(n);
}

std::string boolean(bool b) {
    return b ? "true" : "false";
}

std::string obj(const std::map<std::string, std::string>& kvs) {
    std::string result = "{";
    bool first = true;
    for (const auto& kv : kvs) {
        if (!first) result += ",";
        result += "\"" + kv.first + "\":" + kv.second;
        first = false;
    }
    result += "}";
    return result;
}

std::string arr(const std::vector<std::string>& items) {
    std::string result = "[";
    for (size_t i = 0; i < items.size(); i++) {
        if (i > 0) result += ",";
        result += items[i];
    }
    result += "]";
    return result;
}

std::string sanitize(const std::string& s) {
    // Fix malformed JSON from external MCP servers
    // 1. Remove extra braces like "(default: 5000})" in descriptions
    // 2. Remove "$schema" field that Gemini API rejects
    
    std::string result;
    result.reserve(s.length());
    bool in_string = false;
    
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        
        // Track string boundaries (handle escaped quotes)
        if (c == '"' && (i == 0 || s[i-1] != '\\')) {
            in_string = !in_string;
        }
        
        // Remove errant braces inside strings that would break JSON
        if (in_string && c == '}') {
            if (i + 1 < s.length() && s[i+1] == ')') {
                continue;
            }
        }
        
        result += c;
    }
    
    // Remove "$schema" field (not supported by Gemini API)
    // Pattern: "$schema":"..." or "$schema": "..."
    std::string schema_pattern = "\"$schema\":";
    size_t pos;
    while ((pos = result.find(schema_pattern)) != std::string::npos) {
        // Find the end of the value
        size_t val_start = pos + schema_pattern.length();
        while (val_start < result.length() && result[val_start] == ' ') val_start++;
        
        if (val_start < result.length() && result[val_start] == '"') {
            // String value - find closing quote
            size_t val_end = result.find('"', val_start + 1);
            while (val_end != std::string::npos && result[val_end - 1] == '\\') {
                val_end = result.find('"', val_end + 1);
            }
            if (val_end != std::string::npos) {
                val_end++; // Include closing quote
                // Check if followed by comma
                if (val_end < result.length() && result[val_end] == ',') {
                    val_end++;
                }
                result.erase(pos, val_end - pos);
            }
        }
    }
    
    // Remove "additionalProperties" field (not supported by Gemini API)
    // Pattern: "additionalProperties":false or "additionalProperties":true
    std::string addprops_pattern = "\"additionalProperties\":";
    while ((pos = result.find(addprops_pattern)) != std::string::npos) {
        size_t val_start = pos + addprops_pattern.length();
        // Skip the boolean value (true or false)
        size_t val_end = val_start;
        while (val_end < result.length() && result[val_end] != ',' && result[val_end] != '}') {
            val_end++;
        }
        // Check if followed by comma
        if (val_end < result.length() && result[val_end] == ',') {
            val_end++;
        }
        // Check for preceding comma (if this was last field)
        size_t erase_start = pos;
        if (pos > 0 && result[pos - 1] == ',') {
            erase_start = pos - 1;
        }
        result.erase(erase_start, val_end - erase_start);
    }
    
    // Clean up any trailing commas before } or ]
    pos = 0;
    while ((pos = result.find(",}", pos)) != std::string::npos) {
        result.erase(pos, 1);
    }
    pos = 0;
    while ((pos = result.find(",]", pos)) != std::string::npos) {
        result.erase(pos, 1);
    }
    
    return result;
}

// =============================================================================
// JSON Parsing Functions
// =============================================================================

namespace parse {

std::string unescape(const std::string& s) {
    std::string result;
    result.reserve(s.length());
    for (size_t i = 0; i < s.length(); ++i) {
        if (s[i] == '\\' && i + 1 < s.length()) {
            switch (s[i + 1]) {
                case 'n':  result += '\n'; i++; break;
                case 'r':  result += '\r'; i++; break;
                case 't':  result += '\t'; i++; break;
                case '"':  result += '"';  i++; break;
                case '\\': result += '\\'; i++; break;
                default:   result += s[i];
            }
        } else {
            result += s[i];
        }
    }
    return result;
}

std::string get_string(const std::string& json, const std::string& key) {
    // Find "key"
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    
    // Skip to colon
    pos += search.length();
    while (pos < json.length() && json[pos] != ':') pos++;
    if (pos >= json.length()) return "";
    pos++; // Skip ':'
    
    // Skip whitespace
    while (pos < json.length() && std::isspace(json[pos])) pos++;
    
    // Must start with quote
    if (pos >= json.length() || json[pos] != '"') return "";
    size_t start = pos + 1;
    
    // Find closing quote (handle escaped quotes)
    size_t end = json.find("\"", start);
    while (end != std::string::npos && json[end - 1] == '\\') {
        end = json.find("\"", end + 1);
    }
    if (end == std::string::npos) return "";
    
    return unescape(json.substr(start, end - start));
}

int get_int(const std::string& json, const std::string& key) {
    // Find "key":
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return 0;
    
    // Skip to colon
    pos += search.length();
    while (pos < json.length() && json[pos] != ':') pos++;
    if (pos >= json.length()) return 0;
    pos++; // Skip ':'
    
    // Skip whitespace
    while (pos < json.length() && std::isspace(json[pos])) pos++;
    
    // Parse number (may have leading minus)
    size_t start = pos;
    if (pos < json.length() && json[pos] == '-') pos++;
    while (pos < json.length() && std::isdigit(json[pos])) pos++;
    
    if (pos == start) return 0;
    return std::stoi(json.substr(start, pos - start));
}

bool get_bool(const std::string& json, const std::string& key) {
    // Find "key":
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return false;
    
    // Skip to colon
    pos += search.length();
    while (pos < json.length() && json[pos] != ':') pos++;
    if (pos >= json.length()) return false;
    pos++; // Skip ':'
    
    // Skip whitespace
    while (pos < json.length() && std::isspace(json[pos])) pos++;
    
    // Check for true/false
    if (json.compare(pos, 4, "true") == 0) return true;
    return false;
}

std::string get_object(const std::string& json, const std::string& key) {
    // Handle special case: key is just "{" meaning find first object
    if (key == "{") {
        return first_object(json);
    }
    
    // Find "key":
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "{}";
    pos += search.length();
    
    // Skip whitespace
    while (pos < json.length() && std::isspace(json[pos])) pos++;
    
    // Must start with {
    if (pos >= json.length() || json[pos] != '{') return "{}";
    
    // Find matching closing brace
    size_t start = pos;
    int depth = 0;
    for (; pos < json.length(); pos++) {
        if (json[pos] == '{') depth++;
        else if (json[pos] == '}') {
            depth--;
            if (depth == 0) break;
        }
    }
    
    return json.substr(start, pos - start + 1);
}

std::string get_array(const std::string& json, const std::string& key) {
    // Find "key":
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    pos += search.length();
    
    // Skip whitespace
    while (pos < json.length() && std::isspace(json[pos])) pos++;
    
    // Must start with [
    if (pos >= json.length() || json[pos] != '[') return "";
    
    // Find matching closing bracket
    size_t start = pos;
    int depth = 0;
    for (; pos < json.length(); pos++) {
        if (json[pos] == '[') depth++;
        else if (json[pos] == ']') {
            depth--;
            if (depth == 0) break;
        }
    }
    
    return json.substr(start, pos - start + 1);
}

std::vector<std::string> get_string_array(const std::string& json, const std::string& key) {
    std::vector<std::string> result;
    std::string array_str = get_array(json, key);
    if (array_str.empty() || array_str == "[]") return result;
    
    size_t pos = 1; // Skip '['
    while (pos < array_str.length() - 1) {
        // Find opening quote
        size_t start = array_str.find("\"", pos);
        if (start == std::string::npos || start >= array_str.length() - 1) break;
        start++; // Skip opening quote
        
        // Find closing quote (handle escapes)
        size_t end = array_str.find("\"", start);
        while (end != std::string::npos && array_str[end - 1] == '\\') {
            end = array_str.find("\"", end + 1);
        }
        if (end == std::string::npos) break;
        
        result.push_back(unescape(array_str.substr(start, end - start)));
        
        // Skip to next element
        pos = end + 1;
        while (pos < array_str.length() && (array_str[pos] == ',' || std::isspace(array_str[pos]))) {
            pos++;
        }
    }
    return result;
}

bool has_key(const std::string& json, const std::string& key) {
    return json.find("\"" + key + "\"") != std::string::npos;
}

std::string first_object(const std::string& json) {
    size_t start = json.find("{");
    if (start == std::string::npos) return "{}";
    
    int depth = 0;
    size_t pos = start;
    for (; pos < json.length(); pos++) {
        if (json[pos] == '{') depth++;
        else if (json[pos] == '}') {
            depth--;
            if (depth == 0) break;
        }
    }
    
    return json.substr(start, pos - start + 1);
}

} // namespace parse
} // namespace json

// =============================================================================
// Legacy compatibility (json_parse namespace)
// =============================================================================

namespace json_parse {

std::string extract_val(const std::string& json, const std::string& key) {
    // This is a special legacy function that extracts raw numeric values
    // Key format is like "\"id\":" 
    size_t pos = json.find(key);
    if (pos == std::string::npos) return "";
    
    size_t start = pos + key.length();
    while (start < json.length() && (json[start] == ' ' || json[start] == ':')) start++;
    
    size_t end = start;
    while (end < json.length() && std::isdigit(json[end])) end++;
    
    return json.substr(start, end - start);
}

} // namespace json_parse
