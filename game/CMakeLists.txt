cmake_minimum_required (VERSION 2.6)
project (A_Blocks_League CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(PROJECT_SRCS ${PROJECT_SOURCE_DIR}/main.cpp)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

if (MINGW)
  include_directories("/usr/i686-w64-mingw32/include")
  set(CMAKE_CXX_FLAGS "-static")
endif (MINGW)

set(CMAKE_CXX_FLAGS "-std=c++14 -O0")

add_executable(${PROJECT_NAME} ${PROJECT_SRCS})
target_link_libraries(${PROJECT_NAME} Manager Logger Renderer Window Input Client)

add_subdirectory(src)
