#include "llm/gemini.hpp"
#include "utils/json.hpp"
#include "utils/http.hpp"
#include <map>

GeminiProvider::GeminiProvider(const std::string& model, const std::string& key) 
    : model_name(model), api_key(key) {}

void GeminiProvider::addTool(const std::string& name, const std::string& description, 
                             const std::string& parameters) {
    for (const auto& existing : registered_tool_names) {
        if (existing == name) return;
    }
    registered_tool_names.push_back(name);

    std::map<std::string, std::string> tool;
    tool["name"] = json::str(name);
    tool["description"] = json::str(description);
    tool["parameters"] = parameters;
    
    tools_json.push_back(json::obj(tool));
}

std::string GeminiProvider::chat(const std::string& user_message, 
                                 const std::vector<std::map<std::string, std::string>>& history) {
    std::vector<std::string> contents;
    
    for (const auto& msg : history) {
        std::string role = "user";
        auto it_role = msg.find("role");
        if (it_role != msg.end()) {
            if (it_role->second == "\"assistant\"") role = "model";
            else if (it_role->second == "\"system\"") continue;
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
    
    if (!tools_json.empty()) {
        std::map<std::string, std::string> tool_container;
        tool_container["function_declarations"] = json::arr(tools_json);
        root["tools"] = json::arr({json::obj(tool_container)});
    }

    std::string request_json = json::obj(root);
    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/" + model_name + ":generateContent?key=" + api_key;
    
    return HTTPClient::post(url, request_json);
}
