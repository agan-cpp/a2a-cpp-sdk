#pragma once

#include "agent_message.hpp"
#include <optional>

namespace a2a {

/**
 * @brief Parameters for sending a message to an agent
 */
class MessageSendParams {
public:
    MessageSendParams() = default;
    
    explicit MessageSendParams(const AgentMessage& message)
        : message_(message) {}
    
    // Getters
    const AgentMessage& message() const { return message_; }
    const std::optional<int>& history_length() const { return history_length_; }
    const std::optional<std::string>& context_id() const { return context_id_; }
    const std::optional<std::string>& task_id() const { return task_id_; }
    
    // Setters
    void set_message(const AgentMessage& message) { message_ = message; }
    void set_history_length(int length) { history_length_ = length; }
    void set_context_id(const std::string& id) { context_id_ = id; }
    void set_task_id(const std::string& id) { task_id_ = id; }
    
    /**
     * @brief Serialize to JSON
     */
    std::string to_json() const;
    
    /**
     * @brief Deserialize from JSON
     */
    static MessageSendParams from_json(const std::string& json);
    
    /**
     * @brief Create a new MessageSendParams
     */
    static MessageSendParams create() {
        return MessageSendParams();
    }
    
    /**
     * @brief Fluent API methods
     */
    MessageSendParams& with_message(const AgentMessage& message) {
        message_ = message;
        return *this;
    }
    
    MessageSendParams& with_history_length(int length) {
        history_length_ = length;
        return *this;
    }
    
    MessageSendParams& with_context_id(const std::string& id) {
        context_id_ = id;
        return *this;
    }
    
    MessageSendParams& with_task_id(const std::string& id) {
        task_id_ = id;
        return *this;
    }

private:
    AgentMessage message_;
    std::optional<int> history_length_;
    std::optional<std::string> context_id_;
    std::optional<std::string> task_id_;
};

/**
 * @brief Parameters for querying a task
 */
struct TaskQueryParams {
    std::string id;
    std::optional<int> history_length;
    std::map<std::string, std::string> metadata;
    
    std::string to_json() const;
    static TaskQueryParams from_json(const std::string& json);
};

/**
 * @brief Parameters for task ID operations
 */
struct TaskIdParams {
    std::string id;
    
    std::string to_json() const;
    static TaskIdParams from_json(const std::string& json);
};

} // namespace a2a
