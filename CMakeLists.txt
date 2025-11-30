cmake_minimum_required(VERSION 3.15)
project(a2a-cpp VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Find dependencies
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/third_party)

# Source files
set(A2A_SOURCES
    # Core
    src/core/exception.cpp
    src/core/jsonrpc_request.cpp
    src/core/jsonrpc_response.cpp
    src/core/http_client.cpp
    
    # Models
    src/models/message_part.cpp
    src/models/agent_message.cpp
    src/models/task_status.cpp
    src/models/artifact.cpp
    src/models/agent_task.cpp
    src/models/agent_card.cpp
    src/models/message_send_params.cpp
    
    # Client
    src/client/card_resolver.cpp
    src/client/a2a_client.cpp
    
    # Server
    src/server/memory_task_store.cpp
    src/server/task_manager.cpp
)

# Header files
set(A2A_HEADERS
    # Core
    include/a2a/core/types.hpp
    include/a2a/core/error_code.hpp
    include/a2a/core/exception.hpp
    include/a2a/core/a2a_methods.hpp
    include/a2a/core/jsonrpc_request.hpp
    include/a2a/core/jsonrpc_response.hpp
    include/a2a/core/http_client.hpp
    
    # Models
    include/a2a/models/message_part.hpp
    include/a2a/models/agent_message.hpp
    include/a2a/models/task_status.hpp
    include/a2a/models/artifact.hpp
    include/a2a/models/agent_task.hpp
    include/a2a/models/agent_card.hpp
    include/a2a/models/message_send_params.hpp
    include/a2a/models/a2a_response.hpp
    
    # Client
    include/a2a/client/card_resolver.hpp
    include/a2a/client/a2a_client.hpp
    
    # Server
    include/a2a/server/task_store.hpp
    include/a2a/server/memory_task_store.hpp
    include/a2a/server/task_manager.hpp
)

# Create library
add_library(a2a ${A2A_SOURCES} ${A2A_HEADERS})

target_include_directories(a2a
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(a2a
    PUBLIC
        CURL::libcurl
        Threads::Threads
)

# Compiler warnings
if(MSVC)
    target_compile_options(a2a PRIVATE /W4)
else()
    target_compile_options(a2a PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    # TODO: Add tests directory
    # add_subdirectory(tests)
endif()

# Installation
install(TARGETS a2a
    EXPORT a2a-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/a2a
    DESTINATION include
)

install(EXPORT a2a-targets
    FILE a2a-config.cmake
    NAMESPACE a2a::
    DESTINATION lib/cmake/a2a
)
