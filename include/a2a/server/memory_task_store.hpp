#pragma once

#include "task_store.hpp"
#include <map>
#include <mutex>

namespace a2a {

/**
 * @brief In-memory implementation of ITaskStore
 * Thread-safe using mutex
 */
class MemoryTaskStore : public ITaskStore {
public:
    MemoryTaskStore() = default;
    ~MemoryTaskStore() override = default;
    
    // ITaskStore implementation
    std::optional<AgentTask> get_task(const std::string& task_id) override;
    
    void set_task(const AgentTask& task) override;
    
    void update_status(const std::string& task_id,
                      TaskState status,
                      const std::string& message = "") override;
    
    void add_artifact(const std::string& task_id,
                     const Artifact& artifact) override;
    
    void add_history_message(const std::string& task_id,
                            const AgentMessage& message) override;
    
    std::vector<AgentMessage> get_history(const std::string& context_id,
                                          int max_length = 0) override;
    
    bool delete_task(const std::string& task_id) override;
    
    bool task_exists(const std::string& task_id) override;
    
    /**
     * @brief Get number of tasks in store
     */
    size_t size() const;
    
    /**
     * @brief Clear all tasks
     */
    void clear();

private:
    mutable std::mutex mutex_;
    std::map<std::string, AgentTask> tasks_;
};

} // namespace a2a
