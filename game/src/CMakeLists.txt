cmake_minimum_required(VERSION 2.6)
project(Manager CXX)

set(MANAGER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/private)
set(MANAGER_SRCS ${MANAGER_SOURCE_DIR}/manager.cpp)
set(MANAGER_LIBRARIES ThreadManager)
set(MANAGER_LINK_LIBRARIES ${MANAGER_LIBRARIES})

add_library(${PROJECT_NAME} STATIC ${MANAGER_SRCS})
target_link_libraries(${PROJECT_NAME} ${MANAGER_LINK_LIBRARIES})

add_subdirectory("private")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/modules")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/external")
