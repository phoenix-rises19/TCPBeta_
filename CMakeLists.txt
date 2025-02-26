cmake_minimum_required(VERSION 3.30)
project(TCP_Beta)

set(CMAKE_CXX_STANDARD 20)

find_package(nlohmann_json REQUIRED)
message(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ethernet_packet )

add_executable(TCP_Beta main.cpp
        ethernet_packet/message.cpp
        tcp_stack/machine_state.cpp
        tcp_stack/packet_transfer.cpp
        config.cpp)

add_executable( os_echo_server
        server/os_echo_server.cpp
        server/os_echo_server.hpp)

target_link_libraries(TCP_Beta PRIVATE nlohmann_json::nlohmann_json)
