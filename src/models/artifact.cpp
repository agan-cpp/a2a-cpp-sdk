#include <a2a/models/artifact.hpp>
#include <sstream>

namespace a2a {

std::string Artifact::to_json() const {
    std::ostringstream oss;
    oss << "{";
    
    // Required fields
    oss << "\"id\":\"" << id_ << "\",";
    oss << "\"name\":\"" << name_ << "\"";
    
    // Optional fields
    if (description_.has_value()) {
        oss << ",\"description\":\"" << *description_ << "\"";
    }
    
    if (mime_type_.has_value()) {
        oss << ",\"mimeType\":\"" << *mime_type_ << "\"";
    }
    
    if (url_.has_value()) {
        oss << ",\"url\":\"" << *url_ << "\"";
    }
    
    if (content_.has_value()) {
        oss << ",\"content\":\"" << *content_ << "\"";
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

Artifact Artifact::from_json(const std::string& json) {
    Artifact artifact;
    
    // Extract id
    size_t id_pos = json.find("\"id\":");
    if (id_pos != std::string::npos) {
        size_t start = json.find("\"", id_pos + 5) + 1;
        size_t end = json.find("\"", start);
        artifact.id_ = json.substr(start, end - start);
    }
    
    // Extract name
    size_t name_pos = json.find("\"name\":");
    if (name_pos != std::string::npos) {
        size_t start = json.find("\"", name_pos + 7) + 1;
        size_t end = json.find("\"", start);
        artifact.name_ = json.substr(start, end - start);
    }
    
    // Extract description (optional)
    size_t desc_pos = json.find("\"description\":");
    if (desc_pos != std::string::npos) {
        size_t start = json.find("\"", desc_pos + 14) + 1;
        size_t end = json.find("\"", start);
        artifact.description_ = json.substr(start, end - start);
    }
    
    // Extract mimeType (optional)
    size_t mime_pos = json.find("\"mimeType\":");
    if (mime_pos != std::string::npos) {
        size_t start = json.find("\"", mime_pos + 11) + 1;
        size_t end = json.find("\"", start);
        artifact.mime_type_ = json.substr(start, end - start);
    }
    
    // Extract url (optional)
    size_t url_pos = json.find("\"url\":");
    if (url_pos != std::string::npos) {
        size_t start = json.find("\"", url_pos + 6) + 1;
        size_t end = json.find("\"", start);
        artifact.url_ = json.substr(start, end - start);
    }
    
    // Extract content (optional)
    size_t content_pos = json.find("\"content\":");
    if (content_pos != std::string::npos) {
        size_t start = json.find("\"", content_pos + 10) + 1;
        size_t end = json.find("\"", start);
        artifact.content_ = json.substr(start, end - start);
    }
    
    return artifact;
}

} // namespace a2a
