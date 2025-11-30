#pragma once

#include <cstdint>

namespace a2a {

/**
 * @brief A2A error codes following JSON-RPC 2.0 specification
 */
enum class ErrorCode : int32_t {
    // JSON-RPC standard errors
    ParseError = -32700,
    InvalidRequest = -32600,
    MethodNotFound = -32601,
    InvalidParams = -32602,
    InternalError = -32603,
    
    // A2A specific errors
    TaskNotFound = -32001,
    TaskNotCancelable = -32002,
    UnsupportedOperation = -32003,
    ContentTypeNotSupported = -32004,
    PushNotificationNotSupported = -32005
};

/**
 * @brief Convert error code to string description
 */
inline const char* error_code_to_string(ErrorCode code) {
    switch (code) {
        case ErrorCode::ParseError:
            return "Parse error";
        case ErrorCode::InvalidRequest:
            return "Invalid request";
        case ErrorCode::MethodNotFound:
            return "Method not found";
        case ErrorCode::InvalidParams:
            return "Invalid params";
        case ErrorCode::InternalError:
            return "Internal error";
        case ErrorCode::TaskNotFound:
            return "Task not found";
        case ErrorCode::TaskNotCancelable:
            return "Task not cancelable";
        case ErrorCode::UnsupportedOperation:
            return "Unsupported operation";
        case ErrorCode::ContentTypeNotSupported:
            return "Content type not supported";
        case ErrorCode::PushNotificationNotSupported:
            return "Push notification not supported";
        default:
            return "Unknown error";
    }
}

} // namespace a2a
