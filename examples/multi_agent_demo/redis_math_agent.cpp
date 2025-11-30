#include <a2a/server/task_manager.hpp>
#include <a2a/core/exception.hpp>
#include <a2a/core/jsonrpc_request.hpp>
#include <a2a/core/jsonrpc_response.hpp>
#include "redis_task_store.hpp"
#include "qwen_client.hpp"
#include "http_server.hpp"
#include <iostream>
#include <memory>
#include <csignal>

using namespace a2a;

/**
 * @brief Math Agent (使用 Redis TaskStore)
 * 独立进程，从 Redis 获取历史
 */
class RedisMathAgent {
public:
    explicit RedisMathAgent(const std::string& api_key,
                            std::shared_ptr<ITaskStore> task_store,
                            int port = 5001)
        : task_manager_(task_store)
        , qwen_client_(api_key, "qwen-plus")
        , port_(port) {
        
        task_manager_.set_on_message_received(
            [this](const MessageSendParams& params) {
                return this->handle_message(params);
            }
        );
        
        task_manager_.set_on_agent_card_query(
            [this](const std::string& agent_url) {
                return this->get_agent_card(agent_url);
            }
        );
        
        std::cout << "[Math Agent] 初始化完成（使用 Redis TaskStore）" << std::endl;
    }
    
    void start() {
        std::cout << "[Math Agent] 启动在端口 " << port_ << std::endl;
        
        HttpServer server(port_);
        
        server.register_handler("/", 
            [this](const std::string& request_body) {
                return this->handle_http_request(request_body);
            }
        );
        
        server.register_handler("/.well-known/agent-card.json",
            [this](const std::string&) {
                auto card = task_manager_.get_agent_card(
                    "http://localhost:" + std::to_string(port_)
                );
                return card.to_json();
            }
        );
        
        server.start();
    }

private:
    A2AResponse handle_message(const MessageSendParams& params) {
        const auto& message = params.message();
        std::string user_query = message.get_text();
        
        std::cout << "[Math Agent] 收到查询: " << user_query << std::endl;
        
        if (params.history_length().has_value()) {
            std::cout << "[Math Agent] 请求历史长度: " 
                     << *params.history_length() << std::endl;
        }
        
        try {
            std::string system_prompt = 
                "你是一个专业的数学计算助手。"
                "如果用户提到'上述结果'、'刚才的答案'、'上面答案'等，请根据对话历史理解上下文。";
            
            // ✅ 从 Redis TaskStore 获取历史
            std::string full_context = user_query;
            if (message.context_id().has_value() && params.history_length().has_value()) {
                auto context_id = *message.context_id();
                int history_length = *params.history_length();
                
                auto task_store = task_manager_.get_task_store();
                if (task_store) {
                    auto history = task_store->get_history(context_id, history_length);
                    
                    if (!history.empty()) {
                        std::cout << "[Math Agent] 从 Redis TaskStore 获取到 " 
                                 << history.size() << " 条历史消息" << std::endl;
                        
                        std::string history_text = "对话历史：\n";
                        for (const auto& hist_msg : history) {
                            if (hist_msg.role() == MessageRole::User) {
                                history_text += "用户: " + hist_msg.get_text() + "\n";
                            } else {
                                history_text += "助手: " + hist_msg.get_text() + "\n";
                            }
                        }
                        
                        full_context = history_text + "\n当前问题: " + user_query;
                        std::cout << "[Math Agent] 包含历史的查询长度: " 
                                 << full_context.length() << " 字节" << std::endl;
                    } else {
                        std::cout << "[Math Agent] Redis TaskStore 中没有历史记录" << std::endl;
                    }
                }
            }
            
            std::string ai_response = qwen_client_.chat(system_prompt, full_context);
            
            auto reply = AgentMessage::create()
                .with_role(MessageRole::Agent)
                .with_text(ai_response);
            
            if (message.context_id().has_value()) {
                reply.set_context_id(*message.context_id());
            }
            
            return A2AResponse(reply);
            
        } catch (const std::exception& e) {
            auto error_reply = AgentMessage::create()
                .with_role(MessageRole::Agent)
                .with_text("抱歉，处理数学问题时出错");
            return A2AResponse(error_reply);
        }
    }
    
    AgentCard get_agent_card(const std::string& agent_url) {
        auto card = AgentCard::create()
            .with_name("Math Agent (Redis TaskStore)")
            .with_description("专业的数学计算助手（使用 Redis）")
            .with_url(agent_url)
            .with_version("3.0.0");
        card.set_protocol_version("1.0");
        return card;
    }
    
    std::string handle_http_request(const std::string& request_body) {
        try {
            auto jsonrpc_req = JsonRpcRequest::from_json(request_body);
            auto params = MessageSendParams::from_json(jsonrpc_req.params_json());
            
            auto response = handle_message(params);
            
            std::string result_json;
            if (response.is_message()) {
                result_json = response.as_message().to_json();
            } else {
                result_json = response.as_task().to_json();
            }
            
            auto jsonrpc_res = JsonRpcResponse::create_success(
                jsonrpc_req.id(),
                result_json
            );
            return jsonrpc_res.to_json();
            
        } catch (const std::exception& e) {
            auto error_res = JsonRpcResponse::create_error(
                "error",
                ErrorCode::InternalError,
                e.what()
            );
            return error_res.to_json();
        }
    }
    
    TaskManager task_manager_;
    QwenClient qwen_client_;
    int port_;
};

void signal_handler(int signal) {
    std::cout << "\n[Math Agent] 收到信号 " << signal << "，正在关闭..." << std::endl;
    exit(0);
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    try {
        std::string api_key = "own—key";
        std::string redis_host = "127.0.0.1";
        int redis_port = 6379;
        int port = 5001;
        
        if (argc > 1) {
            port = std::stoi(argv[1]);
        }
        if (argc > 2) {
            redis_host = argv[2];
        }
        if (argc > 3) {
            redis_port = std::stoi(argv[3]);
        }
        
        // ✅ 创建 Redis TaskStore
        auto redis_task_store = std::make_shared<RedisTaskStore>(redis_host, redis_port);
        std::cout << "[Main] 创建 Redis TaskStore: " << redis_host << ":" << redis_port << std::endl;
        
        RedisMathAgent agent(api_key, redis_task_store, port);
        agent.start();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
