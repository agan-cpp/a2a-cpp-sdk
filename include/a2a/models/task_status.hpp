#pragma once

#include "../core/types.hpp"
#include <string>
#include <chrono>

namespace a2a {

/**
 * @brief Agent Task Status
 */
class AgentTaskStatus {
public:
    AgentTaskStatus() 
        : state_(TaskState::Submitted)
        , timestamp_(std::chrono::system_clock::now()) {}
    
    explicit AgentTaskStatus(TaskState state)
        : state_(state)
        , timestamp_(std::chrono::system_clock::now()) {}
    
    AgentTaskStatus(TaskState state, const Timestamp& timestamp)
        : state_(state)
        , timestamp_(timestamp) {}
    
    // Getters
    TaskState state() const { return state_; }
    const Timestamp& timestamp() const { return timestamp_; }
    const std::string& message() const { return message_; }
    
    // Setters
    void set_state(TaskState state) { state_ = state; }
    void set_timestamp(const Timestamp& ts) { timestamp_ = ts; }
    void set_message(const std::string& msg) { message_ = msg; }
    
    /**
     * @brief Check if task is in terminal state
     */
    bool is_terminal() const {
        return state_ == TaskState::Completed ||
               state_ == TaskState::Failed ||
               state_ == TaskState::Canceled ||
               state_ == TaskState::Rejected;
    }
    
    /**
     * @brief Serialize to JSON
     */
    std::string to_json() const;
    
    /**
     * @brief Deserialize from JSON
     */
    static AgentTaskStatus from_json(const std::string& json);

private:
    TaskState state_;
    Timestamp timestamp_;
    std::string message_;  // Optional status message
};

} // namespace a2a
