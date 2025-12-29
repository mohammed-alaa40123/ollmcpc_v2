#pragma once

#include <string>
#include <vector>

class MCPServer {
private:
    int pid;
    int stdin_pipe[2];
    int stdout_pipe[2];
    int request_id;
    std::string server_name;
    std::vector<std::string> server_command;

    bool initialize();
    std::string sendRequest(const std::string& method, const std::string& params);
    void sendNotification(const std::string& method, const std::string& params);
    std::string readResponse();

public:
    MCPServer(const std::string& name);
    ~MCPServer();

    bool connect(const std::vector<std::string>& command);
    std::string listTools();
    std::string callTool(const std::string& tool_name, const std::string& arguments);
    void disconnect();

    std::string getName() const { return server_name; }
    std::vector<std::string> getCommand() const { return server_command; }
};
