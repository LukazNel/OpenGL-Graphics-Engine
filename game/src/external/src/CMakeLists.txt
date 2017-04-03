add_library(StbImage STATIC stb_image_aug.cpp)
add_library(Glad STATIC glad.cpp)
target_link_libraries(Glad dl)
