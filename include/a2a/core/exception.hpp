#pragma once

#include "error_code.hpp"
#include <stdexcept>
#include <string>

namespace a2a {

/**
 * @brief Exception class for A2A protocol errors
 */
class A2AException : public std::runtime_error {
public:
    /**
     * @brief Construct exception with message and error code
     */
    A2AException(const std::string& message, ErrorCode code)
        : std::runtime_error(message)
        , error_code_(code)
        , request_id_() {}
    
    /**
     * @brief Construct exception with message, error code, and request ID
     */
    A2AException(const std::string& message, ErrorCode code, const std::string& request_id)
        : std::runtime_error(message)
        , error_code_(code)
        , request_id_(request_id) {}
    
    /**
     * @brief Get the error code
     */
    ErrorCode error_code() const noexcept {
        return error_code_;
    }
    
    /**
     * @brief Get the request ID (if available)
     */
    const std::string& request_id() const noexcept {
        return request_id_;
    }
    
    /**
     * @brief Get error code as integer
     */
    int32_t error_code_value() const noexcept {
        return static_cast<int32_t>(error_code_);
    }

private:
    ErrorCode error_code_;
    std::string request_id_;
};

} // namespace a2a
