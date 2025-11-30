#pragma once

#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;

/**
 * @brief 阿里百炼 API 客户端
 * 用于调用通义千问模型
 */
class QwenClient {
public:
    explicit QwenClient(const std::string& api_key, 
                       const std::string& model = "qwen-plus")
        : api_key_(api_key)
        , model_(model)
        , api_url_("https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation") {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }
    
    ~QwenClient() {
        curl_global_cleanup();
    }
    
    /**
     * @brief 调用通义千问 API
     * @param system_prompt 系统提示词
     * @param user_message 用户消息
     * @return AI 回复
     */
    std::string chat(const std::string& system_prompt, 
                    const std::string& user_message) {
        // 构造请求 JSON
        json request_body = {
            {"model", model_},
            {"input", {
                {"messages", json::array({
                    {{"role", "system"}, {"content", system_prompt}},
                    {{"role", "user"}, {"content", user_message}}
                })}
            }},
            {"parameters", {
                {"result_format", "message"}
            }}
        };
        
        std::string request_str = request_body.dump();
        
        // 发送 HTTP 请求
        std::string response = send_post_request(request_str);
        
        // 解析响应
        try {
            json response_json = json::parse(response);
            
            // 检查错误
            if (response_json.contains("code")) {
                std::string error_msg = "API Error: " + 
                    response_json.value("message", "Unknown error");
                throw std::runtime_error(error_msg);
            }
            
            // 提取回复内容
            if (response_json.contains("output") && 
                response_json["output"].contains("choices") &&
                !response_json["output"]["choices"].empty()) {
                
                auto& choice = response_json["output"]["choices"][0];
                if (choice.contains("message") && 
                    choice["message"].contains("content")) {
                    return choice["message"]["content"].get<std::string>();
                }
            }
            
            throw std::runtime_error("Invalid response format");
            
        } catch (const json::exception& e) {
            throw std::runtime_error(std::string("JSON parse error: ") + e.what());
        }
    }

private:
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
    
    std::string send_post_request(const std::string& data) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }
        
        std::string response_data;
        
        // 设置请求头
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string auth_header = "Authorization: Bearer " + api_key_;
        headers = curl_slist_append(headers, auth_header.c_str());
        
        // 配置 CURL
        curl_easy_setopt(curl, CURLOPT_URL, api_url_.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        
        // 执行请求
        CURLcode res = curl_easy_perform(curl);
        
        // 清理
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            throw std::runtime_error(std::string("CURL error: ") + 
                                   curl_easy_strerror(res));
        }
        
        return response_data;
    }
    
    std::string api_key_;
    std::string model_;
    std::string api_url_;
};
