#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @brief Agent 注册信息
 */
struct AgentRegistration {
    std::string id;              // Agent 唯一 ID
    std::string name;            // Agent 名称
    std::string address;         // Agent 地址 (http://host:port)
    std::vector<std::string> tags;  // Agent 标签 (如 "math", "translation")
    std::chrono::system_clock::time_point last_heartbeat;  // 最后心跳时间
    json agent_card;             // Agent Card (A2A 协议标准)
    
    // 序列化
    json to_json() const {
        json j = {
            {"id", id},
            {"name", name},
            {"address", address},
            {"tags", tags},
            {"last_heartbeat", std::chrono::system_clock::to_time_t(last_heartbeat)}
        };
        if (!agent_card.empty()) {
            j["agent_card"] = agent_card;
        }
        return j;
    }
    
    // 反序列化
    static AgentRegistration from_json(const json& j) {
        AgentRegistration reg;
        reg.id = j.at("id").get<std::string>();
        reg.name = j.at("name").get<std::string>();
        reg.address = j.at("address").get<std::string>();
        reg.tags = j.at("tags").get<std::vector<std::string>>();
        reg.last_heartbeat = std::chrono::system_clock::now();
        if (j.contains("agent_card")) {
            reg.agent_card = j["agent_card"];
        }
        return reg;
    }
};

/**
 * @brief Agent 注册中心
 */
class AgentRegistry {
public:
    explicit AgentRegistry(int heartbeat_timeout_sec = 30, int cleanup_interval_sec = 60)
        : heartbeat_timeout_(heartbeat_timeout_sec)
        , cleanup_interval_(cleanup_interval_sec) {}
    
    // 注册 Agent
    bool register_agent(const AgentRegistration& registration) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto& reg = agents_[registration.id];
        reg = registration;
        reg.last_heartbeat = std::chrono::system_clock::now();
        
        // 按标签索引
        for (const auto& tag : registration.tags) {
            tags_index_[tag].insert(registration.id);
        }
        
        return true;
    }
    
    // 注销 Agent
    bool deregister_agent(const std::string& agent_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = agents_.find(agent_id);
        if (it == agents_.end()) {
            return false;
        }
        
        // 从标签索引中移除
        for (const auto& tag : it->second.tags) {
            tags_index_[tag].erase(agent_id);
        }
        
        agents_.erase(it);
        return true;
    }
    
    // 心跳
    bool heartbeat(const std::string& agent_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = agents_.find(agent_id);
        if (it == agents_.end()) {
            return false;
        }
        
        it->second.last_heartbeat = std::chrono::system_clock::now();
        return true;
    }
    
    // 根据标签查找 Agent
    std::vector<AgentRegistration> find_agents_by_tag(const std::string& tag) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<AgentRegistration> result;
        
        auto tag_it = tags_index_.find(tag);
        if (tag_it == tags_index_.end()) {
            return result;
        }
        
        for (const auto& agent_id : tag_it->second) {
            auto agent_it = agents_.find(agent_id);
            if (agent_it != agents_.end()) {
                result.push_back(agent_it->second);
            }
        }
        
        return result;
    }
    
    // 获取所有 Agent
    std::vector<AgentRegistration> get_all_agents() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<AgentRegistration> result;
        for (const auto& pair : agents_) {
            result.push_back(pair.second);
        }
        return result;
    }
    
    // 健康检查，移除超时的 Agent
    void check_health() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::system_clock::now();
        std::vector<std::string> to_remove;
        
        for (const auto& pair : agents_) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - pair.second.last_heartbeat).count();
            
            if (elapsed > heartbeat_timeout_) {
                to_remove.push_back(pair.first);
            }
        }
        
        // 移除超时的 Agent
        for (const auto& agent_id : to_remove) {
            auto it = agents_.find(agent_id);
            if (it != agents_.end()) {
                // 从标签索引中移除
                for (const auto& tag : it->second.tags) {
                    tags_index_[tag].erase(agent_id);
                }
                agents_.erase(it);
            }
        }
    }
    
private:
    std::mutex mutex_;
    std::map<std::string, AgentRegistration> agents_;  // agent_id -> registration
    std::map<std::string, std::set<std::string>> tags_index_;  // tag -> agent_ids
    int heartbeat_timeout_;  // 心跳超时时间（秒）
    int cleanup_interval_;   // 清理间隔（秒）
};
