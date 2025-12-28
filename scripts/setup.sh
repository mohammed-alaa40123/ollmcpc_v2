#!/bin/bash
# setup.sh - Robust Local installer for OLLMCPC

set -e

check_pkg() {
    dpkg -s "$1" >/dev/null 2>&1
}

echo "🛠 Checking system dependencies..."
MISSING_PKGS=""
for pkg in build-essential cmake libcurl4-openssl-dev libssl-dev curl nodejs npm; do
    if ! check_pkg "$pkg"; then
        MISSING_PKGS="$MISSING_PKGS $pkg"
    fi
done

if [ -n "$MISSING_PKGS" ]; then
    echo "⬇️  Installing missing packages: $MISSING_PKGS"
    sudo apt-get update
    sudo apt-get install -y $MISSING_PKGS
else
    echo "✅ All system packages are already installed."
fi

# Install Ollama
if ! command -v ollama &> /dev/null; then
    echo "⬇️  Ollama not found. Installing..."
    curl -fsSL https://ollama.com/install.sh | sh
else
    echo "✅ Ollama already installed."
fi

# Build
echo "🏗  Building OLLMCPC..."
mkdir -p build
cd build
cmake ..
make -j$(nproc)
cd ..

# Pull Models
echo "🧬 Pulling models..."
# Check if ollama is running, if not start it temporarily
if ! pgrep -x "ollama" > /dev/null; then
    ollama serve &
    OLLAMA_PID=$!
    sleep 5
fi

ollama pull qwen3:0.6b
ollama pull functiongemma

if [ -n "$OLLAMA_PID" ]; then
    kill $OLLAMA_PID
fi

# Create Default Config
echo "⚙️  Configuring default environment..."
CONFIG_FILE="$HOME/.ollmcpc.json"
if [ ! -f "$CONFIG_FILE" ]; then
    printf '{\n  "default_provider": "ollama",\n  "ollama_model": "functiongemma:latest",\n  "gemini_api_key": "",\n  "gemini_model": "gemini-3-flash-preview",\n  "human_in_loop": true,\n  "servers": [\n    {\n      "name": "filesystem",\n      "command": ["npx", "-y", "@modelcontextprotocol/server-filesystem", "'$HOME'"],\n      "enabled": true\n    },\n    {\n      "name": "web-browsing",\n      "command": ["npx", "-y", "mcp-fetch-server"],\n      "enabled": true\n    }\n  ]\n}' > "$CONFIG_FILE"
    echo "✅ Created $CONFIG_FILE"
else
    echo "ℹ️  $CONFIG_FILE already exists, skipping creation."
fi

# Global Installation
echo "🚀 Installing to /usr/local/bin..."
sudo cp build/ollmcpc /usr/local/bin/
sudo cp build/mcp_server /usr/local/bin/
sudo chmod +x /usr/local/bin/ollmcpc
sudo chmod +x /usr/local/bin/mcp_server

echo ""
echo "✨ SETUP COMPLETE!"
echo "Usage like Ollama:"
echo "  ollmcpc list"
echo "  ollmcpc help"
echo "  ollmcpc serve"
echo ""
echo "Or just type 'ollmcpc' to start the default session."
