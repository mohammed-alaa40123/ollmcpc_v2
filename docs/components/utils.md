# Utilities

OLLMCPC includes a set of lightweight utilities to minimize external dependencies.

## CURL Wrapper (`curl_utils.cpp`)

A thin, thread-safe wrapper around `libcurl` for making HTTP requests to Ollama and Gemini APIs.

## Terminal Styling (`terminal.h`)

Provides macros and functions for:
*   ANSI color output.
*   Box drawing.
*   Progress indicators (simplified).
*   Bold/Italic/Underline text.

## JSON Handling (`json_utils.h`)

While the project uses `nlohmann/json` for complex parsing, the `utils` folder contains helper functions for:
*   Safe value extraction.
*   Pretty-printing JSON logic specifically for the terminal.

## Logging (`logger.h`)

A simple logger that outputs to `debug.log`. It can be configured for different levels:
*   `DEBUG`: Protocol messages, raw JSON-RPC.
*   `INFO`: Normal operation.
*   `ERROR`: Connection failures or protocol violations.

## Key Files:
*   `src/include/utils/curl_utils.h`: Network communication.
*   `src/include/utils/terminal.h`: Aesthetic output tools.
*   `src/include/utils/logger.h`: Debugging and monitoring.
