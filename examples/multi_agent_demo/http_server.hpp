#pragma once

#include <string>
#include <functional>
#include <map>
#include <thread>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>

/**
 * @brief 简单的 HTTP 服务器
 * 用于接收 A2A 协议的 HTTP 请求
 */
class HttpServer {
public:
    using RequestHandler = std::function<std::string(const std::string&)>;
    
    explicit HttpServer(int port) : port_(port), running_(false) {}
    
    ~HttpServer() {
        stop();
    }
    
    void register_handler(const std::string& path, RequestHandler handler) {
        handlers_[path] = handler;
    }
    
    void start() {
        running_ = true;
        
        // 创建 socket
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            throw std::runtime_error("Failed to create socket");
        }
        
        // 设置 socket 选项
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        // 绑定地址
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);
        
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            close(server_fd);
            throw std::runtime_error("Failed to bind to port " + std::to_string(port_));
        }
        
        // 监听
        if (listen(server_fd, 10) < 0) {
            close(server_fd);
            throw std::runtime_error("Failed to listen on port " + std::to_string(port_));
        }
        
        std::cout << "HTTP Server listening on port " << port_ << std::endl;
        
        // 接受连接
        while (running_) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) {
                continue;
            }
            
            // 处理请求（在新线程中）
            std::thread([this, client_fd]() {
                this->handle_client(client_fd);
            }).detach();
        }
        
        close(server_fd);
    }
    
    void stop() {
        running_ = false;
    }

private:
    void handle_client(int client_fd) {
        char buffer[8192] = {0};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        
        if (bytes_read <= 0) {
            close(client_fd);
            return;
        }
        
        std::string request(buffer, bytes_read);
        
        // 解析 HTTP 请求
        std::istringstream request_stream(request);
        std::string method, path, version;
        request_stream >> method >> path >> version;
        
        // 提取请求体
        std::string body;
        size_t body_pos = request.find("\r\n\r\n");
        if (body_pos != std::string::npos) {
            body = request.substr(body_pos + 4);
        }
        
        // 查找处理器
        std::string response_body;
        int status_code = 200;
        
        auto it = handlers_.find(path);
        if (it != handlers_.end()) {
            try {
                response_body = it->second(body);
            } catch (const std::exception& e) {
                status_code = 500;
                response_body = std::string("{\"error\":\"") + e.what() + "\"}";
            }
        } else {
            status_code = 404;
            response_body = "{\"error\":\"Not Found\"}";
        }
        
        // 构造 HTTP 响应
        std::ostringstream response;
        response << "HTTP/1.1 " << status_code << " OK\r\n";
        response << "Content-Type: application/json\r\n";
        response << "Content-Length: " << response_body.length() << "\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "\r\n";
        response << response_body;
        
        std::string response_str = response.str();
        write(client_fd, response_str.c_str(), response_str.length());
        
        close(client_fd);
    }
    
    int port_;
    bool running_;
    std::map<std::string, RequestHandler> handlers_;
};
