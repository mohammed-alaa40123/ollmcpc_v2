# Troubleshooting

Common issues and how to solve them.

## Installation Issues

### Missing `libcurl`
If `cmake` fails with a "Could not find CURL" error:
```bash
sudo apt-get install libcurl4-openssl-dev
```

### Compiler too old
OLLMCPC requires C++17. If your compiler is too old:
```bash
sudo apt-get install gcc-11 g++-11
```
Then run cmake with: `CXX=g++-11 cmake ..`

## Runtime Issues

### Ollama Connection Refused / Couldn't Connect to Server
If you see:
```
curl_easy_perform() failed: Couldn't connect to server
```
or
```
Ollama Connection Refused
```

This means Ollama isn't running. Start it with:
```bash
ollama serve
```

To run it in the background:
```bash
nohup ollama serve > /tmp/ollama.log 2>&1 &
```

### Architecture Mismatch (qemu errors)
If you see errors like:
```
qemu-x86_64: Could not open '/lib64/ld-linux-x86-64.so.2': No such file or directory
```

This means the binaries were compiled for a different CPU architecture (x86_64 vs ARM64). **You must rebuild on your own machine**:
```bash
cd build
rm -rf *
cmake ..
make -j$(nproc)
sudo cp ollmcpc mcp_server /usr/local/bin/
```

### Server Error: "npx not found"
External servers often require Node.js. Ensure it's installed and in your PATH:
```bash
node -v
npm -v
```

### Tool Execution Fails
Check `debug.log` in the root directory. It contains all JSON-RPC traffic between OLLMCPC and the MCP servers.

```bash
tail -f debug.log
```

### "Failed to exec server" for os-assistant
If you see this error on startup:
```
Failed to exec server
Failed to initialize
```

This means the internal `mcp_server` binary cannot be found. Solutions:

1. **If installed via setup script**: The binary should be in `/usr/local/bin/mcp_server`. Verify with:
   ```bash
   which mcp_server
   ```

2. **If running from build directory**: Use the full path:
   ```bash
   ./build/ollmcpc serve
   ```
   Or ensure `mcp_server` is in your PATH.

3. **Rebuild and reinstall**:
   ```bash
   cd build && make -j$(nproc)
   sudo cp mcp_server /usr/local/bin/
   sudo cp ollmcpc /usr/local/bin/
   ```

## Protocol Issues

### "Method not found"
This usually means the MCP server doesn't support a specific feature. Ensure you are using a compatible version of the server.

### HIL Prompt not appearing
Check your configuration (`~/.ollmcpc.json`) and ensure `"human_in_loop": true` is set. Note that "Manual Mode" bypasses HIL prompts as you are already manually selecting tools.
