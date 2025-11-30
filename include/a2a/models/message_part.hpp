#pragma once

#include "../core/types.hpp"
#include <string>
#include <memory>
#include <vector>

namespace a2a {

/**
 * @brief Base class for message parts (polymorphic)
 */
class Part {
public:
    virtual ~Part() = default;
    
    virtual PartKind kind() const = 0;
    virtual std::string to_json() const = 0;
    virtual std::unique_ptr<Part> clone() const = 0;
    
    static std::unique_ptr<Part> from_json(const std::string& json);
};

/**
 * @brief Text message part
 */
class TextPart : public Part {
public:
    TextPart() = default;
    explicit TextPart(const std::string& text) : text_(text) {}
    
    PartKind kind() const override { return PartKind::Text; }
    
    const std::string& text() const { return text_; }
    void set_text(const std::string& text) { text_ = text; }
    
    std::string to_json() const override;
    std::unique_ptr<Part> clone() const override {
        return std::make_unique<TextPart>(text_);
    }

private:
    std::string text_;
};

/**
 * @brief File message part
 */
class FilePart : public Part {
public:
    FilePart() = default;
    FilePart(const std::string& filename, const std::string& mime_type, 
             const std::vector<uint8_t>& data)
        : filename_(filename)
        , mime_type_(mime_type)
        , data_(data) {}
    
    PartKind kind() const override { return PartKind::File; }
    
    const std::string& filename() const { return filename_; }
    const std::string& mime_type() const { return mime_type_; }
    const std::vector<uint8_t>& data() const { return data_; }
    
    void set_filename(const std::string& name) { filename_ = name; }
    void set_mime_type(const std::string& type) { mime_type_ = type; }
    void set_data(const std::vector<uint8_t>& data) { data_ = data; }
    
    std::string to_json() const override;
    std::unique_ptr<Part> clone() const override {
        return std::make_unique<FilePart>(filename_, mime_type_, data_);
    }

private:
    std::string filename_;
    std::string mime_type_;
    std::vector<uint8_t> data_;
};

/**
 * @brief Data message part (structured data)
 */
class DataPart : public Part {
public:
    DataPart() = default;
    explicit DataPart(const std::string& data_json) : data_json_(data_json) {}
    
    PartKind kind() const override { return PartKind::Data; }
    
    const std::string& data_json() const { return data_json_; }
    void set_data_json(const std::string& json) { data_json_ = json; }
    
    std::string to_json() const override;
    std::unique_ptr<Part> clone() const override {
        return std::make_unique<DataPart>(data_json_);
    }

private:
    std::string data_json_;
};

} // namespace a2a
