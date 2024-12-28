cmake_minimum_required(VERSION 3.5)

project(voxel)

file(GLOB_RECURSE SRC
    "voxel/*.cpp"
    "voxel/*.hpp"
)

set (CMAKE_CXX_STANDARD 20)

include_directories(".")
include_directories("./lib/")
include_directories("./lib/omega/")
include_directories("./lib/omega/lib/SDL3/include/")
include_directories("./lib/omega/lib/SDL_image/include/")
include_directories("./lib/omega/lib/SDL_mixer/include/")
include_directories("./lib/omega/lib/glad/include/")
include_directories("./lib/omega/lib/box2d/include/")

add_executable(${PROJECT_NAME} ${SRC})

target_include_directories(${PROJECT_NAME}
    PUBLIC ./lib/omega/
    PUBLIC ./lib/omega/lib/
    PUBLIC ./lib/omega/lib/SDL/include/
    PUBLIC ./lib/omega/lib/SDL_image/include/
    PUBLIC ./lib/omega/lib/SDL_mixer/include/
)

target_link_libraries(${PROJECT_NAME}
    omega
    freetype
    SDL3
    SDL3_image
    SDL3_mixer
    imgui
    glad
    stb
    box2d
    libtmx-parser
    tomlplusplus
    entt
)

target_link_directories(${PROJECT_NAME}
    PUBLIC ./lib/omega/build/
    PUBLIC ./lib/omega/build/lib/SDL/
    PUBLIC ./lib/omega/build/lib/SDL_image/
    PUBLIC ./lib/omega/build/lib/SDL_mixer/
    PUBLIC ./lib/omega/build/lib/imgui/
    PUBLIC ./lib/omega/build/lib/glad/
    PUBLIC ./lib/omega/build/lib/stb/
    PUBLIC ./lib/omega/build/lib/box2d/bin/
    PUBLIC ./lib/omega/build/lib/libtmx-parser/
    PUBLIC ./lib/omega/build/lib/tomlplusplus/
    PUBLIC ./lib/omega/build/lib/json/
    PUBLIC ./lib/omega/build/lib/entt/
)
