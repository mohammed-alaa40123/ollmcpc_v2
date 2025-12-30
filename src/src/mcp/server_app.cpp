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
#include <cstdlib>
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
        // {"osassist_battery_info", "osassist_battery_info.sh", "Show battery and memory info", R"({"type":"object","properties":{}})"},
        // {"osassist_memory_info", "osassist_memory_info.sh", "Show memory and battery info", R"({"type":"object","properties":{}})"},
        // {"osecho_plus", "osecho_plus.sh", "Print a message with optional level and timestamp", R"({"type":"object","properties":{"message":{"type":"string"},"level":{"type":"string"},"ts":{"type":"boolean"},"log":{"type":"string"}}})"},
        // {"osenv_guard", "osenv_guard.sh", "Show environment variables with redacted secrets", R"({"type":"object","properties":{}})"},
        // {"oswhoami", "oswhoami.sh", "Show current user and environment info", R"({"type":"object","properties":{}})"},
        // {"osps", "osps.sh", "List processes sorted by CPU usage", R"({"type":"object","properties":{}})"},
        // {"osproctree", "osproctree.sh", "Show a process tree for a PID", R"({"type":"object","properties":{"pid":{"type":"integer"}}})"},
        // {"oskillsafe", "oskillsafe.sh", "Safely terminate a process by PID", R"({"type":"object","properties":{"pid":{"type":"integer"}},"required":["pid"]})"},
        // {"osstop", "osstop.sh", "Stop a process by PID", R"({"type":"object","properties":{"pid":{"type":"integer"}},"required":["pid"]})"},
        // {"oscont", "oscont.sh", "Resume a stopped process by PID", R"({"type":"object","properties":{"pid":{"type":"integer"}},"required":["pid"]})"},
        // {"osspawnchildren", "osspawnchildren.sh", "Spawn child processes for demo", R"({"type":"object","properties":{"number":{"type":"integer"},"command":{"type":"string"},"nowait":{"type":"boolean"}}})"},
        // {"osorphan", "osorphan.sh", "Run orphan process demo", R"({"type":"object","properties":{}})"},
        // {"oszombie", "oszombie.sh", "Run zombie process demo", R"({"type":"object","properties":{}})"},
        // {"ostreedemo", "ostreedemo.sh", "Run fork tree demo", R"({"type":"object","properties":{}})"},
        // {"oshelp", "oshelp.sh", "Show Mini-OS help and demos", R"({"type":"object","properties":{"command":{"type":"string"},"arg":{"type":"string"}}})"},
        // {"osdiskfree", "osdiskfree.sh", "Show disk usage for a path", R"({"type":"object","properties":{"path":{"type":"string"}},"required":["path"]})"},
        // {"osdir_size_top", "osdir_size_top.sh", "Show largest items in a directory", R"({"type":"object","properties":{}})"},
        // {"osmem_heapstack_range", "osmem_heapstack_range.sh", "Show heap and stack ranges for a PID", R"({"type":"object","properties":{"pid":{"type":"integer"}},"required":["pid"]})"},
        // {"osmem_usage", "osmem_usage.sh", "Show memory summary and status", R"({"type":"object","properties":{}})"},
        // {"osnet_basic", "osnet_basic.sh", "Show basic network status", R"({"type":"object","properties":{}})"},
        // {"osproc_children_list", "osproc_children_list.sh", "Show child processes as a tree", R"({"type":"object","properties":{"pid":{"type":"integer"}},"required":["pid"]})"},
        // {"osproc_find", "osproc_find.sh", "Find processes by name pattern", R"({"type":"object","properties":{"pattern":{"type":"string"}},"required":["pattern"]})"},
        // {"osproc_openfiles", "osproc_openfiles.sh", "Show open files for a process", R"({"type":"object","properties":{"pid":{"type":"integer"}},"required":["pid"]})"},
        // {"osshm_list", "osshm_list.sh", "List shared memory segments", R"({"type":"object","properties":{}})"},
        {"ossig_pingpong", "ossig_pingpong.sh", "Run the signal ping-pong demo", R"({"type":"object","properties":{"rounds":{"type":"integer"}}})"},
        {"osthread_demo", "osthread_demo.sh", "Run the thread roles demo", R"({"type":"object","properties":{}})"},
        {"osthread_sync_demo", "osthread_sync_demo.sh", "Run the thread sync demo", R"({"type":"object","properties":{}})"},
        {"osuptime_plus", "osuptime_plus.sh", "Show uptime, load average, and top processes", R"({"type":"object","properties":{}})"},
        {"process_info", "process_info.sh", "Show detailed process info", R"({"type":"object","properties":{"pid":{"type":"integer"}},"required":["pid"]})"},
        {"process_state", "process_state.sh", "Show process state", R"({"type":"object","properties":{"pid":{"type":"integer"}},"required":["pid"]})"},
        {"run_shell_command", "run_shell_command.sh", "Run a shell command", R"({"type":"object","properties":{"command":{"type":"string"}},"required":["command"]})"}
        
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
             std::string exec_dangerous = json::parse::get_string(req.params, "exec_dangerous");
             std::string args_json = json::parse::get_object(req.params, "arguments");
             
             utils::Logger::debug("Lookup tool: [" + name + "] exec_dangerous=" + exec_dangerous);
             
             // Find tool
             auto it = std::find_if(tools_metadata.begin(), tools_metadata.end(), 
                                   [&](const ToolMeta& t){ return t.name == name; });
             
             if (it != tools_metadata.end()) {
                 utils::Logger::debug("Executing " + it->name);
                 
                 std::string output = execute_tool(it->script, args_json, it->name,exec_dangerous);
                 result = "{\"content\":[{\"type\":\"text\",\"text\":" + json::str(output) + "}]}";
             } else {
                 utils::Logger::error("Tool not found: [" + name + "]");
                 result = "{\"isError\":true,\"content\":[{\"type\":\"text\",\"text\":\"Unknown tool\"}]}";
             }
        }
        
        // Send JSON-RPC response
        std::cout << jsonrpc::response(req.id, result) << std::endl;
    }

    std::string execute_tool(const std::string& script, const std::string& args_json, const std::string& tool_name, std::string exec_dangerous) {
        utils::Logger::debug("execute_tool: " + tool_name + " exec_dangerous=" + exec_dangerous);
        std::string cmd = tools_directory + "/dispatcher " + ((exec_dangerous == "YES") ? "-y" : "-n") + " ";
        std::vector<std::string> args;
        
        // Basic argument parsing leveraging json helper logic or manual (since logic was in ToolRunner)
        // We will inline the specific logic for minimal dependencies
        
        if (tool_name == "process_info" ||
                   tool_name == "process_state" ||
                   tool_name == "osproc_children_list" ||
                   tool_name == "osproc_openfiles" ||
                   tool_name == "osmem_heapstack_range" ||
                   tool_name == "osstop" ||
                   tool_name == "oscont" ||
                   tool_name == "oskillsafe") {
             std::string pid = json_parse::extract_string(args_json, "pid");
             if (pid.empty()) {
                 pid = json_parse::extract_string(args_json, "value");
             }
             if (pid.empty()) {
                 pid = json_parse::extract_val(args_json, "\"pid\":");
             }
             if (pid.empty() && tool_name == "osproc_openfiles") {
                 pid = json_parse::extract_string(args_json, "path");
             }
             if (!pid.empty()) args.push_back(pid);
        } else if (tool_name == "osproc_find") {
             std::string pattern = json_parse::extract_string(args_json, "pattern");
             if (pattern.empty()) {
                 pattern = json_parse::extract_string(args_json, "value");
             }
             if (!pattern.empty()) args.push_back(pattern);
        } else if (tool_name == "osfile_watch") {
             std::string path = json_parse::extract_string(args_json, "path");
             std::string interval = json_parse::extract_string(args_json, "interval");
             std::string lines = json_parse::extract_string(args_json, "lines");
             if (path.empty()) return "Error: osfile_watch requires a path";
             if (!path.empty()) args.push_back(path);
             if (!interval.empty()) {
                 args.push_back("--interval");
                 args.push_back(interval);
             }
             if (!lines.empty()) {
                 args.push_back("--lines");
                 args.push_back(lines);
             }
        } else if (tool_name == "ossig_pingpong") {
             std::string rounds = json_parse::extract_string(args_json, "rounds");
             if (rounds.empty()) {
                 rounds = json_parse::extract_string(args_json, "value");
             }
             if (!rounds.empty()) args.push_back(rounds);
        } else if (tool_name == "osproctree") {
             std::string pid = json_parse::extract_string(args_json, "pid");
             if (pid.empty()) {
                 pid = json_parse::extract_string(args_json, "value");
             }
             if (pid.empty()) {
                 pid = json_parse::extract_val(args_json, "\"pid\":");
             }
             if (!pid.empty()) args.push_back(pid);
        } else if (tool_name == "osspawnchildren") {
             std::string count = json_parse::extract_string(args_json, "number");
             std::string command = json_parse::extract_string(args_json, "command");
             std::string nowait = json_parse::extract_string(args_json, "nowait");
             if (count.empty()) {
                 count = json_parse::extract_string(args_json, "value");
             }
             if (count.empty()) {
                 count = json_parse::extract_val(args_json, "\"number\":");
             }
             if (nowait.empty()) {
                 nowait = json_parse::extract_val(args_json, "\"nowait\":");
             }
             if (!count.empty()) {
                 args.push_back("--number");
                 args.push_back(count);
             }
             if (!command.empty()) {
                 args.push_back("--command");
                 args.push_back(command);
             }
             if (nowait == "true" || nowait == "1") {
                 args.push_back("--nowait");
             }
        } else if (tool_name == "oshelp") {
             std::string command = json_parse::extract_string(args_json, "command");
             std::string arg = json_parse::extract_string(args_json, "arg");
             if (command.empty()) {
                 command = json_parse::extract_string(args_json, "value");
             }
             if (!command.empty()) args.push_back(command);
             if (!arg.empty()) args.push_back(arg);
        } else if (tool_name == "osdiskfree") {
             std::string path = json_parse::extract_string(args_json, "path");
             if (path.empty()) {
                 path = json_parse::extract_string(args_json, "value");
             }
             if (!path.empty()) args.push_back(path);
        } else if (tool_name == "osecho_plus") {
             std::string level = json_parse::extract_string(args_json, "level");
             std::string log = json_parse::extract_string(args_json, "log");
             std::string message = json_parse::extract_string(args_json, "message");
             if (message.empty()) {
                 message = json_parse::extract_string(args_json, "value");
             }
             std::string ts_str = json_parse::extract_string(args_json, "ts");
             bool with_ts = false;
             if (ts_str == "true" || ts_str == "1") with_ts = true;
             if (!level.empty()) {
                 args.push_back("--level");
                 args.push_back(level);
             }
             if (with_ts) {
                 args.push_back("--ts");
             }
             if (!log.empty()) {
                 args.push_back("--log");
                 args.push_back(log);
             }
             if (!message.empty()) args.push_back(message);
        } else if (tool_name == "run_shell_command") {
             std::string c = json_parse::extract_string(args_json, "command");
             if (!c.empty()) args.push_back(c);
        } else if (tool_name == "run_secure_shell_command") {
             std::string c = json_parse::extract_string(args_json, "command");
             if (!c.empty()) args.push_back(c);
        }

        // Construct command line
        std::string script_path = tools_directory + "/" + script;
        if (tool_name == "run_shell_command" || tool_name == "run_secure_shell_command") {
             cmd += script_path;
             for (const auto& arg : args) {
                 std::string escaped = arg;
                 size_t pos = 0;
                 while ((pos = escaped.find("\"", pos)) != std::string::npos) {
                     escaped.replace(pos, 1, "\\\"");
                     pos += 2;
                 }
                 cmd += " \"" + escaped + "\"";
             }
        } else {
             std::string packed = script_path;
             for (const auto& arg : args) {
                 packed += " ";
                 packed += arg;
             }
             std::string escaped = packed;
             size_t pos = 0;
             while ((pos = escaped.find("\"", pos)) != std::string::npos) {
                 escaped.replace(pos, 1, "\\\"");
                 pos += 2;
             }
             cmd += "\"" + escaped + "\"";
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
