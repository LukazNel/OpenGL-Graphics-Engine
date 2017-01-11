cmake_minimum_required (VERSION 2.6)
project (GameUnknown CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(PROJECT_SRCS ${PROJECT_SOURCE_DIR}/main.cpp)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
add_definitions(-std=c++14 -O0)

add_executable(GameUnknown ${PROJECT_SRCS})
target_link_libraries(GameUnknown Manager Logger Renderer Window)

add_subdirectory(src)
