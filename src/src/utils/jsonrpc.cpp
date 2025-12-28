// =============================================================================
// JSON-RPC 2.0 Protocol Implementation
// =============================================================================

#include "utils/jsonrpc.hpp"
#include "utils/json.hpp"
#include <map>

namespace jsonrpc {

// =============================================================================
// Message Building
// =============================================================================

std::string request(int id, const std::string& method, const std::string& params) {
    std::map<std::string, std::string> msg;
    msg["jsonrpc"] = json::str("2.0");
    msg["id"] = json::num(id);
    msg["method"] = json::str(method);
    msg["params"] = params;
    return json::obj(msg);
}

std::string notification(const std::string& method, const std::string& params) {
    std::map<std::string, std::string> msg;
    msg["jsonrpc"] = json::str("2.0");
    msg["method"] = json::str(method);
    msg["params"] = params;
    return json::obj(msg);
}

std::string response(int id, const std::string& result) {
    std::map<std::string, std::string> msg;
    msg["jsonrpc"] = json::str("2.0");
    msg["id"] = json::num(id);
    msg["result"] = result;
    return json::obj(msg);
}

std::string error(int id, int code, const std::string& message) {
    std::map<std::string, std::string> err;
    err["code"] = json::num(code);
    err["message"] = json::str(message);
    
    std::map<std::string, std::string> msg;
    msg["jsonrpc"] = json::str("2.0");
    msg["id"] = json::num(id);
    msg["error"] = json::obj(err);
    return json::obj(msg);
}

// =============================================================================
// Message Parsing
// =============================================================================

Request parse_request(const std::string& json) {
    Request req;
    
    // Extract id (may not exist for notifications)
    std::string id_str = json_parse::extract_val(json, "\"id\":");
    if (!id_str.empty()) {
        req.id = std::stoi(id_str);
    } else {
        req.is_notification = true;
    }
    
    // Extract method
    req.method = json::parse::get_string(json, "method");
    
    // Extract params (either object or array)
    req.params = json::parse::get_object(json, "params");
    if (req.params == "{}") {
        // Maybe it's an array
        std::string arr = json::parse::get_array(json, "params");
        if (!arr.empty()) {
            req.params = arr;
        }
    }
    
    return req;
}

Response parse_response(const std::string& json) {
    Response resp;
    
    // Extract id
    std::string id_str = json_parse::extract_val(json, "\"id\":");
    if (!id_str.empty()) {
        resp.id = std::stoi(id_str);
    }
    
    // Check for error
    if (json::parse::has_key(json, "error")) {
        resp.is_error = true;
        resp.error = json::parse::get_object(json, "error");
    }
    
    // Extract result
    resp.result = json::parse::get_object(json, "result");
    
    return resp;
}

std::string extract_result(const std::string& json) {
    // First try to get result object directly
    std::string result = json::parse::get_object(json, "result");
    if (result != "{}") {
        return result;
    }
    
    // Maybe result is the whole response
    return json;
}

} // namespace jsonrpc
