#include "app/interactive.hpp"
#include "app/config.hpp"
#include "llm/provider_factory.hpp"
#include "utils/json.hpp"
#include "utils/terminal.hpp"
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <set>

namespace app {

void run_interactive_session(MCPClient& client) {
    std::string input;
    std::vector<std::map<std::string, std::string>> conversation_history;
    
    std::cout << "\033[2J\033[H" << std::flush;
    term::print_header("OLLMCPC PREMIUM v3.6", term::CYAN);
    std::cout << "  Type " << term::BOLD << "/help" << term::RESET << " for list of commands.\n";

    while (true) {
        LLMProvider* llm = client.getLLM();
        std::string p_name = llm->name();
        // Robust lowercase conversion for comparison
        for (auto& c : p_name) c = std::tolower(c);
        
        bool is_manual = (p_name.find("manual") != std::string::npos);
        
        std::cout << "\n";
        if (is_manual) {
             std::cout << term::MAGENTA << "╭─ " << term::BOLD << "MODE: USER-DIRECT (MANUAL)" << term::RESET << term::MAGENTA << " " << term::repeat("━", 33) << "╮" << term::RESET << "\n";
             std::cout << term::CYAN << "  >> Press " << term::BOLD << "ENTER" << term::RESET << " to view available tools." << term::RESET << "\n";
             std::cout << term::BOLD << term::CYAN << "  👤 PROMPT " << term::RESET << "> " << std::flush;
        } else {
             std::string name = llm->name();
             std::transform(name.begin(), name.end(), name.begin(), ::toupper);
             std::cout << term::CYAN << "╭─ " << term::BOLD << "MODEL: " << name << term::RESET << term::CYAN << " | " << term::BOLD << "HIL: " << (client.human_in_loop ? term::GREEN + "ACTIVE" : term::RED + "OFF") << term::RESET << term::CYAN << " " << term::repeat("━", 35) << "╮" << term::RESET << "\n";
             std::cout << term::BOLD << term::LIGHT_BLUE << "  👤 INPUT  " << term::RESET << "> " << std::flush;
        }
        
        if (!std::getline(std::cin, input)) break;
        
        // Trim
        size_t first = input.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            if (is_manual) input = "list";
            else continue;
        } else {
            size_t last = input.find_last_not_of(" \t\r\n");
            input = input.substr(first, (last - first + 1));
        }

        // --- Commands ---
        if (input == "/quit" || input == "exit" || input == "/exit") break;
        
        if (input == "/help" || input == "help" || input == "/?") {
            term::print_header("COMMAND CENTRE", term::MAGENTA);
            std::cout << term::CYAN << "  /mode    " << term::RESET << "» Swap backend (Manual ↔ Ollama ↔ Gemini)\n"
                      << term::CYAN << "  /hil     " << term::RESET << "» Toggle Human-in-the-loop Guard\n"
                      << term::CYAN << "  /clear   " << term::RESET << "» Reset screen and context state\n"
                      << term::CYAN << "  /servers " << term::RESET << "» Inspect active MCP nodes\n"
                      << term::CYAN << "  /config  " << term::RESET << "» Open Preferences Manager\n"
                      << term::CYAN << "  /list    " << term::RESET << "» Inventory of available tools\n"
                      << term::CYAN << "  /quit    " << term::RESET << "» Terminate session\n\n";
            continue;
        }

        if (input == "/clear") {
            std::cout << "\033[2J\033[H" << std::flush;
            term::print_header("OLLMCPC PREMIUM v3.6", term::CYAN);
            conversation_history.clear();
            continue;
        }

        if (input == "/servers") {
            term::print_header("ACTIVE MCP NODES", term::MAGENTA);
            auto& servers = client.getServers();
            if (servers.empty()) {
                std::cout << "  " << term::RED << "⚠ NO NODES DETECTED" << term::RESET << "\n";
            } else {
                for (const auto& s : servers) {
                    std::cout << "  " << term::GREEN << "▣" << term::RESET << " " << term::BOLD << s->getName() << term::RESET << "\n";
                    auto cmd = s->getCommand();
                    std::cout << "    " << term::DIM << "Run: ";
                    for (const auto& c : cmd) std::cout << c << " ";
                    std::cout << term::RESET << "\n";
                }
            }
            std::cout << "\n";
            continue;
        }

        if (input == "/mode") {
            Config cfg = Config::load_default();
            std::string old = llm->name();
            if (old == "manual") cfg.default_provider = "ollama";
            else if (old == "ollama") cfg.default_provider = "gemini";
            else cfg.default_provider = "manual";
            
            client.setProvider(createProvider(cfg));
            std::cout << term::YELLOW << "  🔄 SHIFTED: [" << old << "] ➔ [" << term::BOLD << client.getLLM()->name() << term::RESET << "]\n";
            conversation_history.clear();
            continue;
        }

        if (input == "/hil") {
            client.human_in_loop = !client.human_in_loop;
            std::cout << term::MAGENTA << "  ⚙️  HIL GUARD: " << (client.human_in_loop ? term::GREEN + "ENABLED" : term::RED + "DISABLED") << term::RESET << "\n";
            continue;
        }

        if (input == "/list") {
            client.getLLM()->chat("list", {});
            continue;
        }

        if (input == "/config") {
            Config::interactive_setup();
            Config cfg = Config::load_default();
            // Immediate update
            client.setProvider(createProvider(cfg));
            client.human_in_loop = cfg.human_in_loop;
            std::cout << term::GREEN << "  ✨ System preferences applied immediately. Mode: " << term::BOLD << client.getLLM()->name() << term::RESET << "\n";
            conversation_history.clear();
            continue;
        }

        // --- Core Turn Analysis ---
        int loops = 0;
        std::string current_message = input;
        std::set<std::string> turn_tool_signatures;
        
        while (loops < client.loop_limit) {
            // Visual processing feedback
            if (loops == 0) {
                if (!is_manual) term::print_thought("Invoking " + llm->name() + " AI kernel...");
            } else {
                if (!is_manual) term::print_thought("Interpreting tool results & generating summary...");
            }

            std::string response = llm->chat(current_message, conversation_history);
            
            std::string msg_obj = json_parse::extract_json_object(response, "message");
            if (msg_obj == "{}") msg_obj = response;

            std::string content = json_parse::extract_string(msg_obj, "content");
            if (!content.empty()) {
                term::draw_box("ASSISTANT", content, term::WHITE);
                
                // Keep context for conversational flow
                std::map<std::string, std::string> hist_item;
                hist_item["role"] = json::str("assistant");
                hist_item["content"] = json::str(content);
                conversation_history.push_back(hist_item);
            }
            
            // Analyze for Tool Calls
            std::string tool_calls_arr = json_parse::extract_array(msg_obj, "tool_calls");
            if (tool_calls_arr.empty() || tool_calls_arr == "[]") break;
            
            std::string call_obj = json_parse::extract_json_object(tool_calls_arr, "{"); 
            if (call_obj == "{}") break;

            std::string tool_name = json_parse::extract_string(call_obj, "name");
            std::string tool_args = json_parse::extract_json_object(call_obj, "arguments");
            if (tool_name.empty()) break;

            // Security & Loop Prevention
            std::string sig = tool_name + ":" + tool_args;
            if (turn_tool_signatures.count(sig)) {
                term::print_thought("Redundant tool call suppressed: " + tool_name);
                break;
            }
            turn_tool_signatures.insert(sig);

            // Access Control (HIL)
            bool approved = true;
            if (is_manual) {
                approved = true; // Hard override for manual mode
            } else if (client.human_in_loop) {
                term::draw_box("GUARD - ACTION PENDING", "Action:  " + term::BOLD + tool_name + term::RESET + "\nInputs:  " + term::DIM + tool_args + term::RESET, term::YELLOW);
                std::cout << "  " << term::BOLD << "Execute? ([y]/n): " << term::RESET << std::flush;
                std::string app_in;
                std::getline(std::cin, app_in);
                if (!app_in.empty() && app_in != "y" && app_in != "Y") {
                    approved = false;
                }
            } else {
                term::draw_box("AUTO-EXECUTING", tool_name + "...", term::CYAN);
            }

            if (!approved) {
                std::cout << term::RED << "  Operation rejected by user.\n" << term::RESET;
                break;
            }

            // Execution phase
            std::string raw_result = "";
            bool success = false;
            for (const auto& server : client.getServers()) {
                 std::string r = server->callTool(tool_name, tool_args);
                 if (!r.empty()) {
                     raw_result = r;
                     success = true;
                     break;
                 }
            }
            
            if (!success) {
                raw_result = "Exception: Target tool unavailable or script failure.";
            }
            
            if (!is_manual) {
                // PREMIUM: Change System Log color to GREEN for better contrast
                term::draw_box("SYSTEM OUTPUT LOG", raw_result, term::GREEN);
                
                // Force synthesis by adding strong prompt to history
                std::map<std::string, std::string> tool_context;
                tool_context["role"] = json::str("user");
                tool_context["content"] = json::str("Action: Tool [" + tool_name + "] executed.\nResult: " + raw_result + "\nConstraint: Please summarize this data clearly for the human user. Do not call any more tools until the user speaks again.");
                conversation_history.push_back(tool_context);
                
                // Prepare for next turn in loop
                current_message = ""; // LLM will rely on conversational context (history)
            } else {
                term::draw_box("LOCAL DEVICE DATA: " + tool_name, raw_result, term::GREEN);
                break; // Manual is single-turn
            }
            
            loops++;
        }
    }
}

} // namespace app
