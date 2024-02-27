cmake_minimum_required(VERSION 3.25.1)
project(gbda)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -fms-extensions -O3)

add_subdirectory(core)
add_subdirectory(desktop)
