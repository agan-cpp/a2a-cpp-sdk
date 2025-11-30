cmake_minimum_required(VERSION 3.10)
project(multi_agent_demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(CURL REQUIRED)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# 链接 A2A 库
link_directories(${CMAKE_BINARY_DIR})

# Redis TaskStore 实现（分布式部署）
add_library(redis_task_store STATIC redis_task_store.cpp)
target_link_libraries(redis_task_store 
    a2a
    hiredis
)

# Redis Orchestrator（独立进程）
add_executable(redis_orchestrator redis_orchestrator.cpp)
target_link_libraries(redis_orchestrator 
    a2a
    redis_task_store
    CURL::libcurl
    hiredis
    pthread
)

# Redis Math Agent（独立进程）
add_executable(redis_math_agent redis_math_agent.cpp)
target_link_libraries(redis_math_agent 
    a2a
    redis_task_store
    CURL::libcurl
    hiredis
    pthread
)

# 交互式客户端
add_executable(interactive_client interactive_client.cpp)
target_link_libraries(interactive_client 
    CURL::libcurl
    pthread
)

# 注册中心服务器
add_executable(registry_server registry_server.cpp)
target_link_libraries(registry_server 
    a2a
    CURL::libcurl
    pthread
)

# 动态服务发现 Orchestrator
add_executable(dynamic_orchestrator dynamic_orchestrator.cpp)
target_link_libraries(dynamic_orchestrator 
    a2a
    redis_task_store
    CURL::libcurl
    hiredis
    pthread
)

# 动态服务发现 Math Agent
add_executable(dynamic_math_agent dynamic_math_agent.cpp)
target_link_libraries(dynamic_math_agent 
    a2a
    redis_task_store
    CURL::libcurl
    hiredis
    pthread
)

# 安装
install(TARGETS 
    redis_orchestrator
    redis_math_agent
    interactive_client
    registry_server
    dynamic_orchestrator
    dynamic_math_agent
    DESTINATION bin
)

# 安装头文件
install(FILES 
    qwen_client.hpp
    http_server.hpp
    redis_task_store.hpp
    DESTINATION include/multi_agent_demo
)
