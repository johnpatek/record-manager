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
cmake_minimum_required(VERSION 3.0)

project(record-manager-external)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
if(WIN32)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
list(APPEND EXTRA_LIBS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32 vcruntime)
endif()

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(protobuf/cmake)
add_subdirectory(libuv)
add_subdirectory(googletest)
