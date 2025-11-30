#include <a2a/server/task_manager.hpp>
#include <a2a/server/memory_task_store.hpp>
#include <a2a/core/exception.hpp>
#include <iostream>
#include <string>
#include <exception>

using namespace a2a;

int main() {
    std::cout << "=== A2A C++ Echo Agent ===" << std::endl << std::endl;
    
    try {
        // Create task manager with in-memory store
        auto task_store = std::make_shared<MemoryTaskStore>();
        TaskManager task_manager(task_store);
        
        // Set up message handler
        task_manager.set_on_message_received([](const MessageSendParams& params) -> A2AResponse {
            std::cout << "Received message: " << params.message().get_text() << std::endl;
            
            // Echo back the message
            auto response = AgentMessage::create()
                .with_role(MessageRole::Agent)
                .with_text("Echo: " + params.message().get_text());
            
            return A2AResponse(response);
        });
        
        // Set up agent card handler
        task_manager.set_on_agent_card_query([](const std::string& agent_url) -> AgentCard {
            AgentCapabilities caps;
            caps.streaming = true;
            caps.task_management = true;
            
            return AgentCard::create()
                .with_name("Echo Agent")
                .with_description("A simple echo agent that repeats your messages")
                .with_url(agent_url)
                .with_version("1.0.0")
                .with_capabilities(caps)
                .with_input_mode("text")
                .with_output_mode("text");
        });
        
        // Set up task lifecycle handlers
        task_manager.set_on_task_created([](const AgentTask& task) {
            std::cout << "Task created: " << task.id() << std::endl;
        });
        
        task_manager.set_on_task_updated([](const AgentTask& task) {
            std::cout << "Task updated: " << task.id() 
                     << " - Status: " << to_string(task.status().state()) << std::endl;
        });
        
        task_manager.set_on_task_cancelled([](const AgentTask& task) {
            std::cout << "Task cancelled: " << task.id() << std::endl;
        });
        
        std::cout << "Echo Agent initialized successfully!" << std::endl;
        std::cout << "Ready to receive messages..." << std::endl << std::endl;
        
        // In a real implementation, you would start an HTTP server here
        // For this demo, we'll just test the message handler directly
        
        // Test message
        auto test_message = AgentMessage::create()
            .with_role(MessageRole::User)
            .with_text("Hello, Echo Agent!");
        
        auto test_params = MessageSendParams::create()
            .with_message(test_message);
        
        auto response = task_manager.send_message(test_params);
        
        if (response.is_message()) {
            std::cout << "Response: " << response.as_message().get_text() << std::endl;
        }
        
        std::cout << std::endl << "=== Agent demo completed ===" << std::endl;
        
        return 0;
        
    } catch (const A2AException& e) {
        std::cerr << "A2A Error: " << e.what() << std::endl;
        std::cerr << "Error Code: " << e.error_code_value() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
