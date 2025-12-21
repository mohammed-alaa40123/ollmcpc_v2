#include "llm/manual.hpp"
#include "utils/terminal.hpp"
#include "utils/json.hpp"
#include <iostream>

void ManualProvider::addTool(const std::string& name, const std::string& description, 
                             const std::string& parameters) {
    for (const auto& t : tools) {
        if (t.name == name) return;
    }
    tools.push_back({name, description, parameters});
}

std::string ManualProvider::chat(const std::string& user_message, 
                                 const std::vector<std::map<std::string, std::string>>& history) {
    if (tools.empty()) {
        return "{\"message\": {\"content\": \"No tools available.\"}}";
    }
    int choice = 0;
    try { choice = std::stoi(user_message); } catch (...) {}
    if (choice <= 0) {
        for (size_t i = 0; i < tools.size(); i++) {
            if (tools[i].name == user_message) {
                choice = i + 1;
                break;
            }
        }
    }

    if (user_message == "list" || choice <= 0 || choice > (int)tools.size()) {
        std::vector<term::ToolInfo> tool_infos;
        for (const auto& t : tools) {
            tool_infos.push_back({t.name, t.description});
        }
        term::display_tool_menu(tool_infos);
        
        if (user_message == "list") {
            return "{\"message\": {\"content\": \"\"}}";
        }

        std::cout << "  " << term::YELLOW << "0. Skip" << term::RESET << " | or type name directly\n";
        std::cout << "  " << term::DIM << "(Tip: Just press ENTER to skip/cancel)" << term::RESET << "\n";
        std::cout << "\n  " << term::BOLD << "Select a tool # : " << term::RESET << std::flush;
        
        std::string choice_str;
        std::getline(std::cin, choice_str);
        if (choice_str.empty()) return "{\"message\": {\"content\": \"Manual selection skipped.\"}}";
        try { choice = std::stoi(choice_str); } catch (...) {
            for (size_t i = 0; i < tools.size(); i++) {
                if (tools[i].name == choice_str) {
                    choice = i + 1;
                    break;
                }
            }
        }
    }

    if (choice > 0 && choice <= (int)tools.size()) {
        const auto& t = tools[choice - 1];
        
        std::cout << "\n  " << term::BOLD << "Arguments for " << term::RESET << term::CYAN << t.name << term::RESET << "\n";
        std::cout << "  " << term::DIM << "Format: JSON (e.g. {\"path\": \"/tmp\"})" << term::RESET << "\n";
        std::cout << "  Input (enter for empty {}): " << std::flush;
        
        std::string args;
        std::getline(std::cin, args);
        if (args.empty()) args = "{}";

        std::map<std::string, std::string> call;
        call["name"] = json::str(t.name);
        call["id"] = json::str("manual_call_" + std::to_string(choice));
        call["arguments"] = args;

        std::map<std::string, std::string> msg;
        msg["tool_calls"] = json::arr({json::obj(call)});
        
        std::map<std::string, std::string> root;
        root["message"] = json::obj(msg);
        
        return json::obj(root);
    }

    return "{\"message\": {\"content\": \"Manual selection skipped.\"}}";
}
