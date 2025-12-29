#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>

namespace utils {
    class Logger {
    private:
        static std::string get_timestamp() {
            std::time_t now = std::time(nullptr);
            char buf[20];
            std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
            return std::string(buf);
        }

    public:
        static void log(const std::string& level, const std::string& message) {
            std::ofstream file("debug.log", std::ios::app);
            if (file.is_open()) {
                file << "[" << get_timestamp() << "] [" << level << "] " << message << "\n";
            }
        }

        static void debug(const std::string& message) { log("DEBUG", message); }
        static void error(const std::string& message) { log("ERROR", message); }
    };
}
