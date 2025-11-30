#pragma once

#include "task_status.hpp"
#include "artifact.hpp"
#include "agent_message.hpp"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace a2a {

/**
 * @brief Agent Task - represents a task that can be processed by an agent
 */
class AgentTask {
public:
    AgentTask() = default;
    
    AgentTask(const std::string& id, const std::string& context_id)
        : id_(id)
        , context_id_(context_id)
        , status_(TaskState::Submitted) {}
    
    // Getters
    const std::string& id() const { return id_; }
    const std::string& context_id() const { return context_id_; }
    const AgentTaskStatus& status() const { return status_; }
    const std::vector<Artifact>& artifacts() const { return artifacts_; }
    const std::vector<AgentMessage>& history() const { return history_; }
    const std::map<std::string, std::string>& metadata() const { return metadata_; }
    
    // Setters
    void set_id(const std::string& id) { id_ = id; }
    void set_context_id(const std::string& context_id) { context_id_ = context_id; }
    void set_status(const AgentTaskStatus& status) { status_ = status; }
    void set_status(TaskState state) { status_ = AgentTaskStatus(state); }
    
    /**
     * @brief Add an artifact to the task
     */
    void add_artifact(const Artifact& artifact) {
        artifacts_.push_back(artifact);
    }
    
    /**
     * @brief Add a message to the history
     */
    void add_history_message(const AgentMessage& message) {
        history_.push_back(message);
    }
    
    /**
     * @brief Add metadata
     */
    void add_metadata(const std::string& key, const std::string& value) {
        metadata_[key] = value;
    }
    
    /**
     * @brief Check if task is in terminal state
     */
    bool is_terminal() const {
        return status_.is_terminal();
    }
    
    /**
     * @brief Serialize to JSON
     */
    std::string to_json() const;
    
    /**
     * @brief Deserialize from JSON
     */
    static AgentTask from_json(const std::string& json);
    
    /**
     * @brief Create a new AgentTask
     */
    static AgentTask create() {
        return AgentTask();
    }
    
    /**
     * @brief Fluent API methods
     */
    AgentTask& with_id(const std::string& id) {
        id_ = id;
        return *this;
    }
    
    AgentTask& with_context_id(const std::string& context_id) {
        context_id_ = context_id;
        return *this;
    }
    
    AgentTask& with_status(TaskState state) {
        status_ = AgentTaskStatus(state);
        return *this;
    }
    
    AgentTask& with_artifact(const Artifact& artifact) {
        artifacts_.push_back(artifact);
        return *this;
    }
    
    AgentTask& with_history_message(const AgentMessage& message) {
        history_.push_back(message);
        return *this;
    }

private:
    std::string id_;
    std::string context_id_;
    AgentTaskStatus status_;
    std::vector<Artifact> artifacts_;
    std::vector<AgentMessage> history_;
    std::map<std::string, std::string> metadata_;
};

} // namespace a2a
