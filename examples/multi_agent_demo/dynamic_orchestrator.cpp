#include "redis_task_store.hpp"
#include "qwen_client.hpp"
#include "http_server.hpp"
#include <a2a/models/agent_message.hpp>
#include <a2a/models/agent_task.hpp>
#include <a2a/models/task_status.hpp>
#include <a2a/models/message_part.hpp>
#include <a2a/core/jsonrpc_request.hpp>
#include <a2a/core/jsonrpc_response.hpp>
#include <a2a/core/error_code.hpp>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "registry_client.hpp"

using namespace a2a;
using json = nlohmann::json;

// 简单的 HTTP 客户端
class SimpleHttpClient {
public:
    static std::string post(const std::string& url, const std::string& body) {
        CURL* curl = curl_easy_init();
        if (!curl) return "";
        
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
        curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        return response;
    }
};

const std::string API_KEY = "own—key";

class DynamicOrchestrator {
public:
    DynamicOrchestrator(const std::string& agent_id,
                       const std::string& listen_address,
                       const std::string& registry_url,
                       const std::string& redis_host,
                       int redis_port)
        : agent_id_(agent_id)
        , listen_address_(listen_address)
        , task_store_(std::make_shared<RedisTaskStore>(redis_host, redis_port))
        , qwen_client_(API_KEY)
        , registry_client_(registry_url) {
        
        std::cout << "[Orchestrator] 初始化完成" << std::endl;
    }
    
    void start(int port) {
        // 启动 HTTP 服务器
        HttpServer server(port);
        
        // A2A 协议端点
        server.register_handler("/", [this](const std::string& body) {
            return this->handle_request(body);
        });
        
        // Agent Card 端点 (A2A 协议标准)
        server.register_handler("/.well-known/agent-card.json", [this](const std::string& body) {
            return this->get_agent_card();
        });
        
        std::cout << "[Orchestrator] 启动在端口 " << port << std::endl;
        
        // 在后台线程中启动服务器
        std::thread server_thread([&server]() {
            server.start();
        });
        
        // 等待服务器启动
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // 注册到注册中心
        AgentRegistration registration;
        registration.id = agent_id_;
        registration.name = "Orchestrator";
        registration.address = listen_address_;
        registration.tags = {"orchestrator", "coordinator"};
        
        if (registry_client_.register_agent(registration)) {
            std::cout << "[Orchestrator] 已注册到服务中心" << std::endl;
        } else {
            std::cerr << "[Orchestrator] 注册失败" << std::endl;
        }
        
        server_thread.join();
    }

private:
    std::string handle_request(const std::string& body) {
        try {
            auto request_json = json::parse(body);
            auto request = JsonRpcRequest::from_json(body);
            
            if (request.method() == "message/send") {
                auto params_json = request_json["params"];
                auto message = AgentMessage::from_json(params_json["message"].dump());
                
                // 获取文本内容
                std::string user_text;
                if (!message.parts().empty()) {
                    auto text_part = dynamic_cast<TextPart*>(message.parts()[0].get());
                    if (text_part) {
                        user_text = text_part->text();
                    }
                }
                
                std::string context_id = message.context_id().value_or("default");
                
                std::cout << "[Orchestrator] 收到消息: " << user_text << std::endl;
                
                // 保存用户消息
                save_message(context_id, message);
                
                // 识别意图
                std::string intent = analyze_intent(user_text);
                std::cout << "[Orchestrator] 识别意图: " << intent << std::endl;
                
                std::string response_text;
                
                if (intent == "math") {
                    // 动态查找 Math Agent
                    response_text = call_math_agent(user_text, context_id);
                } else {
                    // 通用对话
                    response_text = handle_general_query(user_text, context_id);
                }
                
                // 保存 Agent 响应
                auto response_msg = AgentMessage::create()
                    .with_role(MessageRole::Agent)
                    .with_context_id(context_id);
                response_msg.add_text_part(response_text);
                save_message(context_id, response_msg);
                
                // 返回响应
                auto response = JsonRpcResponse::create_success(request.id(), response_msg.to_json());
                return response.to_json();
            }
            
            return JsonRpcResponse::create_error(request.id(), ErrorCode::MethodNotFound, "Method not found").to_json();
            
        } catch (const std::exception& e) {
            std::cerr << "[Orchestrator] 错误: " << e.what() << std::endl;
            return JsonRpcResponse::create_error("1", ErrorCode::InternalError, e.what()).to_json();
        }
    }
    
    std::string analyze_intent(const std::string& text) {
        std::string prompt = "判断以下用户输入属于哪个类别，只回答类别名称：\n"
                           "- math: 数学计算、方程求解\n"
                           "- general: 其他对话\n\n"
                           "用户输入: " + text;
        
        std::string result = qwen_client_.chat("", prompt);
        
        if (result.find("math") != std::string::npos) {
            return "math";
        }
        return "general";
    }
    
    std::string call_math_agent(const std::string& query, const std::string& context_id) {
        try {
            // 从注册中心查找 Math Agent
            std::string math_agent_url = registry_client_.select_agent_by_tag("math");
            
            std::cout << "[Orchestrator] 调用 Math Agent: " << math_agent_url << std::endl;
            
            // 构造请求
            json request = {
                {"jsonrpc", "2.0"},
                {"id", "1"},
                {"method", "message/send"},
                {"params", {
                    {"message", {
                        {"role", "user"},
                        {"contextId", context_id},
                        {"parts", {{{"kind", "text"}, {"text", query}}}}
                    }},
                    {"historyLength", 5}
                }}
            };
            
            // 发送请求
            std::string response_body = SimpleHttpClient::post(math_agent_url, request.dump());
            auto response_json = json::parse(response_body);
            
            if (response_json.contains("result") && 
                response_json["result"].contains("parts") &&
                !response_json["result"]["parts"].empty()) {
                return response_json["result"]["parts"][0]["text"].get<std::string>();
            }
            
            return "无法解析响应";
            
        } catch (const std::exception& e) {
            std::cerr << "[Orchestrator] 调用 Math Agent 失败: " << e.what() << std::endl;
            return "抱歉，数学服务暂时不可用";
        }
    }
    
    std::string handle_general_query(const std::string& query, const std::string& context_id) {
        auto history = task_store_->get_history(context_id, 5);
        std::string history_text;
        for (const auto& msg : history) {
            std::string role_str = to_string(msg.role());
            std::string text;
            if (!msg.parts().empty()) {
                auto text_part = dynamic_cast<TextPart*>(msg.parts()[0].get());
                if (text_part) {
                    text = text_part->text();
                }
            }
            history_text += role_str + ": " + text + "\n";
        }
        
        return qwen_client_.chat(history_text, query);
    }
    
    void save_message(const std::string& context_id, const AgentMessage& message) {
        if (!task_store_->task_exists(context_id)) {
            auto task = AgentTask::create()
                .with_id(context_id)
                .with_context_id(context_id)
                .with_status(TaskState::Running);
            task_store_->set_task(task);
        }
        task_store_->add_history_message(context_id, message);
    }
    
    std::string get_agent_card() {
        json card = {
            {"name", "Orchestrator Agent"},
            {"description", "智能协调器，负责意图识别和任务分发"},
            {"version", "1.0.0"},
            {"capabilities", {
                {"streaming", false},
                {"push_notifications", false},
                {"task_management", true}
            }},
            {"skills", json::array({
                {
                    {"name", "意图识别"},
                    {"description", "识别用户意图并路由到相应的专业 Agent"},
                    {"input_modes", json::array({"text"})},
                    {"output_modes", json::array({"text"})}
                },
                {
                    {"name", "任务协调"},
                    {"description", "协调多个 Agent 完成复杂任务"},
                    {"input_modes", json::array({"text"})},
                    {"output_modes", json::array({"text"})}
                }
            })},
            {"provider", {
                {"name", "A2A Demo"},
                {"organization", "A2A C++ SDK"}
            }}
        };
        return card.dump();
    }
    
    std::string agent_id_;
    std::string listen_address_;
    std::shared_ptr<RedisTaskStore> task_store_;
    QwenClient qwen_client_;
    RegistryClient registry_client_;
};

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "用法: " << argv[0] << " <agent_id> <port> <registry_url> [redis_host] [redis_port]" << std::endl;
        std::cerr << "示例: " << argv[0] << " orch-1 5000 http://localhost:8500 127.0.0.1 6379" << std::endl;
        return 1;
    }
    
    std::string agent_id = argv[1];
    int port = std::stoi(argv[2]);
    std::string registry_url = argv[3];
    std::string redis_host = argc > 4 ? argv[4] : "127.0.0.1";
    int redis_port = argc > 5 ? std::stoi(argv[5]) : 6379;
    
    std::string listen_address = "http://localhost:" + std::to_string(port);
    
    try {
        DynamicOrchestrator orchestrator(agent_id, listen_address, registry_url, redis_host, redis_port);
        orchestrator.start(port);
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
