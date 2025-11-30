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
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "registry_client.hpp"

using namespace a2a;
using json = nlohmann::json;

const std::string API_KEY = "sk-2663f076466a4c469f3b992c5e629c20";

class DynamicMathAgent {
public:
    DynamicMathAgent(const std::string& agent_id,
                    const std::string& listen_address,
                    const std::string& registry_url,
                    const std::string& redis_host,
                    int redis_port)
        : agent_id_(agent_id)
        , listen_address_(listen_address)
        , task_store_(std::make_shared<RedisTaskStore>(redis_host, redis_port))
        , qwen_client_(API_KEY)
        , registry_client_(registry_url) {
        
        std::cout << "[Math Agent] 初始化完成" << std::endl;
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
        
        std::cout << "[Math Agent] 启动在端口 " << port << std::endl;
        
        // 在后台线程中启动服务器
        std::thread server_thread([&server]() {
            server.start();
        });
        
        // 等待服务器启动
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // 注册到注册中心
        AgentRegistration registration;
        registration.id = agent_id_;
        registration.name = "Math Agent";
        registration.address = listen_address_;
        registration.tags = {"math", "calculator"};
        
        if (registry_client_.register_agent(registration)) {
            std::cout << "[Math Agent] 已注册到服务中心" << std::endl;
        } else {
            std::cerr << "[Math Agent] 注册失败" << std::endl;
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
                
                int history_length = 0;
                if (params_json.contains("historyLength")) {
                    history_length = params_json["historyLength"].get<int>();
                }
                
                std::cout << "[Math Agent] 收到消息: " << user_text 
                         << " (history_length=" << history_length << ")" << std::endl;
                
                // 获取历史
                auto history = task_store_->get_history(context_id, history_length);
                
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
                
                // 调用 AI
                std::string system_prompt = "你是一个数学专家，擅长解决各种数学问题。请直接给出答案，简洁明了。";
                std::string ai_response = qwen_client_.chat(system_prompt + "\n\n" + history_text, user_text);
                
                std::cout << "[Math Agent] AI 响应: " << ai_response << std::endl;
                
                // 返回响应
                auto response_msg = AgentMessage::create()
                    .with_role(MessageRole::Agent)
                    .with_context_id(context_id);
                response_msg.add_text_part(ai_response);
                
                auto response = JsonRpcResponse::create_success(request.id(), response_msg.to_json());
                return response.to_json();
            }
            
            return JsonRpcResponse::create_error(request.id(), ErrorCode::MethodNotFound, "Method not found").to_json();
            
        } catch (const std::exception& e) {
            std::cerr << "[Math Agent] 错误: " << e.what() << std::endl;
            return JsonRpcResponse::create_error("1", ErrorCode::InternalError, e.what()).to_json();
        }
    }
    
    std::string get_agent_card() {
        json card = {
            {"name", "Math Agent"},
            {"description", "数学计算专家，擅长各种数学问题求解"},
            {"version", "1.0.0"},
            {"capabilities", {
                {"streaming", false},
                {"push_notifications", false},
                {"task_management", true}
            }},
            {"skills", json::array({
                {
                    {"name", "数学计算"},
                    {"description", "执行各种数学运算，包括加减乘除、方程求解等"},
                    {"input_modes", json::array({"text"})},
                    {"output_modes", json::array({"text"})}
                },
                {
                    {"name", "上下文理解"},
                    {"description", "理解对话历史，支持引用之前的计算结果"},
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
        std::cerr << "示例: " << argv[0] << " math-1 5001 http://localhost:8500 127.0.0.1 6379" << std::endl;
        return 1;
    }
    
    std::string agent_id = argv[1];
    int port = std::stoi(argv[2]);
    std::string registry_url = argv[3];
    std::string redis_host = argc > 4 ? argv[4] : "127.0.0.1";
    int redis_port = argc > 5 ? std::stoi(argv[5]) : 6379;
    
    std::string listen_address = "http://localhost:" + std::to_string(port);
    
    try {
        DynamicMathAgent agent(agent_id, listen_address, registry_url, redis_host, redis_port);
        agent.start(port);
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
