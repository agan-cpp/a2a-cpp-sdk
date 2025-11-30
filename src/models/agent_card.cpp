#include <a2a/models/agent_card.hpp>
#include <sstream>

namespace a2a {

// AgentCapabilities implementation
std::string AgentCapabilities::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"streaming\":" << (streaming ? "true" : "false") << ","
        << "\"pushNotifications\":" << (push_notifications ? "true" : "false") << ","
        << "\"taskManagement\":" << (task_management ? "true" : "false")
        << "}";
    return oss.str();
}

AgentCapabilities AgentCapabilities::from_json(const std::string& json) {
    AgentCapabilities caps;
    caps.streaming = json.find("\"streaming\":true") != std::string::npos;
    caps.push_notifications = json.find("\"pushNotifications\":true") != std::string::npos;
    caps.task_management = json.find("\"taskManagement\":true") != std::string::npos;
    return caps;
}

// AgentSkill implementation
std::string AgentSkill::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"name\":\"" << name << "\","
        << "\"description\":\"" << description << "\"";
    
    if (!input_modes.empty()) {
        oss << ",\"inputModes\":[";
        for (size_t i = 0; i < input_modes.size(); ++i) {
            if (i > 0) oss << ",";
            oss << "\"" << input_modes[i] << "\"";
        }
        oss << "]";
    }
    
    if (!output_modes.empty()) {
        oss << ",\"outputModes\":[";
        for (size_t i = 0; i < output_modes.size(); ++i) {
            if (i > 0) oss << ",";
            oss << "\"" << output_modes[i] << "\"";
        }
        oss << "]";
    }
    
    oss << "}";
    return oss.str();
}

AgentSkill AgentSkill::from_json(const std::string& json) {
    AgentSkill skill;
    
    // Extract name
    size_t name_pos = json.find("\"name\":");
    if (name_pos != std::string::npos) {
        size_t start = json.find("\"", name_pos + 7) + 1;
        size_t end = json.find("\"", start);
        skill.name = json.substr(start, end - start);
    }
    
    // Extract description
    size_t desc_pos = json.find("\"description\":");
    if (desc_pos != std::string::npos) {
        size_t start = json.find("\"", desc_pos + 14) + 1;
        size_t end = json.find("\"", start);
        skill.description = json.substr(start, end - start);
    }
    
    return skill;
}

// AgentProvider implementation
std::string AgentProvider::to_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"name\":\"" << name << "\","
        << "\"organization\":\"" << organization << "\"";
    
    if (url.has_value()) {
        oss << ",\"url\":\"" << *url << "\"";
    }
    
    oss << "}";
    return oss.str();
}

AgentProvider AgentProvider::from_json(const std::string& json) {
    AgentProvider provider;
    
    // Extract name
    size_t name_pos = json.find("\"name\":");
    if (name_pos != std::string::npos) {
        size_t start = json.find("\"", name_pos + 7) + 1;
        size_t end = json.find("\"", start);
        provider.name = json.substr(start, end - start);
    }
    
    // Extract organization
    size_t org_pos = json.find("\"organization\":");
    if (org_pos != std::string::npos) {
        size_t start = json.find("\"", org_pos + 15) + 1;
        size_t end = json.find("\"", start);
        provider.organization = json.substr(start, end - start);
    }
    
    return provider;
}

// AgentCard implementation
std::string AgentCard::to_json() const {
    std::ostringstream oss;
    oss << "{";
    
    // Required fields
    oss << "\"name\":\"" << name_ << "\",";
    oss << "\"description\":\"" << description_ << "\",";
    oss << "\"url\":\"" << url_ << "\",";
    oss << "\"version\":\"" << version_ << "\",";
    oss << "\"protocolVersion\":\"" << protocol_version_ << "\",";
    oss << "\"capabilities\":" << capabilities_.to_json() << ",";
    
    // Default input modes
    oss << "\"defaultInputModes\":[";
    for (size_t i = 0; i < default_input_modes_.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "\"" << default_input_modes_[i] << "\"";
    }
    oss << "],";
    
    // Default output modes
    oss << "\"defaultOutputModes\":[";
    for (size_t i = 0; i < default_output_modes_.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "\"" << default_output_modes_[i] << "\"";
    }
    oss << "],";
    
    // Skills
    oss << "\"skills\":[";
    for (size_t i = 0; i < skills_.size(); ++i) {
        if (i > 0) oss << ",";
        oss << skills_[i].to_json();
    }
    oss << "],";
    
    // Preferred transport
    oss << "\"preferredTransport\":\"" 
        << (preferred_transport_ == AgentTransport::JsonRpc ? "jsonrpc" : "http") 
        << "\"";
    
    // Optional fields
    if (icon_url_.has_value()) {
        oss << ",\"iconUrl\":\"" << *icon_url_ << "\"";
    }
    
    if (documentation_url_.has_value()) {
        oss << ",\"documentationUrl\":\"" << *documentation_url_ << "\"";
    }
    
    if (provider_.has_value()) {
        oss << ",\"provider\":" << provider_->to_json();
    }
    
    oss << "}";
    return oss.str();
}

AgentCard AgentCard::from_json(const std::string& json) {
    AgentCard card;
    
    // Extract name
    size_t name_pos = json.find("\"name\":");
    if (name_pos != std::string::npos) {
        size_t start = json.find("\"", name_pos + 7) + 1;
        size_t end = json.find("\"", start);
        card.name_ = json.substr(start, end - start);
    }
    
    // Extract description
    size_t desc_pos = json.find("\"description\":");
    if (desc_pos != std::string::npos) {
        size_t start = json.find("\"", desc_pos + 14) + 1;
        size_t end = json.find("\"", start);
        card.description_ = json.substr(start, end - start);
    }
    
    // Extract url
    size_t url_pos = json.find("\"url\":");
    if (url_pos != std::string::npos) {
        size_t start = json.find("\"", url_pos + 6) + 1;
        size_t end = json.find("\"", start);
        card.url_ = json.substr(start, end - start);
    }
    
    // Extract version
    size_t ver_pos = json.find("\"version\":");
    if (ver_pos != std::string::npos) {
        size_t start = json.find("\"", ver_pos + 10) + 1;
        size_t end = json.find("\"", start);
        card.version_ = json.substr(start, end - start);
    }
    
    // Extract protocolVersion
    size_t proto_pos = json.find("\"protocolVersion\":");
    if (proto_pos != std::string::npos) {
        size_t start = json.find("\"", proto_pos + 18) + 1;
        size_t end = json.find("\"", start);
        card.protocol_version_ = json.substr(start, end - start);
    }
    
    // Extract capabilities
    size_t caps_pos = json.find("\"capabilities\":");
    if (caps_pos != std::string::npos) {
        size_t start = caps_pos + 15;
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
        
        std::string caps_json = json.substr(start, end - start);
        card.capabilities_ = AgentCapabilities::from_json(caps_json);
    }
    
    return card;
}

} // namespace a2a
