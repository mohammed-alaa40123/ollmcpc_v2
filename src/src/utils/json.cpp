#include "utils/json.hpp"
#include <sstream>

namespace json {
    std::string escape(const std::string& s) {
        std::string result;
        for (char c : s) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c;
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
}

namespace json_parse {
    std::string unescape(const std::string& s) {
        std::string res;
        for (size_t i = 0; i < s.length(); ++i) {
            if (s[i] == '\\' && i + 1 < s.length()) {
                switch (s[i+1]) {
                    case 'n': res += '\n'; i++; break;
                    case 'r': res += '\r'; i++; break;
                    case 't': res += '\t'; i++; break;
                    case '"': res += '"'; i++; break;
                    case '\\': res += '\\'; i++; break;
                    default: res += s[i];
                }
            } else {
                res += s[i];
            }
        }
        return res;
    }

    std::string extract_string(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\"";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return "";
        
        pos += search.length();
        // Skip whitespace, colon, and find the opening quote
        while (pos < json.length() && json[pos] != ':') pos++;
        if (pos >= json.length()) return "";
        pos++; // Skip ':'
        
        while (pos < json.length() && isspace(json[pos])) pos++;
        if (pos >= json.length() || json[pos] != '"') return "";
        size_t start = pos + 1;
        
        size_t end = json.find("\"", start);
        while (end != std::string::npos && json[end-1] == '\\') {
            end = json.find("\"", end + 1);
        }
        if (end == std::string::npos) return "";
        return unescape(json.substr(start, end - start));
    }
    
    bool has_key(const std::string& json, const std::string& key) {
        return json.find("\"" + key + "\"") != std::string::npos;
    }
    
    std::string extract_object(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        size_t start = json.find(search);
        if (start == std::string::npos) return "";
        start += search.length();
        
        while (start < json.length() && isspace(json[start])) start++;
        
        if (json[start] != '{') return "";
        
        int depth = 0;
        size_t end = start;
        for (; end < json.length(); end++) {
            if (json[end] == '{') depth++;
            else if (json[end] == '}') {
                depth--;
                if (depth == 0) break;
            }
        }
        
        return json.substr(start, end - start + 1);
    }
    
    std::string extract_array(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        size_t start = json.find(search);
        if (start == std::string::npos) return "";
        start += search.length();
        
        while (start < json.length() && isspace(json[start])) start++;
        
        if (json[start] != '[') return "";
        
        int depth = 0;
        size_t end = start;
        for (; end < json.length(); end++) {
            if (json[end] == '[') depth++;
            else if (json[end] == ']') {
                depth--;
                if (depth == 0) break;
            }
        }
        
        return json.substr(start, end - start + 1);
    }

    std::string extract_val(const std::string& json, const std::string& key) {
         size_t pos = json.find(key);
         if (pos == std::string::npos) return "";
         size_t start = pos + key.length();
         while (start < json.length() && (json[start] == ' ' || json[start] == ':')) start++;
         size_t end = start;
         while (end < json.length() && (isdigit(json[end]))) end++;
         return json.substr(start, end - start);
    }

    std::string extract_json_object(const std::string& json, const std::string& key) {
        size_t pos = json.find(key);
        if (pos == std::string::npos) return "{}";
        
        size_t start = json.find("{", pos);
        if (start == std::string::npos) return "{}";
        
        int brackets = 1;
        size_t end = start + 1;
        while (end < json.length() && brackets > 0) {
            if (json[end] == '{') brackets++;
            else if (json[end] == '}') brackets--;
            end++;
        }
        return json.substr(start, end - start);
    }
    std::vector<std::string> extract_string_array(const std::string& json, const std::string& key) {
        std::vector<std::string> result;
        std::string array_str = extract_array(json, key);
        if (array_str.empty() || array_str == "[]") return result;
        
        size_t pos = 1; // skip '['
        while (pos < array_str.length() - 1) {
            size_t start = array_str.find("\"", pos);
            if (start == std::string::npos || start >= array_str.length() - 1) break;
            start++;
            size_t end = array_str.find("\"", start);
            while (end != std::string::npos && array_str[end-1] == '\\') {
                end = array_str.find("\"", end + 1);
            }
            if (end == std::string::npos) break;
            
            result.push_back(unescape(array_str.substr(start, end - start)));
            pos = end + 1;
            while (pos < array_str.length() && (array_str[pos] == ',' || isspace(array_str[pos]))) pos++;
        }
        return result;
    }
}
