// =============================================================================
// JSON Builder and Parser - Lightweight Implementation
// =============================================================================

#include "utils/json.hpp"
#include <cctype>
#include <cstring>

namespace json {

// =============================================================================
// Building Functions
// =============================================================================

std::string escape(const std::string& s) {
    std::string r;
    r.reserve(s.length() + 16);
    for (char c : s) {
        switch (c) {
            case '"':  r += "\\\""; break;
            case '\\': r += "\\\\"; break;
            case '\n': r += "\\n";  break;
            case '\r': r += "\\r";  break;
            case '\t': r += "\\t";  break;
            default:   r += c;
        }
    }
    return r;
}

std::string str(const std::string& s) { return "\"" + escape(s) + "\""; }
std::string num(int n) { return std::to_string(n); }

std::string obj(const std::map<std::string, std::string>& kvs) {
    std::string r = "{";
    bool first = true;
    for (const auto& kv : kvs) {
        if (!first) r += ",";
        r += "\"" + kv.first + "\":" + kv.second;
        first = false;
    }
    return r + "}";
}

std::string arr(const std::vector<std::string>& items) {
    std::string r = "[";
    for (size_t i = 0; i < items.size(); i++) {
        if (i > 0) r += ",";
        r += items[i];
    }
    return r + "]";
}

std::string sanitize(const std::string& s) {
    std::string result;
    result.reserve(s.length());
    bool in_string = false;
    
    // Remove errant braces in strings
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (c == '"' && (i == 0 || s[i-1] != '\\')) in_string = !in_string;
        if (in_string && c == '}' && i + 1 < s.length() && s[i+1] == ')') continue;
        result += c;
    }
    
    // Remove unsupported fields ($schema, additionalProperties)
    for (const auto& pattern : {"\"$schema\":", "\"additionalProperties\":"}) {
        size_t pos;
        while ((pos = result.find(pattern)) != std::string::npos) {
            size_t end = pos + strlen(pattern);
            while (end < result.length() && result[end] == ' ') end++;
            
            if (result[end] == '"') {
                end = result.find('"', end + 1);
                while (end != std::string::npos && result[end - 1] == '\\')
                    end = result.find('"', end + 1);
                if (end != std::string::npos) end++;
            } else {
                while (end < result.length() && result[end] != ',' && result[end] != '}') end++;
            }
            
            if (end < result.length() && result[end] == ',') end++;
            size_t start = (pos > 0 && result[pos - 1] == ',') ? pos - 1 : pos;
            result.erase(start, end - start);
        }
    }
    
    // Clean trailing commas
    size_t pos = 0;
    while ((pos = result.find(",}", pos)) != std::string::npos) result.erase(pos, 1);
    pos = 0;
    while ((pos = result.find(",]", pos)) != std::string::npos) result.erase(pos, 1);
    
    return result;
}

// =============================================================================
// Parsing Functions
// =============================================================================

namespace parse {

std::string unescape(const std::string& s) {
    std::string r;
    r.reserve(s.length());
    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] == '\\' && i + 1 < s.length()) {
            char n = s[++i];
            r += (n == 'n' ? '\n' : n == 'r' ? '\r' : n == 't' ? '\t' : n);
        } else {
            r += s[i];
        }
    }
    return r;
}

std::string get_string(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    
    pos += search.length();
    while (pos < json.length() && json[pos] != ':') pos++;
    if (pos >= json.length()) return "";
    pos++;
    while (pos < json.length() && std::isspace(json[pos])) pos++;
    
    if (pos >= json.length() || json[pos] != '"') return "";
    size_t start = pos + 1;
    size_t end = json.find('"', start);
    while (end != std::string::npos && json[end - 1] == '\\')
        end = json.find('"', end + 1);
    
    return (end == std::string::npos) ? "" : unescape(json.substr(start, end - start));
}

std::string get_object(const std::string& json, const std::string& key) {
    if (key == "{") return first_object(json);
    
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "{}";
    pos += search.length();
    while (pos < json.length() && std::isspace(json[pos])) pos++;
    if (pos >= json.length() || json[pos] != '{') return "{}";
    
    size_t start = pos;
    int depth = 0;
    for (; pos < json.length(); pos++) {
        if (json[pos] == '{') depth++;
        else if (json[pos] == '}' && --depth == 0) break;
    }
    return json.substr(start, pos - start + 1);
}

std::string get_array(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    pos += search.length();
    while (pos < json.length() && std::isspace(json[pos])) pos++;
    if (pos >= json.length() || json[pos] != '[') return "";
    
    size_t start = pos;
    int depth = 0;
    for (; pos < json.length(); pos++) {
        if (json[pos] == '[') depth++;
        else if (json[pos] == ']' && --depth == 0) break;
    }
    return json.substr(start, pos - start + 1);
}

std::string first_object(const std::string& json) {
    size_t start = json.find('{');
    if (start == std::string::npos) return "{}";
    
    int depth = 0;
    size_t pos = start;
    for (; pos < json.length(); pos++) {
        if (json[pos] == '{') depth++;
        else if (json[pos] == '}' && --depth == 0) break;
    }
    return json.substr(start, pos - start + 1);
}

bool has_key(const std::string& json, const std::string& key) {
    return json.find("\"" + key + "\"") != std::string::npos;
}

std::string get_raw_value(const std::string& json, const std::string& key_pattern) {
    size_t pos = json.find(key_pattern);
    if (pos == std::string::npos) return "";
    
    size_t start = pos + key_pattern.length();
    while (start < json.length() && (json[start] == ' ' || json[start] == ':')) start++;
    
    size_t end = start;
    while (end < json.length() && std::isdigit(json[end])) end++;
    
    return json.substr(start, end - start);
}

std::vector<std::string> get_string_array(const std::string& json, const std::string& key) {
    std::vector<std::string> result;
    std::string arr = get_array(json, key);
    if (arr.empty() || arr == "[]") return result;
    
    size_t pos = 1;
    while (pos < arr.length() - 1) {
        size_t start = arr.find('"', pos);
        if (start == std::string::npos) break;
        start++;
        
        size_t end = arr.find('"', start);
        while (end != std::string::npos && arr[end - 1] == '\\')
            end = arr.find('"', end + 1);
        if (end == std::string::npos) break;
        
        result.push_back(unescape(arr.substr(start, end - start)));
        pos = end + 1;
        while (pos < arr.length() && (arr[pos] == ',' || std::isspace(arr[pos]))) pos++;
    }
    return result;
}

} // namespace parse
} // namespace json
