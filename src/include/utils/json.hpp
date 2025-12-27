#pragma once

#include <string>
#include <vector>
#include <map>

namespace json {
    std::string escape(const std::string& s);
    std::string str(const std::string& s);
    std::string num(int n);
    std::string obj(const std::map<std::string, std::string>& kvs);
    std::string arr(const std::vector<std::string>& items);
}

namespace json_parse {
    std::string extract_string(const std::string& json, const std::string& key);
    bool has_key(const std::string& json, const std::string& key);
    std::string extract_object(const std::string& json, const std::string& key);
    std::string extract_array(const std::string& json, const std::string& key);
    std::string extract_val(const std::string& json, const std::string& key);
    std::string extract_json_object(const std::string& json, const std::string& key);
    std::vector<std::string> extract_string_array(const std::string& json, const std::string& key);
}
