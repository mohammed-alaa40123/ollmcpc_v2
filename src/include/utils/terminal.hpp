#include <string>
#include <iostream>
#include <sstream>
#include <vector>

namespace term {
    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string LIGHT_BLUE = "\033[94m";
    const std::string WHITE = "\033[97m";
    const std::string BOLD = "\033[1m";
    const std::string DIM = "\033[2m";

    inline std::string repeat(const std::string& s, int n) {
        std::string res;
        for (int i = 0; i < n; i++) res += s;
        return res;
    }

    inline void print_line(const std::string& color = "") {
        std::cout << color << std::string(60, '-') << RESET << "\n";
    }

    inline void print_header(const std::string& title, const std::string& color) {
        std::cout << color << "┏" << repeat("━", title.length() + 2) << "┓" << RESET << "\n";
        std::cout << color << "┃ " << BOLD << title << RESET << color << " ┃" << RESET << "\n";
        std::cout << color << "┗" << repeat("━", title.length() + 2) << "┛" << RESET << "\n";
    }

    struct ToolInfo {
        std::string name;
        std::string description;
    };

    void draw_box(const std::string& title, const std::string& content, const std::string& color);
    void print_thought(const std::string& thought);
    void display_tool_menu(const std::vector<ToolInfo>& tools);
}
