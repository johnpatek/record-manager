#######################################################################
# Copyright (c) 2020 John R. Patek
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

set(COMMON_ROOT ${CMAKE_SOURCE_DIR}/common/build/install)

set(COMMON_INCLUDE ${COMMON_ROOT}/include)

set(COMMON_LIB ${COMMON_ROOT}/lib)

set(PROJ_INCLUDE ${CMAKE_SOURCE_DIR}/include)

if(WIN32)
else()
find_library(JSON11_LIB libjson11.a PATHS ${COMMON_LIB})
set(CMAKE_CXX_FLAGS  "-std=c++14 -O3 -pthread")
endif()

include_directories(${PROJ_INCLUDE} ${COMMON_INCLUDE})

add_library(rmp STATIC src/record_manager.cpp)
add_executable(client src/client.cpp)
add_executable(server src/server.cpp)
add_executable(unittest test/test.cpp)

target_include_directories(rmp PUBLIC ${PROJ_INCLUDE} ${COMMON_INCLUDE})
target_include_directories(client PUBLIC ${PROJ_INCLUDE} ${COMMON_INCLUDE})
target_include_directories(server PUBLIC ${PROJ_INCLUDE} ${COMMON_INCLUDE})
target_include_directories(unittest PUBLIC ${PROJ_INCLUDE} ${COMMON_INCLUDE})

target_link_libraries(rmp ${JSON11_LIB})
target_link_libraries(client rmp)
target_link_libraries(server rmp)
target_link_libraries(unittest rmp)