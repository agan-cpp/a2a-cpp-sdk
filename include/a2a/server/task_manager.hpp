#pragma once

#include "task_store.hpp"
#include "../models/agent_task.hpp"
#include "../models/agent_card.hpp"
#include "../models/agent_message.hpp"
#include "../models/message_send_params.hpp"
#include "../models/a2a_response.hpp"
#include <functional>
#include <memory>
#include <string>

namespace a2a {

/**
 * @brief Task Manager - manages the complete lifecycle of agent tasks
 */
class TaskManager {
public:
    /**
     * @brief Callback type for message received
     * Returns either AgentMessage or AgentTask
     */
    using MessageCallback = std::function<A2AResponse(const MessageSendParams&)>;
    
    /**
     * @brief Callback type for task lifecycle events
     */
    using TaskCallback = std::function<void(const AgentTask&)>;
    
    /**
     * @brief Callback type for agent card query
     */
    using AgentCardCallback = std::function<AgentCard(const std::string& agent_url)>;
    
    /**
     * @brief Construct with optional custom task store
     */
    explicit TaskManager(std::shared_ptr<ITaskStore> task_store = nullptr);
    
    ~TaskManager();
    
    // Disable copy, enable move
    TaskManager(const TaskManager&) = delete;
    TaskManager& operator=(const TaskManager&) = delete;
    TaskManager(TaskManager&&) noexcept;
    TaskManager& operator=(TaskManager&&) noexcept;
    
    // === Lifecycle Callbacks ===
    
    /**
     * @brief Set callback for when a message is received
     * This is the main handler for agent logic
     */
    void set_on_message_received(MessageCallback callback);
    
    /**
     * @brief Set callback for when a task is created
     */
    void set_on_task_created(TaskCallback callback);
    
    /**
     * @brief Set callback for when a task is cancelled
     */
    void set_on_task_cancelled(TaskCallback callback);
    
    /**
     * @brief Set callback for when a task is updated
     */
    void set_on_task_updated(TaskCallback callback);
    
    /**
     * @brief Set callback for agent card queries
     */
    void set_on_agent_card_query(AgentCardCallback callback);
    
    // === Task Operations ===
    
    /**
     * @brief Create a new task
     * @param context_id Optional context ID (generated if not provided)
     * @param task_id Optional task ID (generated if not provided)
     * @return Created task
     */
    AgentTask create_task(const std::string& context_id = "",
                         const std::string& task_id = "");
    
    /**
     * @brief Get a task by ID
     * @param task_id Task identifier
     * @return Task if found
     * @throws A2AException if not found
     */
    AgentTask get_task(const std::string& task_id);
    
    /**
     * @brief Cancel a task
     * @param task_id Task identifier
     * @return Updated task
     * @throws A2AException if task cannot be cancelled
     */
    AgentTask cancel_task(const std::string& task_id);
    
    /**
     * @brief Update task status
     * @param task_id Task identifier
     * @param status New status
     * @param message Optional message
     */
    void update_status(const std::string& task_id,
                      TaskState status,
                      const AgentMessage* message = nullptr);
    
    /**
     * @brief Add artifact to task
     * @param task_id Task identifier
     * @param artifact Artifact to add
     */
    void return_artifact(const std::string& task_id,
                        const Artifact& artifact);
    
    // === Message Processing ===
    
    /**
     * @brief Process a message (non-streaming)
     * @param params Message parameters
     * @return Response (Task or Message)
     */
    A2AResponse send_message(const MessageSendParams& params);
    
    /**
     * @brief Process a message (streaming)
     * @param params Message parameters
     * @param callback Called for each event
     */
    void send_message_streaming(const MessageSendParams& params,
                               std::function<void(const std::string&)> callback);
    
    /**
     * @brief Get agent card
     * @param agent_url Agent URL
     * @return AgentCard
     */
    AgentCard get_agent_card(const std::string& agent_url);
    
    /**
     * @brief Get the task store
     * @return Shared pointer to task store
     */
    std::shared_ptr<ITaskStore> get_task_store() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace a2a
