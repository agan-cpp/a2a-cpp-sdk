#include <a2a/models/agent_message.hpp>
#include <sstream>

namespace a2a {

std::string AgentMessage::to_json() const {
    std::ostringstream oss;
    oss << "{";
    
    // Required fields
    oss << "\"messageId\":\"" << message_id_ << "\",";
    oss << "\"role\":\"" << to_string(role_) << "\"";
    
    // Optional fields
    if (context_id_.has_value()) {
        oss << ",\"contextId\":\"" << *context_id_ << "\"";
    }
    
    if (task_id_.has_value()) {
        oss << ",\"taskId\":\"" << *task_id_ << "\"";
    }
    
    // Parts array
    oss << ",\"parts\":[";
    for (size_t i = 0; i < parts_.size(); ++i) {
        if (i > 0) oss << ",";
        oss << parts_[i]->to_json();
    }
    oss << "]";
    
    oss << "}";
    return oss.str();
}

AgentMessage AgentMessage::from_json(const std::string& json) {
    AgentMessage msg;
    
    // Extract messageId
    size_t msg_id_pos = json.find("\"messageId\":");
    if (msg_id_pos != std::string::npos) {
        size_t start = json.find("\"", msg_id_pos + 12) + 1;
        size_t end = json.find("\"", start);
        msg.message_id_ = json.substr(start, end - start);
    }
    
    // Extract role
    size_t role_pos = json.find("\"role\":");
    if (role_pos != std::string::npos) {
        size_t start = json.find("\"", role_pos + 7) + 1;
        size_t end = json.find("\"", start);
        std::string role_str = json.substr(start, end - start);
        msg.role_ = message_role_from_string(role_str);
    }
    
    // Extract contextId (optional)
    size_t ctx_id_pos = json.find("\"contextId\":");
    if (ctx_id_pos != std::string::npos) {
        size_t start = json.find("\"", ctx_id_pos + 12) + 1;
        size_t end = json.find("\"", start);
        msg.context_id_ = json.substr(start, end - start);
    }
    
    // Extract taskId (optional)
    size_t task_id_pos = json.find("\"taskId\":");
    if (task_id_pos != std::string::npos) {
        size_t start = json.find("\"", task_id_pos + 9) + 1;
        size_t end = json.find("\"", start);
        msg.task_id_ = json.substr(start, end - start);
    }
    
    // Extract parts array (simplified)
    size_t parts_pos = json.find("\"parts\":[");
    if (parts_pos != std::string::npos) {
        size_t array_start = parts_pos + 9;
        size_t array_end = json.find("]", array_start);
        std::string parts_json = json.substr(array_start, array_end - array_start);
        
        // Parse each part (very simplified)
        int brace_count = 0;
        size_t part_start = 0;
        
        for (size_t i = 0; i < parts_json.length(); ++i) {
            if (parts_json[i] == '{') {
                if (brace_count == 0) part_start = i;
                brace_count++;
            } else if (parts_json[i] == '}') {
                brace_count--;
                if (brace_count == 0) {
                    std::string part_json = parts_json.substr(part_start, i - part_start + 1);
                    auto part = Part::from_json(part_json);
                    if (part) {
                        msg.parts_.push_back(std::move(part));
                    }
                }
            }
        }
    }
    
    return msg;
}

} // namespace a2a
