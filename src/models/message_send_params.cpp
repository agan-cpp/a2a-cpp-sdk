#include <a2a/models/message_send_params.hpp>
#include <sstream>

namespace a2a {

// MessageSendParams implementation
std::string MessageSendParams::to_json() const {
    std::ostringstream oss;
    oss << "{";
    
    // Required field
    oss << "\"message\":" << message_.to_json();
    
    // Optional fields
    if (history_length_.has_value()) {
        oss << ",\"historyLength\":" << *history_length_;
    }
    
    if (context_id_.has_value()) {
        oss << ",\"contextId\":\"" << *context_id_ << "\"";
    }
    
    if (task_id_.has_value()) {
        oss << ",\"taskId\":\"" << *task_id_ << "\"";
    }
    
    oss << "}";
    return oss.str();
}

MessageSendParams MessageSendParams::from_json(const std::string& json) {
    MessageSendParams params;
    
    // Extract message
    size_t msg_pos = json.find("\"message\":");
    if (msg_pos != std::string::npos) {
        size_t start = msg_pos + 10;
        size_t brace_count = 0;
        size_t end = start;
        
        for (size_t i = start; i < json.length(); ++i) {
            if (json[i] == '{') brace_count++;
            else if (json[i] == '}') {
                brace_count--;
                if (brace_count == 0) {
                    end = i + 1;
                    break;
                }
            }
        }
        
        std::string msg_json = json.substr(start, end - start);
        params.message_ = AgentMessage::from_json(msg_json);
    }
    
    // Extract historyLength (optional)
    size_t hist_pos = json.find("\"historyLength\":");
    if (hist_pos != std::string::npos) {
        size_t start = hist_pos + 16;
        size_t end = json.find_first_of(",}", start);
        params.history_length_ = std::stoi(json.substr(start, end - start));
    }
    
    // Extract contextId (optional)
    size_t ctx_pos = json.find("\"contextId\":");
    if (ctx_pos != std::string::npos) {
        size_t start = json.find("\"", ctx_pos + 12) + 1;
        size_t end = json.find("\"", start);
        params.context_id_ = json.substr(start, end - start);
    }
    
    // Extract taskId (optional)
    size_t task_pos = json.find("\"taskId\":");
    if (task_pos != std::string::npos) {
        size_t start = json.find("\"", task_pos + 9) + 1;
        size_t end = json.find("\"", start);
        params.task_id_ = json.substr(start, end - start);
    }
    
    return params;
}

// TaskQueryParams implementation
std::string TaskQueryParams::to_json() const {
    std::ostringstream oss;
    oss << "{";
    
    oss << "\"id\":\"" << id << "\"";
    
    if (history_length.has_value()) {
        oss << ",\"historyLength\":" << *history_length;
    }
    
    if (!metadata.empty()) {
        oss << ",\"metadata\":{";
        bool first = true;
        for (const auto& [key, value] : metadata) {
            if (!first) oss << ",";
            oss << "\"" << key << "\":\"" << value << "\"";
            first = false;
        }
        oss << "}";
    }
    
    oss << "}";
    return oss.str();
}

TaskQueryParams TaskQueryParams::from_json(const std::string& json) {
    TaskQueryParams params;
    
    // Extract id
    size_t id_pos = json.find("\"id\":");
    if (id_pos != std::string::npos) {
        size_t start = json.find("\"", id_pos + 5) + 1;
        size_t end = json.find("\"", start);
        params.id = json.substr(start, end - start);
    }
    
    // Extract historyLength (optional)
    size_t hist_pos = json.find("\"historyLength\":");
    if (hist_pos != std::string::npos) {
        size_t start = hist_pos + 16;
        size_t end = json.find_first_of(",}", start);
        params.history_length = std::stoi(json.substr(start, end - start));
    }
    
    return params;
}

// TaskIdParams implementation
std::string TaskIdParams::to_json() const {
    std::ostringstream oss;
    oss << "{\"id\":\"" << id << "\"}";
    return oss.str();
}

TaskIdParams TaskIdParams::from_json(const std::string& json) {
    TaskIdParams params;
    
    // Extract id
    size_t id_pos = json.find("\"id\":");
    if (id_pos != std::string::npos) {
        size_t start = json.find("\"", id_pos + 5) + 1;
        size_t end = json.find("\"", start);
        params.id = json.substr(start, end - start);
    }
    
    return params;
}

} // namespace a2a
