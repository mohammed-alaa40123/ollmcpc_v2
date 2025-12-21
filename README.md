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

-   **C++17** compatible compiler (GCC/Clang)
-   **CMake** 3.10+
-   **libcurl** (for API communication)
-   **Pthreads** (for multi-threaded MCP handling)

### Installation & Build

Follow these steps to build the project from source:

```bash
mkdir build
cd build
cmake ..
make clean
make -j$(nproc)
```

The build process generates two binaries:
1. `ollmcpc`: The main CLI client.
2. `mcp_server`: The tool host server.

## 📖 Usage

Run the client from the root directory:

```bash
./build/ollmcpc serve
```

### Commands

| Command | Description |
| :--- | :--- |
| `serve` | Start the interactive MCP session. |
| `config` | Run the interactive configuration wizard. |
| `list` | Show available models and providers. |
| `--help` | Show usage information. |

### Options (for `serve`)

-   `--provider <name>`: Override the default provider (`ollama`, `gemini`, or `manual`).
-   `--model <name>`: Override the default model (e.g., `llama3`, `gemini-pro`).

Example:
```bash
./build/ollmcpc serve --provider gemini --model gemini-1.5-flash
```

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

---
Built with ❤️ by the OLLMCPC Team.
