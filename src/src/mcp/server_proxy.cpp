#include "mcp/server_proxy.hpp"
#include "utils/json.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <map>

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
    std::map<std::string, std::string> msg;
    msg["jsonrpc"] = json::str("2.0");
    msg["id"] = json::num(id);
    msg["method"] = json::str(method);
    msg["params"] = params;
    
    std::string json_msg = json::obj(msg) + "\n";
    write(stdin_pipe[1], json_msg.c_str(), json_msg.length());
    
    return readResponse();
}

void MCPServer::sendNotification(const std::string& method, const std::string& params) {
    std::map<std::string, std::string> msg;
    msg["jsonrpc"] = json::str("2.0");
    msg["method"] = json::str(method);
    msg["params"] = params;
    
    std::string json_msg = json::obj(msg) + "\n";
    write(stdin_pipe[1], json_msg.c_str(), json_msg.length());
}

std::string MCPServer::readResponse() {
    char buffer[8192];
    ssize_t n = read(stdout_pipe[0], buffer, sizeof(buffer) - 1);
    if (n <= 0) return "";
    buffer[n] = '\0';
    return std::string(buffer);
}

std::string MCPServer::listTools() {
    return sendRequest("tools/list", "{}");
}

std::string MCPServer::callTool(const std::string& tool_name, const std::string& arguments) {
    std::map<std::string, std::string> params;
    params["name"] = json::str(tool_name);
    params["arguments"] = arguments;
    
    std::string response = sendRequest("tools/call", json::obj(params));
    
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
