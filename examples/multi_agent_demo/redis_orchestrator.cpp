#include <a2a/server/task_manager.hpp>
#include <a2a/client/a2a_client.hpp>
#include <a2a/core/exception.hpp>
#include <a2a/core/jsonrpc_request.hpp>
#include <a2a/core/jsonrpc_response.hpp>
#include "redis_task_store.hpp"
#include "qwen_client.hpp"
#include "http_server.hpp"
#include <iostream>
#include <memory>
#include <algorithm>
#include <csignal>

using namespace a2a;

/**
 * @brief Orchestrator Agent (使用 Redis TaskStore)
 * 独立进程，通过 Redis 共享历史
 */
class RedisOrchestrator {
public:
    explicit RedisOrchestrator(const std::string& api_key,
                               std::shared_ptr<ITaskStore> task_store,
                               int port = 5000)
        : task_manager_(task_store)
        , qwen_client_(api_key, "qwen-plus")
        , port_(port)
        , max_history_length_(10) {
        
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
        
        std::cout << "[Orchestrator] 初始化完成（使用 Redis TaskStore）" << std::endl;
    }
    
    void start() {
        std::cout << "[Orchestrator] 启动在端口 " << port_ << std::endl;
        
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
    
    A2AResponse handle_message(const MessageSendParams& params) {
        const auto& message = params.message();
        std::string user_query = message.get_text();
        auto context_id = message.context_id();
        
        std::cout << "[Orchestrator] 收到用户查询: " << user_query << std::endl;
        
        // ✅ 保存用户消息到 Redis TaskStore
        if (context_id) {
            save_message_to_taskstore(*context_id, message);
        }
        
        // 识别意图
        std::string intent = identify_intent(user_query);
        std::cout << "[Orchestrator] 识别意图: " << intent << std::endl;
        
        std::string response_text;
        
        if (intent == "math") {
            response_text = call_math_agent(user_query, context_id);
        } else {
            response_text = call_general_chat(user_query);
        }
        
        auto reply = AgentMessage::create()
            .with_role(MessageRole::Agent)
            .with_text(response_text);
        
        if (context_id) {
            reply.set_context_id(*context_id);
        }
        
        // ✅ 保存 Agent 回复到 Redis TaskStore
        if (context_id) {
            save_message_to_taskstore(*context_id, reply);
        }
        
        return A2AResponse(reply);
    }
    
    void save_message_to_taskstore(const std::string& context_id, 
                                   const AgentMessage& message) {
        auto task_store = task_manager_.get_task_store();
        if (!task_store) {
            return;
        }
        
        if (!task_store->task_exists(context_id)) {
            auto task = AgentTask::create()
                .with_id(context_id)
                .with_context_id(context_id)
                .with_status(TaskState::Submitted);
            task_store->set_task(task);
            std::cout << "[Orchestrator] 创建新 Task: " << context_id << std::endl;
        }
        
        task_store->add_history_message(context_id, message);
        std::cout << "[Orchestrator] 保存消息到 Redis TaskStore" << std::endl;
    }
    
    std::string identify_intent(const std::string& query) {
        std::string lower_query = query;
        std::transform(lower_query.begin(), lower_query.end(), 
                      lower_query.begin(), ::tolower);
        
        if (lower_query.find("+") != std::string::npos ||
            lower_query.find("-") != std::string::npos ||
            lower_query.find("*") != std::string::npos ||
            lower_query.find("/") != std::string::npos ||
            lower_query.find("计算") != std::string::npos ||
            lower_query.find("等于") != std::string::npos ||
            lower_query.find("答案") != std::string::npos ||
            lower_query.find("结果") != std::string::npos) {
            return "math";
        }
        
        return "general";
    }
    
    std::string call_math_agent(const std::string& query,
                                const std::optional<std::string>& context_id) {
        try {
            // 调用本地 Math Agent
            A2AClient client("http://localhost:5001");
            
            auto message = AgentMessage::create()
                .with_role(MessageRole::User)
                .with_text(query);
            
            if (context_id) {
                message.set_context_id(*context_id);
            }
            
            auto params = MessageSendParams::create()
                .with_message(message)
                .with_history_length(max_history_length_);
            
            std::cout << "[Orchestrator] 调用 Math Agent (history_length=" 
                     << max_history_length_ << ")" << std::endl;
            
            auto response = client.send_message(params);
            
            if (response.is_message()) {
                return response.as_message().get_text();
            }
            return "Math Agent 返回了任务";
            
        } catch (const std::exception& e) {
            return "调用 Math Agent 失败: " + std::string(e.what());
        }
    }
    
    std::string call_general_chat(const std::string& query) {
        try {
            std::string system_prompt = "你是一个智能助手。";
            return qwen_client_.chat(system_prompt, query);
        } catch (const std::exception& e) {
            return "处理请求失败: " + std::string(e.what());
        }
    }
    
    AgentCard get_agent_card(const std::string& agent_url) {
        auto card = AgentCard::create()
            .with_name("Orchestrator (Redis TaskStore)")
            .with_description("智能调度助手（使用 Redis）")
            .with_url(agent_url)
            .with_version("4.0.0");
        card.set_protocol_version("1.0");
        return card;
    }
    
    TaskManager task_manager_;
    QwenClient qwen_client_;
    int port_;
    int max_history_length_;
};

void signal_handler(int signal) {
    std::cout << "\n[Orchestrator] 收到信号 " << signal << "，正在关闭..." << std::endl;
    exit(0);
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    try {
        std::string api_key = "own—key";
        std::string redis_host = "127.0.0.1";
        int redis_port = 6379;
        int port = 5000;
        
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
        
        RedisOrchestrator agent(api_key, redis_task_store, port);
        agent.start();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
