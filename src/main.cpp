#include "app/config.hpp"
#include "mcp/client.hpp"
#include "llm/provider_factory.hpp"
#include "app/interactive.hpp"
#include "utils/terminal.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>

void show_help() {
    term::print_header("OLLMCPC Use", term::CYAN);
    std::cout << "Usage: ollmcpc <command> [options]\n\n"
              << "Commands:\n"
              << "  serve       Start the MCP client session\n"
              << "  config      Run interactive configuration\n"
              << "  list        List available models/providers\n"
              << "  --help      Show this help message\n\n"
              << "Options (for serve):\n"
              << "  --provider <name>   Override default provider (manual, ollama, gemini)\n"
              << "  --model <name>      Override default model\n";
}

void list_modes(const Config& config) {
    term::print_header("Available Modes", term::BLUE);
    std::cout << "1. " << term::BOLD << "Manual" << term::RESET << " (Direct tool control via menu)\n";
    std::cout << "2. " << term::BOLD << "Ollama" << term::RESET << " (Local AI models: " << config.ollama_model << ")\n";
    std::cout << "3. " << term::BOLD << "Gemini" << term::RESET << " (Cloud AI: " << config.gemini_model << ")\n";
}

int main(int argc, char** argv) {
    // Determine path to config if not standard
    Config config = Config::load_default();

    if (argc < 2) {
        show_help();
        return 0;
    }

    std::string command = argv[1];

    if (command == "--help" || command == "-h") {
        show_help();
        return 0;
    }

    if (command == "list") {
        list_modes(config);
        return 0;
    }

    if (command == "config") {
        Config::interactive_setup();
        return 0;
    }

    if (command == "serve") {
        // Init Curl
        curl_global_init(CURL_GLOBAL_DEFAULT);
        
        // Disable buffering for instant TUI
        setvbuf(stdout, NULL, _IONBF, 0);

        // Parse overrides
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--model" && i + 1 < argc) {
                config.ollama_model = argv[++i];
                config.gemini_model = config.ollama_model;
            }
            else if (arg == "--provider" && i + 1 < argc) {
                config.default_provider = argv[++i];
            }
        }
        
        // Create Provider
        auto provider = createProvider(config);
        if (!provider) {
            std::cerr << term::RED << "Failed to create provider: " << config.default_provider << term::RESET << "\n";
            return 1;
        }

        // Create Client
        MCPClient client(std::move(provider));
        client.human_in_loop = config.human_in_loop;

        std::cout << "🔌 Connecting to MCP servers...\n";
        
        // Always add the "os-assistant" server which corresponds to our mcp_server binary
        client.addServer("os-assistant", {"mcp_server"});
 

        // Add Configured Servers
        for (const auto& s : config.servers) {
            if (s.enabled) {
                client.addServer(s.name, s.command);
            } else {
                std::cout << "  " << term::DIM << "○ Skipping disabled server: " << s.name << term::RESET << "\n";
            }
        }

        // Start Loop
        app::run_interactive_session(client);

        std::cout << "\n👋 Goodbye!\n";
        curl_global_cleanup();
        return 0;
    }

    std::cerr << "Unknown command: " << command << "\n";
    show_help();
    return 1;
}
