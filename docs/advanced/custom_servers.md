# Custom MCP Servers

You can easily extend OLLMCPC by adding your own MCP servers.

## Popular MCP Servers

The Model Context Protocol has a growing ecosystem. You can find many servers on GitHub or via NPM.

### Examples:
*   **PostgreSQL**: `npx -y @modelcontextprotocol/server-postgres postgres://...`
*   **Google Maps**: `npx -y @modelcontextprotocol/server-google-maps`
*   **GitHub**: `npx -y @modelcontextprotocol/server-github`

## Adding to OLLMCPC

To add a new server, edit your `~/.ollmcpc.json` and add a new entry to the `servers` array.

```json
{
  "name": "my-sql-server",
  "command": ["npx", "-y", "@modelcontextprotocol/server-postgres", "postgresql://localhost/mydb"],
  "enabled": true
}
```

## Creating Your Own Server

If you want to create a tool specifically for your workflow:

1.  **Choose a Language**: Node.js, Python, and Go have excellent MCP SDKs.
2.  **Implement JSON-RPC**: Your server must speak JSON-RPC 2.0 over `stdin`/`stdout`.
3.  **Define Tools**: Expose the tools your LLM needs.
4.  **Register it**: Add it to OLLMCPC configuration.

### Tool Naming
OLLMCPC automatically prefixes your tools with the server name to avoid collisions.
*   Server: `filesystem`
*   Tool: `read_file`
*   LLM sees: `filesystem_read_file`
