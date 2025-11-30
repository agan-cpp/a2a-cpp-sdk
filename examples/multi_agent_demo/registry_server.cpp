#include "agent_registry.hpp"
#include "http_server.hpp"
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <csignal>

using json = nlohmann::json;

// CURL 回调
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// 获取 Agent Card
json fetch_agent_card(const std::string& agent_address) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return json::object();
    }
    
    std::string url = agent_address + "/.well-known/agent-card.json";
    std::string response_body;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        std::cerr << "[Registry] 无法获取 Agent Card from " << url << ": " 
                  << curl_easy_strerror(res) << std::endl;
        return json::object();
    }
    
    try {
        return json::parse(response_body);
    } catch (const std::exception& e) {
        std::cerr << "[Registry] 解析 Agent Card 失败: " << e.what() << std::endl;
        return json::object();
    }
}

// 全局注册中心实例
std::unique_ptr<AgentRegistry> g_registry;

void signal_handler(int signal) {
    std::cout << "\n[Registry Server] 收到信号 " << signal << "，正在关闭..." << std::endl;
    exit(0);
}

/**
 * @brief 注册中心服务器
 */
class RegistryServer {
public:
    explicit RegistryServer(int port = 8500)
        : port_(port)
        , registry_(std::make_unique<AgentRegistry>(30, 60)) {
        std::cout << "[Registry Server] 初始化完成" << std::endl;
    }
    
    void start() {
        std::cout << "[Registry Server] 启动在端口 " << port_ << std::endl;
        
        HttpServer server(port_);
        
        // 注册 Agent
        server.register_handler("/v1/agent/register", 
            [this](const std::string& body) {
                return this->handle_register(body);
            }
        );
        
        // 注销 Agent
        server.register_handler("/v1/agent/deregister", 
            [this](const std::string& body) {
                return this->handle_deregister(body);
            }
        );
        
        // 心跳
        server.register_handler("/v1/agent/heartbeat", 
            [this](const std::string& body) {
                return this->handle_heartbeat(body);
            }
        );
        
        // 查询 Agent（按标签）
        server.register_handler("/v1/agent/find", 
            [this](const std::string& body) {
                return this->handle_find(body);
            }
        );
        
        // 获取所有 Agent
        server.register_handler("/v1/agents", 
            [this](const std::string&) {
                return this->handle_list_all();
            }
        );
        
        // 健康检查线程
        std::thread health_check_thread([this]() {
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(30));
                registry_->check_health();
            }
        });
        health_check_thread.detach();
        
        server.start();
    }

private:
    std::string handle_register(const std::string& body) {
        try {
            auto j = json::parse(body);
            auto registration = AgentRegistration::from_json(j);
            
            // 获取 Agent Card (A2A 协议标准)
            std::cout << "[Registry] 正在获取 Agent Card from " << registration.address << std::endl;
            registration.agent_card = fetch_agent_card(registration.address);
            
            if (!registration.agent_card.empty()) {
                std::cout << "[Registry] ✅ 成功获取 Agent Card" << std::endl;
            } else {
                std::cout << "[Registry] ⚠️  未能获取 Agent Card，使用基本信息" << std::endl;
            }
            
            bool success = registry_->register_agent(registration);
            
            std::cout << "[Registry] 注册 Agent: " << registration.name 
                      << " (" << registration.id << ") at " << registration.address << std::endl;
            
            json response = {
                {"success", success},
                {"message", success ? "Agent registered successfully" : "Failed to register agent"}
            };
            
            return response.dump();
            
        } catch (const std::exception& e) {
            json response = {
                {"success", false},
                {"error", e.what()}
            };
            return response.dump();
        }
    }
    
    std::string handle_deregister(const std::string& body) {
        try {
            auto j = json::parse(body);
            std::string agent_id = j.at("id").get<std::string>();
            
            bool success = registry_->deregister_agent(agent_id);
            
            std::cout << "[Registry] 注销 Agent: " << agent_id << std::endl;
            
            json response = {
                {"success", success},
                {"message", success ? "Agent deregistered successfully" : "Agent not found"}
            };
            
            return response.dump();
            
        } catch (const std::exception& e) {
            json response = {
                {"success", false},
                {"error", e.what()}
            };
            return response.dump();
        }
    }
    
    std::string handle_heartbeat(const std::string& body) {
        try {
            auto j = json::parse(body);
            std::string agent_id = j.at("id").get<std::string>();
            
            bool success = registry_->heartbeat(agent_id);
            
            json response = {
                {"success", success}
            };
            
            return response.dump();
            
        } catch (const std::exception& e) {
            json response = {
                {"success", false},
                {"error", e.what()}
            };
            return response.dump();
        }
    }
    
    std::string handle_find(const std::string& body) {
        try {
            auto j = json::parse(body);
            std::string tag = j.at("tag").get<std::string>();
            
            auto agents = registry_->find_agents_by_tag(tag);
            
            json result = json::array();
            for (const auto& agent : agents) {
                result.push_back(agent.to_json());
            }
            
            json response = {
                {"success", true},
                {"agents", result},
                {"count", agents.size()}
            };
            
            return response.dump();
            
        } catch (const std::exception& e) {
            json response = {
                {"success", false},
                {"error", e.what()}
            };
            return response.dump();
        }
    }
    
    std::string handle_list_all() {
        try {
            auto agents = registry_->get_all_agents();
            
            json result = json::array();
            for (const auto& agent : agents) {
                result.push_back(agent.to_json());
            }
            
            json response = {
                {"success", true},
                {"agents", result},
                {"count", agents.size()}
            };
            
            return response.dump();
            
        } catch (const std::exception& e) {
            json response = {
                {"success", false},
                {"error", e.what()}
            };
            return response.dump();
        }
    }
    
    int port_;
    std::unique_ptr<AgentRegistry> registry_;
};

int main() {
    // 设置信号处理
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    try {
        RegistryServer server(8500);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "[Registry Server] 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
