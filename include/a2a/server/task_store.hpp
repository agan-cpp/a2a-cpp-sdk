#pragma once

#include "../models/agent_task.hpp"
#include <string>
#include <optional>
#include <memory>

namespace a2a {

/**
 * @brief Interface for storing and retrieving agent tasks
 */
class ITaskStore {
public:
    virtual ~ITaskStore() = default;
    
    /**
     * @brief Retrieve a task by its ID
     * @param task_id Task identifier
     * @return Task if found, nullopt otherwise
     */
    virtual std::optional<AgentTask> get_task(const std::string& task_id) = 0;
    
    /**
     * @brief Store or update a task
     * @param task Task to store
     */
    virtual void set_task(const AgentTask& task) = 0;
    
    /**
     * @brief Update task status
     * @param task_id Task identifier
     * @param status New status
     * @param message Optional message associated with status
     */
    virtual void update_status(const std::string& task_id,
                              TaskState status,
                              const std::string& message = "") = 0;
    
    /**
     * @brief Add artifact to task
     * @param task_id Task identifier
     * @param artifact Artifact to add
     */
    virtual void add_artifact(const std::string& task_id,
                             const Artifact& artifact) = 0;
    
    /**
     * @brief Add message to task history
     * @param task_id Task identifier
     * @param message Message to add
     */
    virtual void add_history_message(const std::string& task_id,
                                    const AgentMessage& message) = 0;
    
    /**
     * @brief Get history messages for a task/context
     * @param context_id Context identifier (or task_id)
     * @param max_length Maximum number of messages to return (0 = all)
     * @return Vector of history messages
     */
    virtual std::vector<AgentMessage> get_history(const std::string& context_id,
                                                   int max_length = 0) = 0;
    
    /**
     * @brief Delete a task
     * @param task_id Task identifier
     * @return true if task was deleted, false if not found
     */
    virtual bool delete_task(const std::string& task_id) = 0;
    
    /**
     * @brief Check if task exists
     * @param task_id Task identifier
     * @return true if task exists
     */
    virtual bool task_exists(const std::string& task_id) = 0;
};

} // namespace a2a
