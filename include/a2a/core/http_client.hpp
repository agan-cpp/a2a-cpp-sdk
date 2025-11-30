#pragma once

#include <string>
#include <map>
#include <memory>
#include <functional>

namespace a2a {

/**
 * @brief HTTP Response
 */
struct HttpResponse {
    int status_code;
    std::string body;
    std::map<std::string, std::string> headers;
    
    bool is_success() const {
        return status_code >= 200 && status_code < 300;
    }
};

/**
 * @brief HTTP Client wrapper (uses libcurl internally)
 */
class HttpClient {
public:
    HttpClient();
    ~HttpClient();
    
    // Disable copy, enable move
    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;
    HttpClient(HttpClient&&) noexcept;
    HttpClient& operator=(HttpClient&&) noexcept;
    
    /**
     * @brief Perform GET request
     */
    HttpResponse get(const std::string& url);
    
    /**
     * @brief Perform POST request
     */
    HttpResponse post(const std::string& url, 
                     const std::string& body,
                     const std::string& content_type = "application/json");
    
    /**
     * @brief Perform POST request with streaming response
     * @param callback Called for each chunk of data received
     */
    void post_stream(const std::string& url,
                    const std::string& body,
                    const std::string& content_type,
                    std::function<void(const std::string&)> callback);
    
    /**
     * @brief Set request timeout in seconds
     */
    void set_timeout(long seconds);
    
    /**
     * @brief Add custom header
     */
    void add_header(const std::string& key, const std::string& value);
    
    /**
     * @brief Clear all custom headers
     */
    void clear_headers();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace a2a
