cmake_minimum_required(VERSION 3.20)
project(prosched)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)
enable_testing()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/config.txt"
    COPYONLY
)

add_executable(prosched
  # src/ 
  src/main.cpp
  src/controller/Controller.cpp
  src/view/View.cpp
  src/view/page/Menu.cpp

  # scheduler
  src/scheduler/Scheduler.cpp
  src/scheduler/process/Process.cpp
  src/scheduler/worker/Worker.cpp
)
target_include_directories(prosched PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(prosched PRIVATE -Wall -Wextra -Wpedantic)

add_executable(prosched_tests
  tests/SchedulerTest.cpp
  # tests/WorkerTest.cpp
  tests/ProcessTest.cpp
  src/scheduler/Scheduler.cpp
  src/scheduler/process/Process.cpp
  src/scheduler/worker/Worker.cpp
  src/commands/Command.cpp
)
target_include_directories(prosched_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(prosched_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(prosched_tests)