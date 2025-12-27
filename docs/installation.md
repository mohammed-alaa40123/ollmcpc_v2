# Installation Guide

Follow these steps to set up OLLMCPC on your local machine.

## Prerequisites

OLLMCPC requires a Linux environment (Ubuntu or Debian recommended).

### System Packages

Install the necessary build tools and libraries:

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libcurl4-openssl-dev \
    libssl-dev \
    curl \
    nodejs \
    npm
```

### LLM Backends

*   **Ollama (Recommended for local LLM)**:
    Install via their official script:
    ```bash
    curl -fsSL https://ollama.com/install.sh | sh
    ```
*   **Gemini**:
    No local installation required, but you will need an API Key from the [Google AI Studio](https://aistudio.google.com/).

---

## Build from Source

Clone the repository and build using CMake:

```bash
git clone https://github.com/mohammed-alaa40123/ollmcpc_v2.git
cd ollmcpc_v2
mkdir build && cd build
cmake ..
make -j$(nproc)
```

This generates two main binaries in the `build/` directory:
1.  `ollmcpc`: The interactive client application.
2.  `mcp_server`: The internal provider of system tools.

## Automated Setup

For a faster workflow, you can use the provided setup script:

```bash
chmod +x scripts/setup.sh
./scripts/setup.sh
```

The script will:
1.  Check and install missing system dependencies.
2.  Install Ollama if not present.
3.  Build the project.
4.  Pull the recommended models (`functiongemma` and `qwen3:0.6b`).
5.  Create a default configuration file.
6.  Install the binaries to `/usr/local/bin`.
