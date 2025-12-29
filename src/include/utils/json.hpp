#pragma once

#include <string>
#include <vector>
#include <map>

// =============================================================================
// JSON Builder and Parser Module
// =============================================================================
//
// Lightweight JSON utilities using simple string manipulation.
// NOT a full JSON parser - designed for speed and simplicity.
//
// Namespaces:
//   json::       - Build JSON strings
//   json::parse  - Parse/extract values from JSON strings
//
// =============================================================================

namespace json {
    // -------------------------------------------------------------------------
    // JSON Building Functions
    // -------------------------------------------------------------------------
    
    /// Escape special characters for JSON strings (\n, \r, \t, \, ")
    std::string escape(const std::string& s);
    
    /// Create a JSON string value: "value" (with quotes and escaping)
    std::string str(const std::string& s);
    
    /// Create a JSON number value: 123
    std::string num(int n);
    
    /// Create a JSON object from key-value pairs: {"key": value, ...}
    /// Note: Values should already be JSON-formatted (use str(), num(), etc.)
    std::string obj(const std::map<std::string, std::string>& kvs);
    
    /// Create a JSON array from items: [item1, item2, ...]
    /// Note: Items should already be JSON-formatted
    std::string arr(const std::vector<std::string>& items);
    
    /// Sanitize malformed JSON from external MCP servers
    /// Removes $schema, additionalProperties fields; fixes trailing commas
    std::string sanitize(const std::string& s);

    // -------------------------------------------------------------------------
    // JSON Parsing Functions
    // -------------------------------------------------------------------------
    namespace parse {
        /// Unescape a JSON string value (convert \n, \r, \t, \", \\ back)
        std::string unescape(const std::string& s);
        
        /// Extract a string value for a given key: {"key": "value"} -> "value"
        std::string get_string(const std::string& json, const std::string& key);
        
        /// Extract a nested object for a given key: {"key": {...}} -> "{...}"
        std::string get_object(const std::string& json, const std::string& key);
        
        /// Extract an array for a given key: {"key": [...]} -> "[...]"
        std::string get_array(const std::string& json, const std::string& key);
        
        /// Check if a key exists in the JSON
        bool has_key(const std::string& json, const std::string& key);
        
        /// Extract the first JSON object from a string (finds first { and matches })
        std::string first_object(const std::string& json);
        
        /// Extract raw numeric value for a key pattern (e.g. "\"id\":")
        std::string get_raw_value(const std::string& json, const std::string& key_pattern);
        
        /// Extract array of strings: {"key": ["a", "b"]} -> ["a", "b"]
        std::vector<std::string> get_string_array(const std::string& json, const std::string& key);
    }
}

// Legacy compatibility (deprecated - will be removed)
namespace json_parse {
    inline std::string extract_string(const std::string& j, const std::string& k) { 
        return json::parse::get_string(j, k); 
    }
    inline std::string extract_array(const std::string& j, const std::string& k) { 
        return json::parse::get_array(j, k); 
    }
    inline std::string extract_val(const std::string& json, const std::string& key) {
        return json::parse::get_raw_value(json, key);
    }
}
