cmake_minimum_required(VERSION 3.5)

project(voxel)

file(GLOB_RECURSE SRC
    "voxel/*.cpp"
    "voxel/*.hpp"
)

SET (CMAKE_CXX_FLAGS "-Wall -Wextra -Wcast-qual -Wconversion-null -Wformat-security \
-Wmissing-declarations -Woverlength-strings -Wpointer-arith -Wundef \
-Wunused-local-typedefs -Wunused-result -Wvarargs -Wvla \
-Wwrite-strings -DNOMINMAX -fno-omit-frame-pointer \
-std=c++20 -fPIC -g")

include_directories(".")
include_directories("./lib/")
include_directories("../omega/")
include_directories("../omega/lib/")
include_directories("../omega/lib/glad/include/")
include_directories("../omega/lib/box2d/include/")

add_executable(${PROJECT_NAME} ${SRC})

target_include_directories(${PROJECT_NAME}
    PUBLIC lib/omega/
)

target_link_libraries(${PROJECT_NAME}
    omega
    imgui
    glad
    box2d
    stb
    libtmx-parser
    SDL2
)

target_link_directories(${PROJECT_NAME}
    PUBLIC ../omega/build/
    PUBLIC ../omega/build/lib/imgui/
    PUBLIC ../omega/build/lib/box2d/bin
    PUBLIC ../omega/build/lib/glad/
    PUBLIC ../omega/build/lib/stb/
    PUBLIC ../omega/build/lib/libtmx-parser/
)

