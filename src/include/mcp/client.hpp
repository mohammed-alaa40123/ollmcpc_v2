#pragma once

#include "llm/provider.hpp"
#include "mcp/server_proxy.hpp"
#include <string>
#include <vector>
#include <memory>
#include <map>

class MCPClient {
public:
    MCPClient(std::unique_ptr<LLMProvider> provider);
    ~MCPClient();

    void addServer(const std::string& name, const std::vector<std::string>& command);
    
    // Core chat method
    std::string chat(const std::string& user_message, 
                     const std::vector<std::map<std::string, std::string>>& history = {});
    
    void setProvider(std::unique_ptr<LLMProvider> new_provider);
    
    LLMProvider* getLLM() { return llm.get(); }
    const std::vector<std::unique_ptr<MCPServer>>& getServers() const { return servers; }
    
    bool human_in_loop = true;
    int loop_limit = 5;

    // Explicit call to register tools from scratch if needed
    void syncTools() { registerTools(); }

private:
    std::unique_ptr<LLMProvider> llm;
    std::vector<std::unique_ptr<MCPServer>> servers;
    std::vector<std::string> conversation_history; 
    
    void registerTools();
};
