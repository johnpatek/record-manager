cmake_minimum_required(VERSION 3.0)

project(record-manager-common)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
if(WIN32)
list(APPEND EXTRA_LIBS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32 vcruntime)
endif()
add_subdirectory(json11)