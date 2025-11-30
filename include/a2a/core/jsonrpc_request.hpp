#pragma once

#include <string>
#include <optional>
#include <memory>

namespace a2a {

/**
 * @brief JSON-RPC 2.0 Request
 */
class JsonRpcRequest {
public:
    JsonRpcRequest() = default;
    
    JsonRpcRequest(const std::string& id, 
                   const std::string& method,
                   const std::string& params_json = "{}")
        : jsonrpc_("2.0")
        , id_(id)
        , method_(method)
        , params_json_(params_json) {}
    
    // Getters
    const std::string& jsonrpc() const { return jsonrpc_; }
    const std::string& id() const { return id_; }
    const std::string& method() const { return method_; }
    const std::string& params_json() const { return params_json_; }
    
    // Setters
    void set_id(const std::string& id) { id_ = id; }
    void set_method(const std::string& method) { method_ = method; }
    void set_params_json(const std::string& params) { params_json_ = params; }
    
    /**
     * @brief Serialize to JSON string
     */
    std::string to_json() const;
    
    /**
     * @brief Deserialize from JSON string
     */
    static JsonRpcRequest from_json(const std::string& json);

private:
    std::string jsonrpc_ = "2.0";
    std::string id_;
    std::string method_;
    std::string params_json_ = "{}";
};

} // namespace a2a
