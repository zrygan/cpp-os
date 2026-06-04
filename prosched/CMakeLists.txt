cmake_minimum_required(VERSION 3.20)
project(prosched)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(prosched 
  src/main.cpp
  src/controller/Controller.cpp
  src/view/View.cpp
  src/view/page/Menu.cpp
)

target_include_directories(prosched PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(prosched PRIVATE
  -Wall -Wextra -Wpedantic
)