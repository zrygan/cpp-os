project(prosched)

cmake_minimum_required(VERSION 3.20)

add_executable(prosched 
  src/main.cpp
  src/Controller/Controller.cpp
  src/View/View.cpp
  src/View/Page/Menu.cpp
)

target_include_directories(prosched PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(prosched PRIVATE
  -Wall -Wextra -Wpedantic
)