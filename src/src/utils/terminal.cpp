#include "utils/terminal.hpp"
#include <iostream>
#include <sstream>

namespace term {

void draw_box(const std::string& title, const std::string& content, const std::string& color) {
    int width = 80;
    int inner_width = width - 4;
    std::cout << color << "╭" << repeat("─", 2) << "[ " << BOLD << title << RESET << color << " ]" << repeat("─", width - (int)title.length() - 7) << "╮" << RESET << "\n";
    
    std::stringstream ss(content);
    std::string line;
    while (std::getline(ss, line)) {
        while (line.length() > (size_t)inner_width) {
            size_t pos = line.find_last_of(" \t", inner_width);
            if (pos == std::string::npos) pos = inner_width;
            
            std::string part = line.substr(0, pos);
            std::cout << color << "│ " << RESET << part << std::string(inner_width - part.length(), ' ') << color << " │" << RESET << "\n";
            line = line.substr(pos == inner_width ? pos : pos + 1);
        }
        std::cout << color << "│ " << RESET << line << std::string(inner_width - line.length(), ' ') << color << " │" << RESET << "\n";
    }
    
    std::cout << color << "╰" << repeat("─", width - 2) << "╯" << RESET << "\n";
}

void print_thought(const std::string& thought) {
    if (thought.empty()) return;
    std::cout << DIM << "┏━ Thinking ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << RESET << "\n";
    std::stringstream ss(thought);
    std::string line;
    while (std::getline(ss, line)) {
        std::cout << DIM << "┃ " << line << RESET << "\n";
    }
    std::cout << DIM << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << RESET << "\n\n";
}

void display_tool_menu(const std::vector<ToolInfo>& tools) {
    if (tools.empty()) {
        draw_box("AVAILABLE TOOLS & SCRIPTS", "No tools available.", CYAN);
        return;
    }

    std::string menu;
    size_t num_tools = tools.size();
    size_t cols = 3;
    size_t rows = (num_tools + cols - 1) / cols;
    const size_t col_width = 21;

    for (size_t r = 0; r < rows; r++) {
        std::string line;
        for (size_t c = 0; c < cols; c++) {
            size_t idx = c * rows + r;
            if (idx < num_tools) {
                std::string idx_str = std::to_string(idx + 1) + ". ";
                std::string name = tools[idx].name;
                
                if (name.length() > col_width - 4) {
                    name = name.substr(0, col_width - 6) + "..";
                }

                std::string item = idx_str + name;
                line += item;
                
                if (c < cols - 1) {
                    for (size_t p = item.length(); p < col_width; p++) line += " ";
                }
            }
        }
        menu += line + (r < rows - 1 ? "\n\n" : "");
    }

    draw_box("AVAILABLE TOOLS & SCRIPTS", menu, CYAN);
}

} // namespace term
