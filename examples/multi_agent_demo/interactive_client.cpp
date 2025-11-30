#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <ctime>
#include <sstream>

using json = nlohmann::json;

// CURL 回调函数
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// 发送消息到 Agent
std::string send_message(const std::string& text, const std::string& context_id) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "错误: 无法初始化 CURL";
    }

    // 构造 JSON-RPC 请求
    json request = {
        {"jsonrpc", "2.0"},
        {"id", std::to_string(std::time(nullptr))},
        {"method", "message/send"},
        {"params", {
            {"message", {
                {"role", "user"},
                {"contextId", context_id},
                {"parts", {{{"kind", "text"}, {"text", text}}}}
            }}
        }}
    };

    std::string request_body = request.dump();
    std::string response_body;

    // 设置 CURL 选项
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:5000/");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    // 执行请求
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "错误: " + std::string(curl_easy_strerror(res));
    }

    // 解析响应
    try {
        auto response = json::parse(response_body);
        
        if (response.contains("error")) {
            return "错误: " + response["error"]["message"].get<std::string>();
        }
        
        if (response.contains("result") && 
            response["result"].contains("parts") && 
            !response["result"]["parts"].empty()) {
            return response["result"]["parts"][0]["text"].get<std::string>();
        }
        
        return "错误: 无法解析响应";
        
    } catch (const std::exception& e) {
        return "错误: " + std::string(e.what());
    }
}

// 生成会话 ID
std::string generate_session_id() {
    std::stringstream ss;
    ss << "session-" << std::time(nullptr);
    return ss.str();
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   A2A 交互式测试客户端" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // 生成会话 ID
    std::string session_id = generate_session_id();
    std::cout << "会话 ID: " << session_id << std::endl;
    std::cout << std::endl;
    
    std::cout << "提示：" << std::endl;
    std::cout << "  - 输入您的问题，按回车发送" << std::endl;
    std::cout << "  - 输入 'quit' 或 'exit' 退出" << std::endl;
    std::cout << "  - 输入 'new' 开始新的会话" << std::endl;
    std::cout << "  - 输入 'clear' 清屏" << std::endl;
    std::cout << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << std::endl;

    std::string input;
    int turn = 0;

    while (true) {
        // 显示提示符
        std::cout << "\033[1;32m您\033[0m> ";
        std::getline(std::cin, input);

        // 去除首尾空格
        input.erase(0, input.find_first_not_of(" \t\n\r"));
        input.erase(input.find_last_not_of(" \t\n\r") + 1);

        // 检查是否为空
        if (input.empty()) {
            continue;
        }

        // 处理特殊命令
        if (input == "quit" || input == "exit") {
            std::cout << std::endl;
            std::cout << "感谢使用！再见！" << std::endl;
            break;
        }

        if (input == "new") {
            session_id = generate_session_id();
            turn = 0;
            std::cout << std::endl;
            std::cout << "✓ 已开始新会话: " << session_id << std::endl;
            std::cout << std::endl;
            continue;
        }

        if (input == "clear") {
            std::cout << "\033[2J\033[1;1H";
            std::cout << "========================================" << std::endl;
            std::cout << "   A2A 交互式测试客户端" << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << std::endl;
            std::cout << "会话 ID: " << session_id << std::endl;
            std::cout << std::endl;
            continue;
        }

        // 发送消息
        turn++;
        std::cout << std::endl;
        std::cout << "正在思考..." << std::flush;

        std::string response = send_message(input, session_id);

        // 清除 "正在思考..." 提示
        std::cout << "\r                    \r";

        // 显示响应
        std::cout << "\033[1;34mAgent\033[0m> " << response << std::endl;
        std::cout << std::endl;
    }

    return 0;
}
