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
    
    // HIL
    if (content.find("\"human_in_loop\": false") != std::string::npos) config.human_in_loop = false;
    
    // Servers (Very basic parsing of array of objects)
    // For now we assume the file structure is simple enough or user added via code.
    // Given the lack of a full JSON lib in this snippet, complex parsing is tricky.
    // We will leave server loading basic/empty for this recreation or rely on manual edit.
    
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
    file << "  \"servers\": []\n"; // Saving empty for now to avoid corrupting manual edits
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
