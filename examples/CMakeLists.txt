# Examples CMakeLists.txt

# Client Demo
add_executable(client_demo
    client_demo/main.cpp
)

target_link_libraries(client_demo
    PRIVATE a2a
)

# Echo Agent
add_executable(echo_agent
    echo_agent/main.cpp
)

target_link_libraries(echo_agent
    PRIVATE a2a
)

# JSON-RPC Test
add_executable(jsonrpc_test
    jsonrpc_test.cpp
)

target_link_libraries(jsonrpc_test
    PRIVATE a2a
)

# Multi-Agent Demo
add_subdirectory(multi_agent_demo)

# Installation
install(TARGETS client_demo echo_agent jsonrpc_test
    RUNTIME DESTINATION bin/examples
)
