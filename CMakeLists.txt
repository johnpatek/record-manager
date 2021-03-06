#######################################################################
# Copyright (c) 2021 John R. Patek
# 
# This software is provided 'as-is', without any express or implied 
# warranty. In no event will the authors be held liable for any 
# damages arising from the use of this software.
# 
# Permission is granted to anyone to use this software for any 
# purpose, including commercial applications, and to alter it and 
# redistribute it freely, subject to the following restrictions:
# 
#    1. The origin of this software must not be misrepresented; you 
#       must not claim that you wrote the original software. If you 
#       use this software in a product, an acknowledgment in the 
#       product documentation would be appreciated but is not 
#       required.
#    
#    2. Altered source versions must be plainly marked as such, and 
#       must not be misrepresented as being the original software.
#    
#    3. This notice may not be removed or altered from any source 
#       distribution.
# 
#####################################################################
cmake_minimum_required(VERSION 2.8)

project(record-manager)

set(EXTERNAL_ROOT ${CMAKE_SOURCE_DIR}/external/build/install)

set(EXTERNAL_INCLUDE ${EXTERNAL_ROOT}/include)

set(EXTERNAL_LIB ${EXTERNAL_ROOT}/lib)

set(PROJ_INCLUDE ${CMAKE_SOURCE_DIR}/include)

set(OBJ_INCLUDE ${CMAKE_SOURCE_DIR}/objects)

if(WIN32)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  find_library(PROTOBUF_LIB libprotobuf.lib PATHS ${EXTERNAL_LIB})
  find_library(UV_LIB uv_a.lib PATHS ${EXTERNAL_LIB}/Release)
  find_library(GTEST gtest.lib PATHS ${EXTERNAL_LIB})
  find_library(GTEST_MAIN gtest_main.lib PATHS ${EXTERNAL_LIB})
  set(DL "")
else()
  find_library(PROTOBUF_LIB libprotobuf.a PATHS ${EXTERNAL_LIB})
  find_library(UV_LIB libuv_a.a PATHS ${EXTERNAL_LIB})
  find_library(GTEST libgtest.a PATHS ${EXTERNAL_LIB})
  find_library(GTEST_MAIN libgtest_main.a PATHS ${EXTERNAL_LIB})
  set(DL dl)
  set(CMAKE_CXX_FLAGS  "-std=c++14 -O3 -pthread -Wall")
endif()

include_directories(${PROJ_INCLUDE} ${EXTERNAL_INCLUDE})

add_library(rmp-obj STATIC objects/rmp.pb.h objects/rmp.pb.cc)
add_library(rmp STATIC src/record_manager.cpp)
add_executable(client src/client.cpp)
add_executable(server src/server.cpp)
add_executable(unittest test/test.cpp test/test.h)

target_include_directories(rmp-obj PUBLIC ${PROJ_INCLUDE} ${EXTERNAL_INCLUDE} ${OBJ_INCLUDE})
target_include_directories(rmp PUBLIC ${PROJ_INCLUDE} ${EXTERNAL_INCLUDE} ${OBJ_INCLUDE})
target_include_directories(client PUBLIC ${PROJ_INCLUDE} ${EXTERNAL_INCLUDE} ${OBJ_INCLUDE})
target_include_directories(server PUBLIC ${PROJ_INCLUDE} ${EXTERNAL_INCLUDE} ${OBJ_INCLUDE})
target_include_directories(unittest PUBLIC ${PROJ_INCLUDE} ${EXTERNAL_INCLUDE} ${OBJ_INCLUDE})

target_link_libraries(rmp-obj ${PROTOBUF_LIB})
target_link_libraries(rmp rmp-obj ${PROTOBUF_LIB} ${UV_LIB} ${DL})
target_link_libraries(client rmp ${PROTOBUF_LIB})
target_link_libraries(server rmp ${PROTOBUF_LIB})
target_link_libraries(unittest rmp ${PROTOBUF_LIB} ${GTEST} ${GTEST_MAIN})