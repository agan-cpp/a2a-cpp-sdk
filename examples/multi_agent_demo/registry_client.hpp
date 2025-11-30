#pragma once

#include "agent_registry.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <stdexcept>
#include <thread>
#include <atomic>

using json = nlohmann::json;

// CURL 回调函数
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * @brief 注册中心客户端
 */
class RegistryClient {
public:
    explicit RegistryClient(const std::string& registry_url = "http://localhost:8500")
        : registry_url_(registry_url)
        , heartbeat_running_(false) {}
    
    ~RegistryClient() {
        stop_heartbeat();
    }
    
    // 注册 Agent
    bool register_agent(const AgentRegistration& registration) {
        json request = registration.to_json();
        auto response = post("/v1/agent/register", request.dump());
        
        if (response.contains("success") && response["success"].get<bool>()) {
            // 保存注册信息用于心跳
            current_registration_ = registration;
            // 启动心跳线程
            start_heartbeat();
            return true;
        }
        
        return false;
    }
    
    // 注销 Agent
    bool deregister_agent(const std::string& agent_id) {
        stop_heartbeat();
        
        json request = {{"id", agent_id}};
        auto response = post("/v1/agent/deregister", request.dump());
        
        return response.contains("success") && response["success"].get<bool>();
    }
    
    // 根据标签查找 Agent
    std::vector<AgentRegistration> find_agents_by_tag(const std::string& tag) {
        json request = {{"tag", tag}};
        auto response = post("/v1/agent/find", request.dump());
        
        std::vector<AgentRegistration> result;
        
        if (response.contains("agents") && response["agents"].is_array()) {
            for (const auto& agent_json : response["agents"]) {
                result.push_back(AgentRegistration::from_json(agent_json));
            }
        }
        
        return result;
    }
    
    // 获取所有 Agent
    std::vector<AgentRegistration> get_all_agents() {
        auto response = get("/v1/agents");
        
        std::vector<AgentRegistration> result;
        
        if (response.contains("agents") && response["agents"].is_array()) {
            for (const auto& agent_json : response["agents"]) {
                result.push_back(AgentRegistration::from_json(agent_json));
            }
        }
        
        return result;
    }
    
    // 选择一个 Agent（负载均衡：轮询）
    std::string select_agent_by_tag(const std::string& tag) {
        auto agents = find_agents_by_tag(tag);
        
        if (agents.empty()) {
            throw std::runtime_error("No agent found with tag: " + tag);
        }
        
        // 简单轮询
        static std::map<std::string, size_t> round_robin_index;
        size_t& index = round_robin_index[tag];
        
        std::string address = agents[index % agents.size()].address;
        index++;
        
        return address;
    }

private:
    // 发送 POST 请求
    json post(const std::string& path, const std::string& body) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }
        
        std::string url = registry_url_ + path;
        std::string response_body;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        
        CURLcode res = curl_easy_perform(curl);
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            throw std::runtime_error("CURL error: " + std::string(curl_easy_strerror(res)));
        }
        
        return json::parse(response_body);
    }
    
    // 发送 GET 请求
    json get(const std::string& path) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }
        
        std::string url = registry_url_ + path;
        std::string response_body;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            throw std::runtime_error("CURL error: " + std::string(curl_easy_strerror(res)));
        }
        
        return json::parse(response_body);
    }
    
    // 启动心跳线程
    void start_heartbeat() {
        if (heartbeat_running_) {
            return;
        }
        
        heartbeat_running_ = true;
        heartbeat_thread_ = std::thread([this]() {
            while (heartbeat_running_) {
                try {
                    json request = {{"id", current_registration_.id}};
                    post("/v1/agent/heartbeat", request.dump());
                } catch (const std::exception& e) {
                    // 忽略心跳错误
                }
                
                std::this_thread::sleep_for(std::chrono::seconds(10));
            }
        });
    }
    
    // 停止心跳线程
    void stop_heartbeat() {
        if (!heartbeat_running_) {
            return;
        }
        
        heartbeat_running_ = false;
        if (heartbeat_thread_.joinable()) {
            heartbeat_thread_.join();
        }
    }
    
    std::string registry_url_;
    AgentRegistration current_registration_;
    std::atomic<bool> heartbeat_running_;
    std::thread heartbeat_thread_;
};
