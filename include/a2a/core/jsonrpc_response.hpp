#pragma once

#include "error_code.hpp"
#include <string>
#include <optional>

namespace a2a {

/**
 * @brief JSON-RPC 2.0 Error object
 */
struct JsonRpcError {
    int32_t code;
    std::string message;
    std::string data;  // Optional additional data
    
    JsonRpcError() : code(0) {}
    JsonRpcError(int32_t c, const std::string& msg) 
        : code(c), message(msg) {}
    JsonRpcError(ErrorCode ec, const std::string& msg)
        : code(static_cast<int32_t>(ec)), message(msg) {}
};

/**
 * @brief JSON-RPC 2.0 Response
 */
class JsonRpcResponse {
public:
    JsonRpcResponse() = default;
    
    // Success response
    JsonRpcResponse(const std::string& id, const std::string& result_json)
        : jsonrpc_("2.0")
        , id_(id)
        , result_json_(result_json)
        , error_() {}
    
    // Error response
    JsonRpcResponse(const std::string& id, const JsonRpcError& error)
        : jsonrpc_("2.0")
        , id_(id)
        , result_json_()
        , error_(error) {}
    
    // Getters
    const std::string& jsonrpc() const { return jsonrpc_; }
    const std::string& id() const { return id_; }
    const std::optional<std::string>& result_json() const { return result_json_; }
    const std::optional<JsonRpcError>& error() const { return error_; }
    
    bool is_error() const { return error_.has_value(); }
    bool is_success() const { return result_json_.has_value(); }
    
    /**
     * @brief Serialize to JSON string
     */
    std::string to_json() const;
    
    /**
     * @brief Deserialize from JSON string
     */
    static JsonRpcResponse from_json(const std::string& json);
    
    /**
     * @brief Create error response
     */
    static JsonRpcResponse create_error(const std::string& id, 
                                       ErrorCode code, 
                                       const std::string& message);
    
    /**
     * @brief Create success response
     */
    static JsonRpcResponse create_success(const std::string& id, 
                                         const std::string& result_json);

private:
    std::string jsonrpc_ = "2.0";
    std::string id_;
    std::optional<std::string> result_json_;
    std::optional<JsonRpcError> error_;
};

} // namespace a2a
