// echo_agent_with_server.cpp
// 带 HTTP 服务器的完整 Echo Agent 示例
// 
// 注意：这个示例需要额外的 HTTP 服务器库（如 cpp-httplib）
// 当前项目没有包含 HTTP 服务器实现，这只是演示代码

#include <a2a/server/task_manager.hpp>
#include <a2a/server/memory_task_store.hpp>
#include <a2a/core/exception.hpp>
#include <iostream>
#include <string>

// 注意：需要安装 cpp-httplib
// sudo apt-get install libhttplib-dev
// 或者从 https://github.com/yhirose/cpp-httplib 下载
// #include <httplib.h>

using namespace a2a;

int main() {
    std::cout << "=== A2A C++ Echo Agent with HTTP Server ===" << std::endl << std::endl;
    
    try {
        // 1. 创建任务管理器
        auto task_store = std::make_shared<MemoryTaskStore>();
        TaskManager task_manager(task_store);
        
        // 2. 设置消息处理器
        task_manager.set_on_message_received([](const MessageSendParams& params) -> A2AResponse {
            std::cout << "Received message: " << params.message().get_text() << std::endl;
            
            auto response = AgentMessage::create()
                .with_role(MessageRole::Agent)
                .with_text("Echo: " + params.message().get_text());
            
            return A2AResponse(response);
        });
        
        // 3. 设置 Agent Card 处理器
        task_manager.set_on_agent_card_query([](const std::string& agent_url) -> AgentCard {
            AgentCapabilities caps;
            caps.streaming = true;
            caps.task_management = true;
            
            return AgentCard::create()
                .with_name("Echo Agent")
                .with_description("A simple echo agent that repeats your messages")
                .with_url(agent_url)
                .with_version("1.0.0")
                .with_capabilities(caps);
        });
        
        std::cout << "Echo Agent initialized successfully!" << std::endl;
        
        // 4. 启动 HTTP 服务器（需要 cpp-httplib）
        /*
        httplib::Server server;
        
        // 处理 Agent Card 请求
        server.Get("/a2a/agent.card", [&](const httplib::Request& req, httplib::Response& res) {
            try {
                auto card = task_manager.get_agent_card("http://localhost:5000");
                res.set_content(card.to_json(), "application/json");
            } catch (const std::exception& e) {
                res.status = 500;
                res.set_content(e.what(), "text/plain");
            }
        });
        
        // 处理消息请求
        server.Post("/a2a", [&](const httplib::Request& req, httplib::Response& res) {
            try {
                // 解析 JSON-RPC 请求
                auto json_req = nlohmann::json::parse(req.body);
                std::string method = json_req["method"];
                
                if (method == "a2a.message.send") {
                    // 解析参数
                    auto params = MessageSendParams::from_json(json_req["params"]);
                    
                    // 调用处理器
                    auto response = task_manager.send_message(params);
                    
                    // 构建 JSON-RPC 响应
                    nlohmann::json json_res = {
                        {"jsonrpc", "2.0"},
                        {"result", response.to_json()},
                        {"id", json_req["id"]}
                    };
                    
                    res.set_content(json_res.dump(), "application/json");
                } else {
                    // 方法不存在
                    nlohmann::json error_res = {
                        {"jsonrpc", "2.0"},
                        {"error", {
                            {"code", -32601},
                            {"message", "Method not found"}
                        }},
                        {"id", json_req["id"]}
                    };
                    res.status = 404;
                    res.set_content(error_res.dump(), "application/json");
                }
            } catch (const std::exception& e) {
                res.status = 500;
                res.set_content(e.what(), "text/plain");
            }
        });
        
        std::cout << "Starting HTTP server on http://localhost:5000" << std::endl;
        std::cout << "Press Ctrl+C to stop..." << std::endl << std::endl;
        
        // 启动服务器（阻塞）
        server.listen("0.0.0.0", 5000);
        */
        
        // 当前版本没有 HTTP 服务器，只能自测试
        std::cout << "Note: This version doesn't include HTTP server." << std::endl;
        std::cout << "To enable HTTP server, install cpp-httplib and uncomment the code above." << std::endl;
        std::cout << std::endl;
        
        // 自测试
        std::cout << "Running self-test..." << std::endl;
        auto test_message = AgentMessage::create()
            .with_role(MessageRole::User)
            .with_text("Hello, Echo Agent!");
        
        auto test_params = MessageSendParams::create()
            .with_message(test_message);
        
        auto response = task_manager.send_message(test_params);
        
        if (response.is_message()) {
            std::cout << "Response: " << response.as_message().get_text() << std::endl;
        }
        
        return 0;
        
    } catch (const A2AException& e) {
        std::cerr << "A2A Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
