cmake_minimum_required(VERSION 3.10)
project(monorepo)

add_subdirectory(mockos)
add_subdirectory(prosched)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)