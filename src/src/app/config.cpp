#include "app/config.hpp"
#include "utils/json.hpp"
#include "utils/terminal.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>

std::string get_config_path() {
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) home = pwd->pw_dir;
    }
    return std::string(home ? home : ".") + "/.ollmcpc.json";
}

Config Config::load_default() {
    Config config;
    std::string path = get_config_path();
    std::ifstream file(path);
    if (!file.is_open()) return config;

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    // Simple parsing (robust parser would use a library)
    std::string provider = json_parse::extract_string(content, "default_provider");
    if (!provider.empty()) config.default_provider = provider;
    
    std::string omodel = json_parse::extract_string(content, "ollama_model");
    if (!omodel.empty()) config.ollama_model = omodel;
    
    std::string gkey = json_parse::extract_string(content, "gemini_api_key");
    if (!gkey.empty()) config.gemini_api_key = gkey;

    std::string gmodel = json_parse::extract_string(content, "gemini_model");
    if (!gmodel.empty()) config.gemini_model = gmodel;
    
    // HIL
    if (content.find("\"human_in_loop\": false") != std::string::npos || 
        content.find("\"human_in_loop\":false") != std::string::npos) {
        config.human_in_loop = false;
    } else {
        config.human_in_loop = true;
    }
    
    // Servers
    std::string servers_arr = json_parse::extract_array(content, "servers");
    if (!servers_arr.empty() && servers_arr != "[]") {
        size_t pos = 1; // skip '['
        while (pos < servers_arr.length() - 1) {
            size_t start = servers_arr.find("{", pos);
            if (start == std::string::npos || start >= servers_arr.length() - 1) break;
            
            std::string obj = json_parse::extract_json_object(servers_arr.substr(start), "{");
            if (obj != "{}") {
                MCPServerConfig s;
                s.name = json_parse::extract_string(obj, "name");
                s.command = json_parse::extract_string_array(obj, "command");
                
                // Parse enabled (default true)
                s.enabled = true;
                if (obj.find("\"enabled\": false") != std::string::npos || 
                    obj.find("\"enabled\":false") != std::string::npos) {
                    s.enabled = false;
                }

                if (!s.name.empty() && !s.command.empty()) {
                    config.servers.push_back(s);
                }
            }
            pos = start + obj.length();
            while (pos < servers_arr.length() && (servers_arr[pos] == ',' || isspace(servers_arr[pos]))) pos++;
        }
    }
    
    return config;
}

void Config::save_default() const {
    std::string path = get_config_path();
    std::ofstream file(path);
    if (!file.is_open()) return;

    file << "{\n";
    file << "  \"default_provider\": " << json::str(default_provider) << ",\n";
    file << "  \"ollama_model\": " << json::str(ollama_model) << ",\n";
    file << "  \"gemini_api_key\": " << json::str(gemini_api_key) << ",\n";
    file << "  \"gemini_model\": " << json::str(gemini_model) << ",\n";
    file << "  \"human_in_loop\": " << (human_in_loop ? "true" : "false") << ",\n";
    
    file << "  \"servers\": [\n";
    for (size_t i = 0; i < servers.size(); ++i) {
        file << "    {\n";
        file << "      \"name\": " << json::str(servers[i].name) << ",\n";
        
        // Serialize command args as JSON strings
        std::vector<std::string> cmd_json;
        for (const auto& arg : servers[i].command) {
            cmd_json.push_back(json::str(arg));
        }
        file << "      \"command\": " << json::arr(cmd_json) << ",\n";
        file << "      \"enabled\": " << (servers[i].enabled ? "true" : "false") << "\n";
        
        file << "    }" << (i < servers.size() - 1 ? "," : "") << "\n";
    }
    file << "  ]\n";
    file << "}\n";
}

void Config::interactive_setup() {
    Config config = load_default();
    
    term::print_header("ollmcpc Configuration Setup", term::CYAN);
    
    std::string input;
    
    std::cout << "Default Provider [" << config.default_provider << "] (ollama/gemini/manual): ";
    std::getline(std::cin, input);
    if (!input.empty()) config.default_provider = input;
    
    if (config.default_provider == "ollama") {
        std::cout << "Ollama Model [" << config.ollama_model << "]: ";
        std::getline(std::cin, input);
        if (!input.empty()) config.ollama_model = input;
    } else if (config.default_provider == "gemini") {
        std::cout << "Gemini API Key [" << (config.gemini_api_key.empty() ? "None" : "***") << "]: ";
        std::getline(std::cin, input);
        if (!input.empty()) config.gemini_api_key = input;
    }
    
    std::cout << "Human-in-the-Loop Approval [" << (config.human_in_loop ? "y" : "n") << "] (y/n): ";
    std::getline(std::cin, input);
    if (!input.empty()) config.human_in_loop = (input == "y" || input == "Y");
    
    config.save_default();
    std::cout << term::GREEN << "\n✓ Configuration saved to " << get_config_path() << term::RESET << "\n";
}
