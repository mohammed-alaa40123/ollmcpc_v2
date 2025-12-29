#include "mcp/server_proxy.hpp"
#include "utils/json.hpp"
#include "utils/jsonrpc.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

MCPServer::MCPServer(const std::string& name) : server_name(name), request_id(0), pid(-1) {}

MCPServer::~MCPServer() {
    disconnect();
}

bool MCPServer::connect(const std::vector<std::string>& command) {
    server_command = command;
    if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0) {
        std::cerr << "Failed to create pipes\n";
        return false;
    }
    
    pid = fork();
    if (pid < 0) {
        std::cerr << "Failed to fork\n";
        return false;
    }
    
    if (pid == 0) {
        // Child process
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        
        std::vector<char*> args;
        for (const auto& arg : command) {
            args.push_back(const_cast<char*>(arg.c_str()));
        }
        args.push_back(nullptr);
        
        execvp(args[0], args.data());
        std::cerr << "Failed to exec server\n";
        exit(1);
    }
    
    // Parent process
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    
    return initialize();
}
//cite https://modelcontextprotocol.io/specification/2025-06-18/schema
bool MCPServer::initialize() {
    std::map<std::string, std::string> params;
    params["protocolVersion"] = json::str("2024-11-05");
    params["clientInfo"] = json::obj({
        {"name", json::str("cpp-mcp-client")},
        {"version", json::str("1.0.0")}
    });
    params["capabilities"] = "{}";
    
    std::string response = sendRequest("initialize", json::obj(params));
    
    if (response.empty()) {
        std::cerr << "Failed to initialize\n";
        return false;
    }
    
    // Send initialized notification
    sendNotification("notifications/initialized", "{}");
    
    std::cout << "✓ Connected to MCP server: " << server_name << "\n";
    return true;
}

std::string MCPServer::sendRequest(const std::string& method, const std::string& params) {
    int id = ++request_id;
    std::string json_msg = jsonrpc::request(id, method, params) + "\n";
    write(stdin_pipe[1], json_msg.c_str(), json_msg.length());
    return readResponse();
}

void MCPServer::sendNotification(const std::string& method, const std::string& params) {
    std::string json_msg = jsonrpc::notification(method, params) + "\n";
    write(stdin_pipe[1], json_msg.c_str(), json_msg.length());
}

#include "utils/logger.hpp"
#include <poll.h>

std::string MCPServer::readResponse() {
    std::string line;
    char c;
    
    // We expect JSON-RPC messages to be on a single line ending in \n
    // Many servers print logs or npx info to stdout, so we skip non-JSON lines
    
    int retries = 300; // Try reading for up to 30 seconds (slow APIs like Monica AI)
    while (retries-- > 0) {
        line = "";
        bool found_json = false;
        
        // Read one line
        while (true) {
            ssize_t n = read(stdout_pipe[0], &c, 1);
            if (n <= 0) break;
            if (c == '\n') break;
            line += c;
        }
        
        if (line.empty()) {
            usleep(100000); // 100ms
            continue;
        }
        
        // Check if it looks like JSON
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first != std::string::npos && line[first] == '{') {
            utils::Logger::debug("[" + server_name + "] recv: " + line);
            return line;
        } else {
            utils::Logger::debug("[" + server_name + "] junk: " + line);
        }
    }
    
    return "";
}

std::string MCPServer::listTools() {
    return sendRequest("tools/list", "{}");
}

std::string MCPServer::callTool(const std::string& tool_name, const std::string& arguments) {
    std::map<std::string, std::string> params;
    params["name"] = json::str(tool_name);
    params["arguments"] = arguments;
    
    std::string response = sendRequest("tools/call", json::obj(params));
    
    // Check for JSON-RPC error first
    std::string error_msg = json::parse::get_string(response, "message");
    if (!error_msg.empty()) {
        return "MCP error: " + error_msg;
    }
    
    // Extract the text content from the response
    std::string content_array = json_parse::extract_array(response, "content");
    if (!content_array.empty()) {
        std::string text = json_parse::extract_string(content_array, "text");
        return text;
    }
    
    return "";
}

void MCPServer::disconnect() {
    if (pid > 0) {
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        kill(pid, SIGTERM);
        waitpid(pid, nullptr, 0);
        pid = -1;
    }
}
