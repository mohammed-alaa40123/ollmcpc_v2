# Application Layer

The Application layer manages the user interaction and high-level orchestration of the system.

## Entry Point (`main.cpp`)

Located in `src/main.cpp`. It uses a custom CLI parser to route commands:
*   `serve`: Starts the interactive chat session.
*   `config`: Starts the configuration wizard.
*   `list`: Lists available models from the configured providers.

## Interactive Mode (`interactive.cpp`)

The "Brain" of the UI, found in `src/app/interactive.cpp`.

### Responsibilities:
*   **Chat Loop**: Keeps the conversation going until the user exits.
*   **Slash Commands**: Processes commands like `/help`, `/servers`, and `/toggle`.
*   **HIL Logic**: Implements the Human-in-the-Loop check before any tool is executed.
*   **Terminal Styling**: Uses `utils/terminal.h` to provide a rich, colorized output.

## Configuration Manager (`config.cpp`)

Found in `src/app/config.cpp`.

### Features:
*   **JSON Persistence**: Saves and loads settings from `~/.ollmcpc.json`.
*   **Interactive Setup**: Provides a terminal-based UI for initial configuration.
*   **Default Injection**: Ensures healthy defaults are present if the config file is missing or corrupted.

## Key Files:
*   `src/include/app/interactive.h`: UI definitions.
*   `src/include/app/config.h`: Configuration structures.
*   `src/src/app/interactive.cpp`: Implementation of the UI logic.
*   `src/src/app/config.cpp`: Implementation of persistence logic.
