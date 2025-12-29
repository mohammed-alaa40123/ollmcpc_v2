# LLM Providers

OLLMCPC uses a Strategy Pattern to support multiple LLM backends. All providers inherit from the `LLMProvider` base class.

## Base Class (`llm_provider.h`)

Defines the interface for communication:
*   `chat(messages, tools)`: Sends a prompt and available tools to the LLM.
*   `list_models()`: Returns a list of supported models.

## Ollama Provider (`ollama_provider.cpp`)

Connects to a local Ollama instance (usually at `http://localhost:11434`).

### Features:
*   Standard chat-completion API.
*   Native tool support (functions).
*   Streaming responses.

## Gemini Provider (`gemini_provider.cpp`)

Connects to Google's Gemini API.

### Features:
*   High latency but powerful reasoning.
*   Strict Tool-Calling schemas.
*   Requires an API Key.

## Manual Provider (`manual_provider.cpp`)

A special "debug" provider that doesn't use an LLM.

### Purpose:
*   Allows the user to manually select tools from a menu.
*   Perfect for testing if a new MCP tool is working correctly without spending tokens or waiting for local inference.

## Key Files:
*   `src/include/llm/llm_provider.h`: Abstract base class.
*   `src/src/llm/ollama_provider.cpp`: Ollama implementation.
*   `src/src/llm/gemini_provider.cpp`: Gemini implementation.
*   `src/src/llm/manual_provider.cpp`: Interactive tool selector.
