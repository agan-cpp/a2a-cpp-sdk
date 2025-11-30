#include <a2a/client/a2a_client.hpp>
#include <a2a/client/card_resolver.hpp>
#include <a2a/core/exception.hpp>
#include <iostream>
#include <string>
#include <exception>

using namespace a2a;

int main() {
    try {
        std::cout << "=== A2A C++ Client Demo ===" << std::endl << std::endl;
        
        // 1. Discover agent capabilities
        std::cout << "1. Discovering agent..." << std::endl;
        A2ACardResolver resolver("http://localhost:5000");
        auto agent_card = resolver.get_agent_card();
        
        std::cout << "   Agent Name: " << agent_card.name() << std::endl;
        std::cout << "   Description: " << agent_card.description() << std::endl;
        std::cout << "   Version: " << agent_card.version() << std::endl;
        std::cout << "   Protocol: " << agent_card.protocol_version() << std::endl;
        std::cout << std::endl;
        
        // 2. Create client
        std::cout << "2. Creating client..." << std::endl;
        A2AClient client(agent_card.url());
        std::cout << "   Client created successfully" << std::endl << std::endl;
        
        // 3. Send a simple message
        std::cout << "3. Sending message..." << std::endl;
        auto message = AgentMessage::create()
            .with_role(MessageRole::User)
            .with_text("Hello, Agent! How are you?");
        
        auto params = MessageSendParams::create()
            .with_message(message);
        
        auto response = client.send_message(params);
        
        if (response.is_message()) {
            std::cout << "   Response (Message): " 
                     << response.as_message().get_text() << std::endl;
        } else if (response.is_task()) {
            std::cout << "   Response (Task): " 
                     << response.as_task().id() << std::endl;
            std::cout << "   Status: " 
                     << to_string(response.as_task().status().state()) << std::endl;
        }
        std::cout << std::endl;
        
        // 4. Send streaming message
        std::cout << "4. Sending streaming message..." << std::endl;
        auto stream_message = AgentMessage::create()
            .with_role(MessageRole::User)
            .with_text("Generate a story about AI");
        
        auto stream_params = MessageSendParams::create()
            .with_message(stream_message);
        
        client.send_message_streaming(stream_params, [](const std::string& event) {
            std::cout << "   Event: " << event << std::endl;
        });
        
        std::cout << std::endl;
        std::cout << "=== Demo completed ===" << std::endl;
        
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
