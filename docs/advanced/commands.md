# Commands & Toggling

OLLMCPC provides several internal commands to manage your session in real-time.

## Slash Commands

In the interactive chat, any line starting with `/` is treated as a command rather than a prompt for the LLM.

### `/help`
Displays a list of all available slash commands and their descriptions.

### `/servers`
Lists all configured MCP servers and their current status:
*   **Active**: The server is running and communicating.
*   **Disabled**: The server is configured but turned off by the user.
*   **Error**: The server failed to start or lost connection.

### `/toggle <name>`
Enables or disables a specific server by its name.
*   **Example**: `/toggle filesystem`
*   **Effect**: If the server was active, it will be shut down. If it was disabled, it will be launched and initialized.

### `/exit`
Safely shuts down all MCP servers and exits the application.

## Human-in-the-Loop (HIL)

Safety is a core principle of OLLMCPC. When HIL is enabled (default), the client will intercept every tool execution request.

### Approval Prompt
When the LLM wants to run a tool, you will see a colored box showing:
*   The tool name.
*   The arguments the LLM wants to pass.

You can then:
*   **Press Enter**: Approve the execution.
*   **Type 'no'**: Deny the execution (the LLM will receive a "User denied execution" error).
*   **Type 'edit'**: (Planned) Modify the arguments before execution.
