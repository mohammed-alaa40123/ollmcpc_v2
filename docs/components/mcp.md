# MCP Backend

The MCP Backend is the core of OLLMCPC's power, implementing the Model Context Protocol communication.

## MCP Client (`client.cpp`)

The `MCPClient` manages the lifecycle of all connected servers.

### Functions:
*   **Initialization**: Sends the `initialize` request to all servers.
*   **Tool Aggregation**: Merges tools from all servers into a single list for the LLM.
*   **Routing**: When a tool called `filesystem_read_file` is requested, it knows to strip the `filesystem_` prefix and send the `read_file` request to the correct server.

## Server Proxy (`server_proxy.cpp`)

Handles the low-level details of running an external MCP server.

### Implementation:
*   **Pipe Communication**: Uses `dup2` and `pipe` to capture `stdin` and `stdout` of the child process.
*   **JSON-RPC 2.0**: Implements the standard JSON-RPC protocol over the pipes.
*   **Async Reading**: Uses threads to read output from the server so the main client stays responsive.

## Internal Server (`mcp_server` target)

A standalone binary built from `src/mcp/server.cpp`. It provides the "OS Assistant" tools.

### Key Tools:
*   `execute_shell`: Run arbitrary commands safely.
*   `cpu_usage`: Monitor system load.
*   `process_list`: See what's running.

## Key Files:
*   `src/include/mcp/client.h`: The MCP registry and manager.
*   `src/include/mcp/server_proxy.h`: External process bridge.
*   `src/src/mcp/client.cpp`: Logic for cross-server coordination.
*   `src/src/mcp/server_proxy.cpp`: Logic for stdio/pipe communication.
