#include "utils/json.hpp"
#include "utils/jsonrpc.hpp"
#include "utils/logger.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <array>
#include <cstdio>

class MCPServerApp {
public:
    MCPServerApp() : tools_directory("/usr/local/share/ollmcpc/tools") {
        // Tools are installed globally to /usr/local/share/ollmcpc/tools
        // This allows running ollmcpc from any directory
    }
 

    void run() {
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line.empty()) continue;
            process_request(line);
        }
    }

private:
    std::string tools_directory;

    struct ToolMeta {
        std::string name;
        std::string script;
        std::string description;
        std::string inputSchema;
    };
    
    // Hardcoded tool definitions
    std::vector<ToolMeta> tools_metadata = {
        {"get_top_cpu", "get_top_cpu.sh", "Get top CPU processes", R"({"type":"object","properties":{"count":{"type":"integer"}}})"},
        {"system_uptime", "system_uptime.sh", "Show system uptime", R"({"type":"object","properties":{}})"},
        {"check_port_usage", "check_port_usage.sh", "Check port usage", R"({"type":"object","properties":{"port":{"type":"integer"}},"required":["port"]})"},
        {"list_directory", "list_directory.sh", "List directory", R"({"type":"object","properties":{"path":{"type":"string"}}})"},
        {"get_process_threads", "get_process_threads.sh", "List process threads", R"({"type":"object","properties":{"pid":{"type":"integer"}},"required":["pid"]})"},
        {"get_process_tree", "get_process_tree.sh", "Show process tree", R"({"type":"object","properties":{"pid":{"type":"integer"}}})"},
        {"check_io_status", "check_io_status.sh", "Check system IO", R"({"type":"object","properties":{}})"},
        {"get_active_connections", "get_active_connections.sh", "List network connections", R"({"type":"object","properties":{}})"},
        {"run_shell_command", "run_shell_command.sh", "Run a shell command", R"({"type":"object","properties":{"command":{"type":"string"}},"required":["command"]})"},
        {"run_secure_shell_command", "dispatcher", "Run a shell command in a secure way and it regards admin priverlages", R"({"type":"object","properties":{"command":{"type":"string"}},"required":["command"]})"}
    };


    void process_request(const std::string& json_req) {
        utils::Logger::debug("Server received: " + json_req);
        
        // Parse JSON-RPC request
        auto req = jsonrpc::parse_request(json_req);
        
        if (req.is_notification) {
            utils::Logger::debug("Request ignored (notification)");
            return; 
        }
        
        std::string result = "{}";
        
        if (req.method == "initialize") {
            result = R"({
                "protocolVersion": "2024-11-05",
                "serverInfo": {"name": "c-mcp-server", "version": "1.2"},
                "capabilities": {"tools": {}}
            })";
        } else if (req.method == "tools/list") {
            result = "{\"tools\": [";
            for (size_t i = 0; i < tools_metadata.size(); ++i) {
                if (i > 0) result += ",";
                result += "{\"name\":" + json::str(tools_metadata[i].name) + 
                          ",\"description\":" + json::str(tools_metadata[i].description) + 
                          ",\"inputSchema\":" + tools_metadata[i].inputSchema + "}";
            }
            result += "]}";
        } else if (req.method == "tools/call") {
             std::string name = json::parse::get_string(req.params, "name");
             std::string args_json = json::parse::get_object(req.params, "arguments");
             
             utils::Logger::debug("Lookup tool: [" + name + "]");
             
             // Find tool
             auto it = std::find_if(tools_metadata.begin(), tools_metadata.end(), 
                                   [&](const ToolMeta& t){ return t.name == name; });
             
             if (it != tools_metadata.end()) {
                 utils::Logger::debug("Executing " + it->name);
                 std::string output = execute_tool(it->script, args_json, it->name);
                 result = "{\"content\":[{\"type\":\"text\",\"text\":" + json::str(output) + "}]}";
             } else {
                 utils::Logger::error("Tool not found: [" + name + "]");
                 result = "{\"isError\":true,\"content\":[{\"type\":\"text\",\"text\":\"Unknown tool\"}]}";
             }
        }
        
        // Send JSON-RPC response
        std::cout << jsonrpc::response(req.id, result) << std::endl;
    }

    std::string execute_tool(const std::string& script, const std::string& args_json, const std::string& tool_name) {
        std::string cmd = tools_directory + "/" + script;
        std::vector<std::string> args;
        
        // Basic argument parsing leveraging json helper logic or manual (since logic was in ToolRunner)
        // We will inline the specific logic for minimal dependencies
        
        if (tool_name == "check_port_usage") {
             std::string port = json_parse::extract_val(args_json, "\"port\":");
             if (!port.empty()) args.push_back(port);
        } else if (tool_name == "list_directory") {
             std::string path = json_parse::extract_string(args_json, "path");
             if (path.empty()) path = ".";
             args.push_back(path);
        } else if (tool_name == "get_process_threads" || tool_name == "get_process_tree") {
             std::string pid = json_parse::extract_val(args_json, "\"pid\":");
             if (!pid.empty()) args.push_back(pid);
        } else if (tool_name == "run_shell_command") {
             std::string c = json_parse::extract_string(args_json, "command");
             if (!c.empty()) args.push_back(c);
        } else if (tool_name == "run_secure_shell_command") {
             std::string c = json_parse::extract_string(args_json, "command");
             if (!c.empty()) args.push_back(c);
        }

        // Construct command line
        for (const auto& arg : args) {
             // Basic escaping
             std::string escaped = arg;
             size_t pos = 0;
             while ((pos = escaped.find("\"", pos)) != std::string::npos) {
                 escaped.replace(pos, 1, "\\\"");
                 pos += 2;
             }
             cmd += " \"" + escaped + "\"";
        }
        
        // Log the command being executed
        utils::Logger::debug("[execute_tool] Tool: " + tool_name);
        utils::Logger::debug("[execute_tool] Script: " + script);
        utils::Logger::debug("[execute_tool] Full command: " + cmd);
        utils::Logger::debug("[execute_tool] Args JSON: " + args_json);
        
        // Execute
        std::array<char, 256> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    
        if (!pipe) {
            utils::Logger::error("[execute_tool] popen() failed for: " + cmd);
            return "Error: Failed to execute tool script";
        }
    
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
    
        if (!result.empty() && result.back() == '\n') result.pop_back();
        
        utils::Logger::debug("[execute_tool] Result length: " + std::to_string(result.length()));
        if (result.length() < 500) {
            utils::Logger::debug("[execute_tool] Result: " + result);
        }
        
        return result;
    }
};

int main() {
    MCPServerApp server;
    server.run();
    return 0;
}
