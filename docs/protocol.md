# JSON-RPC Protocol & Tool Integration

This document explains how OLLMCPC implements the Model Context Protocol (MCP) using JSON-RPC 2.0, and how tools are integrated with Large Language Models.

## Overview

OLLMCPC uses **JSON-RPC 2.0** as the transport protocol for all MCP communication. This enables:
- Bidirectional communication between the client and MCP servers
- Standardized request/response patterns
- Tool discovery and execution across different server implementations

## JSON-RPC 2.0 Basics

### Message Structure

All JSON-RPC messages are single-line JSON objects terminated by a newline (`\n`).

#### Request Format
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "tools/list",
  "params": {}
}
```

#### Response Format
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "tools": [...]
  }
}
```

#### Notification Format (No Response Expected)
```json
{
  "jsonrpc": "2.0",
  "method": "notifications/initialized",
  "params": {}
}
```

---

## MCP Server Lifecycle

### 1. Connection Establishment

When OLLMCPC starts, it spawns external MCP servers as child processes using `fork()` and `pipe()`:

```cpp
// Parent process keeps stdin_pipe[1] and stdout_pipe[0]
// Child process uses stdin_pipe[0] for input, stdout_pipe[1] for output
```

### 2. Initialization Handshake

**Request** (Client → Server):
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "initialize",
  "params": {
    "protocolVersion": "2024-11-05",
    "clientInfo": {
      "name": "cpp-mcp-client",
      "version": "1.0.0"
    },
    "capabilities": {}
  }
}
```

**Response** (Server → Client):
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "protocolVersion": "2024-11-05",
    "serverInfo": {
      "name": "filesystem-server",
      "version": "1.0.0"
    },
    "capabilities": {
      "tools": {}
    }
  }
}
```

**Notification** (Client → Server):
```json
{
  "jsonrpc": "2.0",
  "method": "notifications/initialized",
  "params": {}
}
```

### 3. Tool Discovery

**Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 2,
  "method": "tools/list",
  "params": {}
}
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 2,
  "result": {
    "tools": [
      {
        "name": "read_file",
        "description": "Read the contents of a file",
        "inputSchema": {
          "type": "object",
          "properties": {
            "path": {
              "type": "string",
              "description": "Path to the file"
            }
          },
          "required": ["path"]
        }
      }
    ]
  }
}
```

### 4. Tool Execution

**Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 3,
  "method": "tools/call",
  "params": {
    "name": "read_file",
    "arguments": {
      "path": "/home/user/example.txt"
    }
  }
}
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 3,
  "result": {
    "content": [
      {
        "type": "text",
        "text": "File contents here..."
      }
    ]
  }
}
```

---

## LLM Integration

### Tool Schema Transformation

MCP tools are transformed into LLM-compatible function schemas. Here's how OLLMCPC does it:

#### From MCP Format
```json
{
  "name": "read_file",
  "description": "Read a file from the filesystem",
  "inputSchema": {
    "type": "object",
    "properties": {
      "path": {"type": "string"}
    }
  }
}
```

#### To Ollama/OpenAI Format
```json
{
  "type": "function",
  "function": {
    "name": "filesystem_read_file",
    "description": "Read a file from the filesystem",
    "parameters": {
      "type": "object",
      "properties": {
        "path": {"type": "string"}
      }
    }
  }
}
```

**Note**: OLLMCPC prefixes tool names with the server name (e.g., `filesystem_read_file`) to avoid naming collisions when multiple servers provide similar tools.

### Complete LLM Request

When you send a message to the LLM, OLLMCPC constructs a request like this:

```json
{
  "model": "functiongemma",
  "messages": [
    {
      "role": "system",
      "content": "You are a powerful terminal assistant. You can use tools to interact with the system..."
    },
    {
      "role": "user",
      "content": "What files are in my home directory?"
    }
  ],
  "tools": [
    {
      "type": "function",
      "function": {
        "name": "filesystem_list_directory",
        "description": "List files in a directory",
        "parameters": {
          "type": "object",
          "properties": {
            "path": {"type": "string"}
          }
        }
      }
    },
    {
      "type": "function",
      "function": {
        "name": "os_assistant_run_shell_command",
        "description": "Execute a shell command",
        "parameters": {
          "type": "object",
          "properties": {
            "command": {"type": "string"}
          }
        }
      }
    }
  ],
  "stream": false
}
```

### LLM Response with Tool Call

The LLM responds with a tool call request:

```json
{
  "message": {
    "role": "assistant",
    "content": "",
    "tool_calls": [
      {
        "id": "call_abc123",
        "type": "function",
        "function": {
          "name": "filesystem_list_directory",
          "arguments": "{\"path\": \"/home/user\"}"
        }
      }
    ]
  }
}
```

---

## The Conversation Loop

OLLMCPC implements an intelligent multi-turn conversation loop:

### Step-by-Step Flow

1. **User Input**: User sends a message
2. **LLM Analysis**: Message + conversation history + available tools → LLM
3. **Tool Call Detection**: Parse LLM response for `tool_calls`
4. **Human-in-the-Loop (HIL)**: If enabled, ask user for approval
5. **Tool Execution**: Route to correct MCP server, execute tool
6. **Result Injection**: Add tool result to conversation history
7. **Synthesis**: LLM processes the tool result and generates a human-readable response
8. **Loop**: Repeat if LLM requests more tools (up to `loop_limit`)

### Example Conversation History

```json
[
  {
    "role": "system",
    "content": "You are a powerful terminal assistant..."
  },
  {
    "role": "user",
    "content": "What's my CPU usage?"
  },
  {
    "role": "assistant",
    "content": "",
    "tool_calls": [{"function": {"name": "os_assistant_cpu_usage", "arguments": "{}"}}]
  },
  {
    "role": "user",
    "content": "Action: Tool [cpu_usage] executed.\nResult: CPU: 23.4%\nConstraint: Please summarize this data clearly..."
  },
  {
    "role": "assistant",
    "content": "Your current CPU usage is 23.4%, which indicates a light system load."
  }
]
```

---

## Implementation Details

### Pipe Communication

OLLMCPC uses UNIX pipes for stdio communication with MCP servers:

```cpp
// Write request to server's stdin
std::string json_msg = json::obj(msg) + "\n";
write(stdin_pipe[1], json_msg.c_str(), json_msg.length());

// Read response from server's stdout
char c;
std::string line;
while (read(stdout_pipe[0], &c, 1) > 0) {
    if (c == '\n') break;
    line += c;
}
```

### Handling Non-JSON Output

Many MCP servers (especially those run via `npx`) output logs or warnings to stdout. OLLMCPC filters these:

```cpp
// Skip lines that don't start with '{'
if (line[0] != '{') {
    Logger::debug("junk: " + line);
    continue;
}
```

### Loop Prevention

To prevent infinite loops, OLLMCPC tracks tool signatures within a single turn:

```cpp
std::string sig = tool_name + ":" + tool_args;
if (turn_tool_signatures.count(sig)) {
    // Already called this exact tool with these args
    break;
}
turn_tool_signatures.insert(sig);
```

---

## Security Considerations

### Human-in-the-Loop (HIL)

Before executing any tool, OLLMCPC can prompt for user approval:

```
╭─ GUARD - ACTION PENDING ────────────────────╮
│ Action:  run_shell_command                  │
│ Inputs:  {"command": "rm -rf /tmp/cache"}   │
╰─────────────────────────────────────────────╯
Execute? ([y]/n):
```

### Sudo Handling

When a tool requests `sudo`, OLLMCPC:
1. Detects the `sudo` prefix in the command
2. Prompts the user for their password (with hidden input)
3. Rewrites the command to use `echo password | sudo -S`

### Argument Sanitization

Tool arguments are compacted to prevent newline injection:

```cpp
for (char c : tool_args) {
    if (c == '\n' || c == '\r' || c == '\t') 
        compacted_args += ' ';
    else 
        compacted_args += c;
}
```

---

## References

- [Model Context Protocol Specification](https://modelcontextprotocol.io/specification/2025-06-18/schema)
- [JSON-RPC 2.0 Specification](https://www.jsonrpc.org/specification)
- Source: `src/src/mcp/server_proxy.cpp`
- Source: `src/src/app/interactive.cpp`
- Source: `src/src/llm/ollama.cpp`
