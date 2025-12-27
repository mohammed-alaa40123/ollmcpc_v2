# OLLMCPC (Ollama/LLM MCP Client)

**OLLMCPC** is a high-performance terminal-based client for the **Model Context Protocol (MCP)**. It bridges the gap between Large Language Models (LLMs) and your local system tools, allowing AI models to execute shell scripts and interact with your environment safely.

## 🚀 Features

-   **Multi-Provider Support**:
    -   **Ollama**: Use local LLMs (e.g., Llama 3, Mistral) via the Ollama API.
    -   **Gemini**: Connect to Google's cloud-based Gemini models.
    -   **Manual Mode**: Directly control tools via an interactive menu (perfect for debugging).
-   **Local MCP Server**: Includes a built-in server (`mcp_server`) that exposes a suite of system tools (CPU monitoring, process management, shell command execution, etc.).
-   **Human-in-the-Loop (HIL)**: Safety first! Requires explicit user approval before the LLM executes any destructive or system-altering tools.
-   **Interactive TUI**: A beautiful, colorized terminal interface for seamless chat and tool interaction.
-   **Extensible**: Easily add new tools by dropping shell scripts into the `tools/` directory.

## 🛠️ Getting Started

### Prerequisites

Before building OLLMCPC, ensure your system has the following dependencies:

- **Compiler**: GCC 9+ or Clang 10+ (C++17 support required).
- **Build System**: CMake 3.10+ and Make.
- **Libraries**:
    - `libcurl`: For all HTTP communication (Ollama/Gemini APIs).
    - `libssl`: For secure connections.
    - `nlohmann-json`: (Bundled or system) For JSON serialization.
- **Backends**:
    - **Ollama**: Required for local LLM support. [Install here](https://ollama.com).
    - **Node.js/NPM**: Required for running external MCP servers (like filesystem or fetch).

**Quick Install (Ubuntu/Debian):**
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libcurl4-openssl-dev libssl-dev nodejs npm
```

### Installation & Build

#### Automated Setup
We provide a setup script that handles dependencies, builds the project, pulls recommended models, and installs the binaries:

```bash
chmod +x scripts/setup.sh
./scripts/setup.sh
```

#### Manual Build
If you prefer to build manually:

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

This will produce:
- `ollmcpc`: The main interactive client.
- `mcp_server`: The internal tool host.

---

## 📖 Usage

### Initializing Configuration
Before your first run, generate a default configuration:

```bash
./build/ollmcpc config
```
This creates `~/.ollmcpc.json`. You can edit this file to add API keys or additional MCP servers.

### Starting a Session
The primary way to use OLLMCPC is the `serve` command:

```bash
# Start with default settings (from config)
./build/ollmcpc serve

# Start with a specific provider and model
./build/ollmcpc serve --provider gemini --model gemini-1.5-flash
```

### Interactive Commands
Inside the session, you can use:
- `/help`: Show available internal commands.
- `/servers`: List all connected MCP servers and their status.
- `/toggle <name>`: Enable/Disable a specific MCP server.
- `/exit` or `Ctrl+D`: End the session.

### Direct Tool Access (Manual Mode)
If you just want to test tools without an LLM:
```bash
./build/ollmcpc serve --provider manual
```
Use the arrow keys and Enter to select and execute tools.


## ⚙️ Configuration

OLLMCPC stores its settings in `~/.ollmcpc.json`. You can configure:
-   Default LLM provider and models.
-   API keys for cloud providers.
-   Human-in-the-loop safety settings.
-   Additional MCP servers to connect to.

To configure interactively, run:
```bash
./build/ollmcpc config
```

## 📂 Project Structure

-   `src/`: Core C++ source code.
    -   `main.cpp`: CLI entry point.
    -   `app/`: UI and configuration logic.
    -   `llm/`: Providers for Ollama, Gemini, and Manual control.
    -   `mcp/`: Protocol implementation (Client and Server).
    -   `utils/`: Networking (CURL), JSON handling, and Terminal styling.
-   `tools/`: A collection of shell scripts used by the built-in MCP server to perform system tasks.

## 🛡️ Safety

The project implements a **Human-in-the-Loop** mechanism. When an LLM requests a tool execution, the client pauses and prompts the user for confirmation. This ensures you have full control over what the AI does on your system.

## 📚 Detailed Documentation

For more in-depth information about the architecture, components, and advanced configuration, check out our documentation website.

To view it locally:
```bash
./scripts/serve_docs.sh
```
Then visit `http://127.0.0.1:8000` in your browser.

---
Built with ❤️ by the OLLMCPC Team.

