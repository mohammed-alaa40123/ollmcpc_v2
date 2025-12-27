#pragma once

#include <string>
#include <vector>
#include <map>

struct Config {
    std::string default_provider = "ollama";
    std::string ollama_model = "functiongemma";
    std::string gemini_api_key = "";
    std::string gemini_model = "gemini-1.5-flash";
    bool human_in_loop = true;
    
    struct MCPServerConfig {
        std::string name;
        std::vector<std::string> command;
    };
    std::vector<MCPServerConfig> servers;

    static Config load_default();
    void save_default() const;
    static void interactive_setup();
};
