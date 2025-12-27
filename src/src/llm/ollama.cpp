#include "llm/ollama.hpp"
#include "utils/json.hpp"
#include "utils/http.hpp"
#include <map>

OllamaProvider::OllamaProvider(const std::string& model) 
    : model_name(model), ollama_url("http://localhost:11434") {}

void OllamaProvider::addTool(const std::string& name, const std::string& description, 
                             const std::string& parameters) {
    for (const auto& existing : registered_tool_names) {
        if (existing == name) return;
    }
    registered_tool_names.push_back(name);

    std::map<std::string, std::string> func;
    func["name"] = json::str(name);
    func["description"] = json::str(description);
    func["parameters"] = parameters;
    
    std::map<std::string, std::string> tool;
    tool["type"] = json::str("function");
    tool["function"] = json::obj(func);
    
    tools_json.push_back(json::obj(tool));
    tools.push_back({name, description, parameters});
}

std::string OllamaProvider::chat(const std::string& user_message, 
                                 const std::vector<std::map<std::string, std::string>>& history) {
    std::vector<std::string> messages;
    
    for (const auto& msg : history) {
        messages.push_back(json::obj(msg));
    }
    
    if (!user_message.empty()) {
        std::map<std::string, std::string> user_msg;
        user_msg["role"] = json::str("user");
        user_msg["content"] = json::str(user_message);
        messages.push_back(json::obj(user_msg));
    }
    
    std::map<std::string, std::string> request;
    request["model"] = json::str(model_name);
    request["messages"] = json::arr(messages);
    request["tools"] = json::arr(tools_json);
    request["stream"] = "false";
    
    std::string request_json = json::obj(request);
    
    return HTTPClient::post(ollama_url + "/api/chat", request_json);
}
