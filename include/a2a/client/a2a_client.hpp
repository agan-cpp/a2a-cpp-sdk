#pragma once

#include "../models/agent_task.hpp"
#include "../models/agent_message.hpp"
#include "../models/message_send_params.hpp"
#include "../models/a2a_response.hpp"
#include "../core/http_client.hpp"
#include <string>
#include <memory>
#include <functional>

namespace a2a {

/**
 * @brief Main A2A Client for communicating with agents
 */
class A2AClient {
public:
    /**
     * @brief Construct client with agent base URL
     * @param base_url Base URL of the agent service
     */
    explicit A2AClient(const std::string& base_url);
    
    ~A2AClient();
    
    // Disable copy, enable move
    A2AClient(const A2AClient&) = delete;
    A2AClient& operator=(const A2AClient&) = delete;
    A2AClient(A2AClient&&) noexcept;
    A2AClient& operator=(A2AClient&&) noexcept;
    
    /**
     * @brief Send a non-streaming message request
     * @param params Message parameters
     * @return A2AResponse containing Task or Message
     * @throws A2AException on error
     */
    A2AResponse send_message(const MessageSendParams& params);
    
    /**
     * @brief Send a streaming message request
     * @param params Message parameters
     * @param callback Called for each event received (Task, Message, or status update)
     * @throws A2AException on error
     */
    void send_message_streaming(const MessageSendParams& params,
                               std::function<void(const std::string&)> callback);
    
    /**
     * @brief Get a task by ID
     * @param task_id Task identifier
     * @return AgentTask object
     * @throws A2AException if task not found
     */
    AgentTask get_task(const std::string& task_id);
    
    /**
     * @brief Cancel a task
     * @param task_id Task identifier
     * @return Updated AgentTask
     * @throws A2AException if task cannot be cancelled
     */
    AgentTask cancel_task(const std::string& task_id);
    
    /**
     * @brief Subscribe to task updates (streaming)
     * @param task_id Task identifier
     * @param callback Called for each event received
     * @throws A2AException on error
     */
    void subscribe_to_task(const std::string& task_id,
                          std::function<void(const std::string&)> callback);
    
    /**
     * @brief Set request timeout
     * @param seconds Timeout in seconds
     */
    void set_timeout(long seconds);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace a2a
