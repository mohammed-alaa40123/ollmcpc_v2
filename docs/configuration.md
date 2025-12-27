# Configuration

OLLMCPC configuration is stored in a JSON file at `~/.ollmcpc.json`.

## Configuration Wizard

You can initialize or update your configuration interactively:

```bash
./build/ollmcpc config
```

This will prompt you for:
*   Default LLM provider.
*   Ollama model name.
*   Gemini API Key and model name.
*   Human-in-the-Loop (HIL) toggle.

## Manual Configuration

Heres an example of a complete `~/.ollmcpc.json` file:

```json
{
  "default_provider": "ollama",
  "ollama_model": "functiongemma",
  "gemini_api_key": "YOUR_API_KEY_HERE",
  "gemini_model": "gemini-1.5-flash",
  "human_in_loop": true,
  "servers": [
    {
      "name": "filesystem",
      "command": ["npx", "-y", "@modelcontextprotocol/server-filesystem", "/home/user/documents"],
      "enabled": true
    },
    {
      "name": "web-browsing",
      "command": ["npx", "-y", "mcp-fetch-server"],
      "enabled": true
    }
  ]
}
```

### Options Explained

| Key | Type | Description |
| :--- | :--- | :--- |
| `default_provider` | string | Either `ollama`, `gemini`, or `manual`. |
| `ollama_model` | string | The model name in your local Ollama library. |
| `gemini_api_key` | string | Your Google AI Studio API key. |
| `gemini_model` | string | The specific Gemini model to use (e.g., `gemini-1.5-pro`). |
| `human_in_loop` | boolean | If true, asks for permission before executing tools. |
| `servers` | array | A list of external MCP servers to launch. |

### Server Configuration

Each object in the `servers` array requires:
*   `name`: A unique identifier for the server.
*   `command`: An array of strings representing the command to run (e.g., `["npx", "mcp-server-name"]`).
*   `enabled`: (Optional) If set to `false`, the server won't be started automatically.
