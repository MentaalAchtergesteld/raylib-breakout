cmake_minimum_required(VERSION 3.25.0)
project(breakout LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)

set(RAYLIB_DIR "/usr/lib/raylib-5.5_linux_amd64")

include_directories(${RAYLIB_DIR}/include)
link_directories(${RAYLIB_DIR}/lib)

add_executable(breakout main.c)
target_compile_options(breakout PRIVATE -g -fsanitize=address,undefined)
target_link_options(breakout PRIVATE -fsanitize=address,undefined)

target_link_libraries(breakout raylib m dl pthread)
