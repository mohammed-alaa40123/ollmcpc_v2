#include "llm/gemini.hpp"
#include "utils/json.hpp"
#include "utils/http.hpp"
#include "utils/logger.hpp"
#include <map>

GeminiProvider::GeminiProvider(const std::string& model, const std::string& key) 
    : model_name(model), api_key(key) {}

void GeminiProvider::addTool(const std::string& name, const std::string& description, 
                             const std::string& parameters) {
    if (hasToolNamed(name)) return;

    std::map<std::string, std::string> tool;
    tool["name"] = json::str(name);
    tool["description"] = json::str(description);
    tool["parameters"] = parameters;
    
    tools_json.push_back(json::obj(tool));
    tools.push_back({name, description, parameters});
    utils::Logger::debug("GeminiProvider: Added tool " + name + " (Total: " + std::to_string(tools.size()) + ")");
}

std::string GeminiProvider::chat(const std::string& user_message, 
                                 const std::vector<std::map<std::string, std::string>>& history) {
    std::vector<std::string> contents;
    std::string system_instr;
    
    for (const auto& msg : history) {
        std::string role = "user";
        auto it_role = msg.find("role");
        if (it_role != msg.end()) {
            std::string r_val = it_role->second;
            // Remove quotes if present
            if (r_val.front() == '"') r_val = r_val.substr(1, r_val.length() - 2);
            
            if (r_val == "assistant" || r_val == "model") role = "model";
            else if (r_val == "system") {
                auto it_content = msg.find("content");
                if (it_content != msg.end()) {
                    std::string s_val = it_content->second;
                    if (s_val.front() == '"') s_val = s_val.substr(1, s_val.length() - 2);
                    system_instr = s_val;
                }
                continue;
            }
        }
        
        std::map<std::string, std::string> part;
        auto it_content = msg.find("content");
        if (it_content != msg.end()) {
            part["text"] = it_content->second;
        }
        
        std::map<std::string, std::string> content;
        content["role"] = json::str(role);
        content["parts"] = json::arr({json::obj(part)});
        contents.push_back(json::obj(content));
    }
    
    if (!user_message.empty()) {
        std::map<std::string, std::string> current_part;
        current_part["text"] = json::str(user_message);
        
        std::map<std::string, std::string> current_content;
        current_content["role"] = json::str("user");
        current_content["parts"] = json::arr({json::obj(current_part)});
        contents.push_back(json::obj(current_content));
    }

    std::map<std::string, std::string> root;
    root["contents"] = json::arr(contents);
    
    if (!system_instr.empty()) {
        std::map<std::string, std::string> s_part;
        s_part["text"] = json::str(system_instr);
        root["system_instruction"] = json::obj({{"parts", json::arr({json::obj(s_part)})}});
    }

    if (!tools_json.empty()) {
        std::map<std::string, std::string> tool_container;
        tool_container["function_declarations"] = json::arr(tools_json);
        root["tools"] = json::arr({json::obj(tool_container)});
    }

    std::string request_json = json::obj(root);
    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/" + model_name + ":generateContent?key=" + api_key;
    
    utils::Logger::debug("Gemini Request: " + request_json);
    std::string response = HTTPClient::post(url, request_json);
    utils::Logger::debug("Gemini Response: " + response);

    // Basic normalization for interactive.cpp
    std::string candidates = json::parse::get_array(response, "candidates");
    if (candidates.empty() || candidates == "[]") {
        return response; // Return error or empty
    }

    std::string first_candidate = json::parse::first_object(candidates);
    std::string content_obj = json::parse::get_object(first_candidate, "content");
    std::string parts_arr = json::parse::get_array(content_obj, "parts");
    std::string first_part = json::parse::first_object(parts_arr);
    
    std::string text = json::parse::get_string(first_part, "text");
    std::string func_call = json::parse::get_object(first_part, "functionCall");
    
    std::map<std::string, std::string> msg;
    msg["role"] = json::str("assistant");
    msg["content"] = json::str(text);
    
    if (func_call != "{}" && !func_call.empty()) {
        std::string name = json::parse::get_string(func_call, "name");
        std::string args = json::parse::get_object(func_call, "args");
        
        // Compact arguments
        std::string compact_args;
        for (char c : args) {
            if (c == '\n' || c == '\r' || c == '\t') compact_args += ' ';
            else compact_args += c;
        }
        
        std::map<std::string, std::string> tool_call;
        tool_call["name"] = json::str(name);
        tool_call["arguments"] = compact_args;
        
        msg["tool_calls"] = json::arr({json::obj(tool_call)});
    }
    
    std::map<std::string, std::string> final_root;
    final_root["message"] = json::obj(msg);
    
    return json::obj(final_root);
}
