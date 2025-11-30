#include <a2a/models/agent_task.hpp>
#include <sstream>

namespace a2a {

std::string AgentTask::to_json() const {
    std::ostringstream oss;
    oss << "{";
    
    // Required fields
    oss << "\"id\":\"" << id_ << "\",";
    oss << "\"contextId\":\"" << context_id_ << "\",";
    oss << "\"status\":" << status_.to_json();
    
    // Artifacts array
    if (!artifacts_.empty()) {
        oss << ",\"artifacts\":[";
        for (size_t i = 0; i < artifacts_.size(); ++i) {
            if (i > 0) oss << ",";
            oss << artifacts_[i].to_json();
        }
        oss << "]";
    }
    
    // History array
    if (!history_.empty()) {
        oss << ",\"history\":[";
        for (size_t i = 0; i < history_.size(); ++i) {
            if (i > 0) oss << ",";
            oss << history_[i].to_json();
        }
        oss << "]";
    }
    
    // Metadata
    if (!metadata_.empty()) {
        oss << ",\"metadata\":{";
        bool first = true;
        for (const auto& [key, value] : metadata_) {
            if (!first) oss << ",";
            oss << "\"" << key << "\":\"" << value << "\"";
            first = false;
        }
        oss << "}";
    }
    
    oss << "}";
    return oss.str();
}

AgentTask AgentTask::from_json(const std::string& json) {
    AgentTask task;
    
    // Extract id
    size_t id_pos = json.find("\"id\":");
    if (id_pos != std::string::npos) {
        size_t start = json.find("\"", id_pos + 5) + 1;
        size_t end = json.find("\"", start);
        task.id_ = json.substr(start, end - start);
    }
    
    // Extract contextId
    size_t ctx_pos = json.find("\"contextId\":");
    if (ctx_pos != std::string::npos) {
        size_t start = json.find("\"", ctx_pos + 12) + 1;
        size_t end = json.find("\"", start);
        task.context_id_ = json.substr(start, end - start);
    }
    
    // Extract status
    size_t status_pos = json.find("\"status\":");
    if (status_pos != std::string::npos) {
        size_t start = status_pos + 9;
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
        
        std::string status_json = json.substr(start, end - start);
        task.status_ = AgentTaskStatus::from_json(status_json);
    }
    
    // Extract artifacts (simplified)
    size_t artifacts_pos = json.find("\"artifacts\":[");
    if (artifacts_pos != std::string::npos) {
        // Simplified array parsing
        // In production, use proper JSON library
    }
    
    // Extract history (simplified)
    size_t history_pos = json.find("\"history\":[");
    if (history_pos != std::string::npos) {
        // Simplified array parsing
        // In production, use proper JSON library
    }
    
    return task;
}

} // namespace a2a
