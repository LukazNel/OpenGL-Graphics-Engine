cmake_minimum_required(VERSION 2.6)
project(Renderer CXX)

set(MODULE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/private)
set(MODULE_SRCS ${MODULE_SOURCE_DIR}/renderer.cpp)
set(MODULE_LIBRARIES ProgramManager BufferManager UniformManager)
set(MODULE_LINK_LIBRARIES ${MODULE_LIBRARIES} dl ${OPENGL_glu_LIBRARY} StbImage)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
add_library(${PROJECT_NAME} STATIC ${MODULE_SRCS} )
target_link_libraries(${PROJECT_NAME} ${MODULE_LINK_LIBRARIES})

add_subdirectory("private")
