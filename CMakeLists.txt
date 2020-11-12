cmake_minimum_required(VERSION 2.8)

project(record-manager)

if(WIN32)
else()
set(CMAKE_CXX_FLAGS  "-std=c++14 -O3")
endif()

include_directories(include common/json11/include)

add_library(rmp STATIC src/record_manager.cpp)
add_executable(client src/client.cpp)
add_executable(server src/server.cpp)

target_include_directories(rmp PUBLIC include)
target_include_directories(client PUBLIC include)
target_include_directories(server PUBLIC include)

target_link_libraries(client rmp)
target_link_libraries(server rmp)