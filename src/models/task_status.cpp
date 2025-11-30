#include <a2a/models/task_status.hpp>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace a2a {

// Helper to convert timestamp to ISO 8601 string
static std::string timestamp_to_iso8601(const Timestamp& ts) {
    auto time_t_val = std::chrono::system_clock::to_time_t(ts);
    std::tm tm_val;
    
#ifdef _WIN32
    gmtime_s(&tm_val, &time_t_val);
#else
    gmtime_r(&time_t_val, &tm_val);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm_val, "%Y-%m-%dT%H:%M:%S");
    
    // Add milliseconds
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        ts.time_since_epoch()
    ) % 1000;
    
    oss << "." << std::setfill('0') << std::setw(3) << ms.count() << "Z";
    
    return oss.str();
}

std::string AgentTaskStatus::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"state\":\"" << to_string(state_) << "\","
        << "\"timestamp\":\"" << timestamp_to_iso8601(timestamp_) << "\"";
    
    if (!message_.empty()) {
        oss << ",\"message\":\"" << message_ << "\"";
    }
    
    oss << "}";
    return oss.str();
}

AgentTaskStatus AgentTaskStatus::from_json(const std::string& json) {
    AgentTaskStatus status;
    
    // Extract state
    size_t state_pos = json.find("\"state\":");
    if (state_pos != std::string::npos) {
        size_t start = json.find("\"", state_pos + 8) + 1;
        size_t end = json.find("\"", start);
        std::string state_str = json.substr(start, end - start);
        status.state_ = task_state_from_string(state_str);
    }
    
    // Extract timestamp (simplified - just use current time)
    status.timestamp_ = std::chrono::system_clock::now();
    
    // Extract message (optional)
    size_t msg_pos = json.find("\"message\":");
    if (msg_pos != std::string::npos) {
        size_t start = json.find("\"", msg_pos + 10) + 1;
        size_t end = json.find("\"", start);
        status.message_ = json.substr(start, end - start);
    }
    
    return status;
}

} // namespace a2a
