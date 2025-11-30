#pragma once

#include "../core/types.hpp"
#include "message_part.hpp"
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <ctime>

namespace a2a {

/**
 * @brief Agent Message - represents a message in the A2A protocol
 */
class AgentMessage {
public:
    AgentMessage() = default;
    
    // Copy constructor
    AgentMessage(const AgentMessage& other)
        : message_id_(other.message_id_)
        , context_id_(other.context_id_)
        , task_id_(other.task_id_)
        , role_(other.role_)
    {
        // Deep copy parts
        for (const auto& part : other.parts_) {
            parts_.push_back(part->clone());
        }
    }
    
    // Copy assignment
    AgentMessage& operator=(const AgentMessage& other) {
        if (this != &other) {
            message_id_ = other.message_id_;
            context_id_ = other.context_id_;
            task_id_ = other.task_id_;
            role_ = other.role_;
            
            // Deep copy parts
            parts_.clear();
            for (const auto& part : other.parts_) {
                parts_.push_back(part->clone());
            }
        }
        return *this;
    }
    
    // Move constructor and assignment (default)
    AgentMessage(AgentMessage&&) = default;
    AgentMessage& operator=(AgentMessage&&) = default;
    
    // Getters
    const std::string& message_id() const { return message_id_; }
    const std::optional<std::string>& context_id() const { return context_id_; }
    const std::optional<std::string>& task_id() const { return task_id_; }
    MessageRole role() const { return role_; }
    const std::vector<std::unique_ptr<Part>>& parts() const { return parts_; }
    
    // Setters
    void set_message_id(const std::string& id) { message_id_ = id; }
    void set_context_id(const std::string& id) { context_id_ = id; }
    void set_task_id(const std::string& id) { task_id_ = id; }
    void set_role(MessageRole role) { role_ = role; }
    
    /**
     * @brief Add a text part to the message
     */
    void add_text_part(const std::string& text) {
        parts_.push_back(std::make_unique<TextPart>(text));
    }
    
    /**
     * @brief Add a file part to the message
     */
    void add_file_part(const std::string& filename, 
                      const std::string& mime_type,
                      const std::vector<uint8_t>& data) {
        parts_.push_back(std::make_unique<FilePart>(filename, mime_type, data));
    }
    
    /**
     * @brief Add a data part to the message
     */
    void add_data_part(const std::string& data_json) {
        parts_.push_back(std::make_unique<DataPart>(data_json));
    }
    
    /**
     * @brief Add a part (takes ownership)
     */
    void add_part(std::unique_ptr<Part> part) {
        parts_.push_back(std::move(part));
    }
    
    /**
     * @brief Get the first text part content (convenience method)
     */
    std::string get_text() const {
        for (const auto& part : parts_) {
            if (part->kind() == PartKind::Text) {
                return static_cast<const TextPart*>(part.get())->text();
            }
        }
        return "";
    }
    
    /**
     * @brief Serialize to JSON
     */
    std::string to_json() const;
    
    /**
     * @brief Deserialize from JSON
     */
    static AgentMessage from_json(const std::string& json);
    
    /**
     * @brief Create a new AgentMessage with default values
     */
    static AgentMessage create() {
        AgentMessage msg;
        msg.message_id_ = "msg-" + std::to_string(std::time(nullptr));
        return msg;
    }
    
    /**
     * @brief Fluent API methods for building messages
     */
    AgentMessage& with_message_id(const std::string& id) {
        message_id_ = id;
        return *this;
    }
    
    AgentMessage& with_context_id(const std::string& id) {
        context_id_ = id;
        return *this;
    }
    
    AgentMessage& with_task_id(const std::string& id) {
        task_id_ = id;
        return *this;
    }
    
    AgentMessage& with_role(MessageRole role) {
        role_ = role;
        return *this;
    }
    
    AgentMessage& with_text(const std::string& text) {
        add_text_part(text);
        return *this;
    }

private:
    std::string message_id_;
    std::optional<std::string> context_id_;
    std::optional<std::string> task_id_;
    MessageRole role_ = MessageRole::User;
    std::vector<std::unique_ptr<Part>> parts_;
};

} // namespace a2a
