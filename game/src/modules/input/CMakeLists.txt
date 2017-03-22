cmake_minimum_required(VERSION 2.6)
project(Input CXX)

set(MODULE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/private)
set(MODULE_SRCS ${MODULE_SOURCE_DIR}/input.cpp)
set(MODULE_LIBRARIES)
set(MODULE_LINK_LIBRARIES ${MODULE_LIBRARIES})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
add_library(${PROJECT_NAME} STATIC ${MODULE_SRCS} )
target_link_libraries(${PROJECT_NAME} ${MODULE_LINK_LIBRARIES})

add_subdirectory("private")
