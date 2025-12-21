cmake_minimum_required(VERSION 3.10)
project(ollmcpc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Includes
include_directories(src/include)

# Dependencies
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

# --- Library Modules ---
# Utils
set(UTILS_SRCS 
    src/src/utils/json.cpp
    src/src/utils/http.cpp
    src/src/utils/terminal.cpp
)

# LLM Providers
set(LLM_SRCS 
    src/src/llm/ollama.cpp
    src/src/llm/gemini.cpp
    src/src/llm/manual.cpp
)

# MCP (Client Side)
set(MCP_CLIENT_SRCS 
    src/src/mcp/client.cpp
    src/src/mcp/server_proxy.cpp
)

# App Core
set(APP_SRCS
    src/src/app/config.cpp
    src/src/app/interactive.cpp
)

# --- Executables ---

# 1. Main CLI (Client)
add_executable(ollmcpc 
    src/main.cpp
    ${UTILS_SRCS}
    ${LLM_SRCS}
    ${MCP_CLIENT_SRCS}
    ${APP_SRCS}
)
target_link_libraries(ollmcpc PRIVATE CURL::libcurl Threads::Threads)

# 2. Local MCP Server (Host for tools)
add_executable(mcp_server
    src/src/mcp/server_app.cpp
    src/src/utils/json.cpp  # Server needs JSON helper
)
target_link_libraries(mcp_server PRIVATE Threads::Threads)

# Installation
install(TARGETS ollmcpc mcp_server DESTINATION bin)
