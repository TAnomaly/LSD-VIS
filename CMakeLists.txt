cmake_minimum_required(VERSION 3.10)
project(music_sequencer C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)

# GLFW
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

# Main executable
add_executable(music_sequencer src/main.c)

target_link_libraries(music_sequencer PRIVATE
    OpenGL::GL
    glfw
    winmm
)

# Copy sounds directory to build directory
add_custom_command(TARGET music_sequencer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/sounds
    ${CMAKE_BINARY_DIR}/Release/sounds
) 