#pragma once

#include <string>
#include <optional>
#include <map>

namespace a2a {

/**
 * @brief Artifact - represents output produced by an agent
 */
class Artifact {
public:
    Artifact() = default;
    
    Artifact(const std::string& id, const std::string& name)
        : id_(id), name_(name) {}
    
    // Getters
    const std::string& id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::optional<std::string>& description() const { return description_; }
    const std::optional<std::string>& mime_type() const { return mime_type_; }
    const std::optional<std::string>& url() const { return url_; }
    const std::optional<std::string>& content() const { return content_; }
    const std::map<std::string, std::string>& metadata() const { return metadata_; }
    
    // Setters
    void set_id(const std::string& id) { id_ = id; }
    void set_name(const std::string& name) { name_ = name; }
    void set_description(const std::string& desc) { description_ = desc; }
    void set_mime_type(const std::string& type) { mime_type_ = type; }
    void set_url(const std::string& url) { url_ = url; }
    void set_content(const std::string& content) { content_ = content; }
    void add_metadata(const std::string& key, const std::string& value) {
        metadata_[key] = value;
    }
    
    /**
     * @brief Serialize to JSON
     */
    std::string to_json() const;
    
    /**
     * @brief Deserialize from JSON
     */
    static Artifact from_json(const std::string& json);
    
    /**
     * @brief Create a new Artifact
     */
    static Artifact create() {
        return Artifact();
    }
    
    /**
     * @brief Fluent API methods
     */
    Artifact& with_id(const std::string& id) {
        id_ = id;
        return *this;
    }
    
    Artifact& with_name(const std::string& name) {
        name_ = name;
        return *this;
    }
    
    Artifact& with_description(const std::string& desc) {
        description_ = desc;
        return *this;
    }
    
    Artifact& with_mime_type(const std::string& type) {
        mime_type_ = type;
        return *this;
    }
    
    Artifact& with_url(const std::string& url) {
        url_ = url;
        return *this;
    }
    
    Artifact& with_content(const std::string& content) {
        content_ = content;
        return *this;
    }
    
    Artifact& with_metadata(const std::string& key, const std::string& value) {
        metadata_[key] = value;
        return *this;
    }

private:
    std::string id_;
    std::string name_;
    std::optional<std::string> description_;
    std::optional<std::string> mime_type_;
    std::optional<std::string> url_;
    std::optional<std::string> content_;
    std::map<std::string, std::string> metadata_;
};

} // namespace a2a
