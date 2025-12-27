#include "mcp/client.hpp"
#include "app/config.hpp"
#include "llm/provider_factory.hpp"
#include "utils/json.hpp"
#include "utils/terminal.hpp"
#include "utils/logger.hpp"
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

MCPClient::MCPClient(std::unique_ptr<LLMProvider> provider) : llm(std::move(provider)) {}

MCPClient::~MCPClient() = default;

void MCPClient::addServer(const std::string& name, const std::vector<std::string>& command) {
    auto server = std::make_unique<MCPServer>(name);
    if (server->connect(command)) {
        servers.push_back(std::move(server));
        registerTools();
    }
}

void MCPClient::setProvider(std::unique_ptr<LLMProvider> new_provider) {
    if (new_provider) {
        llm = std::move(new_provider);
        registerTools(); 
    }
}

void MCPClient::registerTools() {
    if (!llm) return;
    
    // Providers handle duplicate check in addTool. 
    
    for (const auto& server : servers) {
        std::string list_resp = server->listTools();
        
        std::string result_obj = json_parse::extract_json_object(list_resp, "result");
        if (result_obj == "{}" || result_obj.length() < 10) {
            std::string t_arr = json_parse::extract_array(list_resp, "tools");
            if (!t_arr.empty()) result_obj = list_resp;
        }

        std::string tools_arr = json_parse::extract_array(result_obj, "tools");
        if (tools_arr.empty() || tools_arr == "[]") {
            utils::Logger::debug("No tools found for server: " + server->getName());
            continue;
        }

        size_t pos = 0;
        int tool_count = 0;
        while (pos < tools_arr.length()) {
            pos = tools_arr.find("{", pos);
            if (pos == std::string::npos) break;

            std::string tool_obj = json_parse::extract_json_object(tools_arr.substr(pos), "{");
            if (tool_obj != "{}") {
                std::string name = json_parse::extract_string(tool_obj, "name");
                std::string desc = json_parse::extract_string(tool_obj, "description");
                std::string schema = json_parse::extract_json_object(tool_obj, "inputSchema");
                
                if (!name.empty()) {
                    llm->addTool(name, desc, schema);
                    tool_count++;
                }
            }
            pos += tool_obj.length();
        }
        utils::Logger::debug("Registered " + std::to_string(tool_count) + " tools from " + server->getName());
    }
}

std::string MCPClient::chat(const std::string& user_message, const std::vector<std::map<std::string, std::string>>& history) {
    if (!llm) return "Error: No LLM provider";
    return llm->chat(user_message, history);
}
