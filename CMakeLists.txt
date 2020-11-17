cmake_minimum_required(VERSION 2.8)

project(record-manager)

set(COMMON_ROOT ${CMAKE_SOURCE_DIR}/common/build/install)

set(COMMON_INCLUDE ${COMMON_ROOT}/include)

set(COMMON_LIB ${COMMON_ROOT}/lib)

set(PROJ_INCLUDE ${CMAKE_SOURCE_DIR}/include)

if(WIN32)
else()
find_library(JSON11_LIB libjson11.a PATHS ${COMMON_LIB})
set(CMAKE_CXX_FLAGS  "-std=c++14 -O3")
endif()

include_directories(${PROJ_INCLUDE} ${COMMON_INCLUDE})

add_library(rmp STATIC src/record_manager.cpp)
add_executable(client src/client.cpp)
add_executable(server src/server.cpp)

target_include_directories(rmp PUBLIC ${PROJ_INCLUDE} ${COMMON_INCLUDE})
target_include_directories(client PUBLIC ${PROJ_INCLUDE} ${COMMON_INCLUDE})
target_include_directories(server PUBLIC ${PROJ_INCLUDE} ${COMMON_INCLUDE})

target_link_libraries(rmp ${JSON11_LIB})
target_link_libraries(client rmp)
target_link_libraries(server rmp)