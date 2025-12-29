#pragma once

#include <string>

// =============================================================================
// JSON-RPC 2.0 Protocol Module
// =============================================================================
//
// This module handles JSON-RPC 2.0 message building and parsing.
// Used for communication between MCP clients and servers.
//
// Spec: https://www.jsonrpc.org/specification
//
// =============================================================================

namespace jsonrpc {

// =============================================================================
// JSON-RPC Message Building
// =============================================================================

/// Create a JSON-RPC request message
/// @param id Unique request identifier
/// @param method Method name to call
/// @param params JSON-formatted parameters (object or array)
/// @return Complete JSON-RPC request string
std::string request(int id, const std::string& method, const std::string& params);

/// Create a JSON-RPC notification (request without id - no response expected)
/// @param method Method name
/// @param params JSON-formatted parameters
/// @return Complete JSON-RPC notification string
std::string notification(const std::string& method, const std::string& params);

/// Create a JSON-RPC success response
/// @param id Request ID being responded to
/// @param result JSON-formatted result
/// @return Complete JSON-RPC response string
std::string response(int id, const std::string& result);

/// Create a JSON-RPC error response
/// @param id Request ID being responded to
/// @param code Error code
/// @param message Error message
/// @return Complete JSON-RPC error response string
std::string error(int id, int code, const std::string& message);

// =============================================================================
// JSON-RPC Message Parsing
// =============================================================================

/// Parsed JSON-RPC request structure
struct Request {
    int id = 0;           // Request ID (0 if notification)
    std::string method;   // Method name
    std::string params;   // Raw JSON params string
    bool is_notification = false;
};

/// Parsed JSON-RPC response structure  
struct Response {
    int id = 0;            // Response ID
    std::string result;    // Result JSON string (if success)
    std::string error;     // Error JSON string (if error)
    bool is_error = false;
};

/// Parse a JSON-RPC request from a JSON string
Request parse_request(const std::string& json);

/// Parse a JSON-RPC response from a JSON string
Response parse_response(const std::string& json);

/// Extract the result object from a JSON-RPC response
/// Convenience function that handles both success and nested result structures
std::string extract_result(const std::string& json);

} // namespace jsonrpc
