#pragma once

#include <string>
#include <vector>
#include <map>

// =============================================================================
// JSON Builder and Parser Module
// =============================================================================
//
// This module provides lightweight JSON building and parsing utilities.
// It is NOT a full JSON parser - it uses simple string manipulation for speed.
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
    
    /// Create a JSON boolean value: true/false
    std::string boolean(bool b);
    
    /// Create a JSON object from key-value pairs: {"key": value, ...}
    /// Note: Values should already be JSON-formatted (use str(), num(), etc.)
    std::string obj(const std::map<std::string, std::string>& kvs);
    
    /// Create a JSON array from items: [item1, item2, ...]
    /// Note: Items should already be JSON-formatted
    std::string arr(const std::vector<std::string>& items);
    
    /// Sanitize malformed JSON from external sources
    /// Fixes issues like extra braces inside quoted strings
    std::string sanitize(const std::string& s);

    // -------------------------------------------------------------------------
    // JSON Parsing Functions (in nested namespace)
    // -------------------------------------------------------------------------
    namespace parse {
        /// Unescape a JSON string value (convert \n, \r, \t, \", \\ back)
        std::string unescape(const std::string& s);
        
        /// Extract a string value for a given key: {"key": "value"} -> "value"
        std::string get_string(const std::string& json, const std::string& key);
        
        /// Extract an integer value for a given key: {"key": 123} -> 123
        int get_int(const std::string& json, const std::string& key);
        
        /// Extract a boolean value for a given key: {"key": true} -> true
        bool get_bool(const std::string& json, const std::string& key);
        
        /// Extract a nested object for a given key: {"key": {...}} -> "{...}"
        std::string get_object(const std::string& json, const std::string& key);
        
        /// Extract an array for a given key: {"key": [...]} -> "[...]"
        std::string get_array(const std::string& json, const std::string& key);
        
        /// Extract array of strings: {"key": ["a", "b"]} -> ["a", "b"]
        std::vector<std::string> get_string_array(const std::string& json, const std::string& key);
        
        /// Check if a key exists in the JSON
        bool has_key(const std::string& json, const std::string& key);
        
        /// Extract the first JSON object from a string (finds first { and matches })
        /// Useful for parsing arrays of objects: [{...}, {...}]
        std::string first_object(const std::string& json);
    }
}

// =============================================================================
// Legacy aliases for backwards compatibility (will be removed in future)
// =============================================================================
namespace json_parse {
    inline std::string unescape(const std::string& s) { return json::parse::unescape(s); }
    inline std::string extract_string(const std::string& j, const std::string& k) { return json::parse::get_string(j, k); }
    inline std::string extract_object(const std::string& j, const std::string& k) { return json::parse::get_object(j, k); }
    inline std::string extract_json_object(const std::string& j, const std::string& k) { return json::parse::get_object(j, k); }
    inline std::string extract_array(const std::string& j, const std::string& k) { return json::parse::get_array(j, k); }
    inline std::vector<std::string> extract_string_array(const std::string& j, const std::string& k) { return json::parse::get_string_array(j, k); }
    inline bool has_key(const std::string& j, const std::string& k) { return json::parse::has_key(j, k); }
    
    // Special case: extract raw numeric value (legacy function)
    std::string extract_val(const std::string& json, const std::string& key);
}
