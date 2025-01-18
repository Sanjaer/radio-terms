cmake_minimum_required (VERSION 3.16)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest CONFIG REQUIRED) 

enable_testing()

add_library(
    commands STATIC
    commands.cpp
)

target_link_libraries(
  commands PUBLIC 
  rt_compiler_flags
)

target_include_directories(commands PRIVATE ${PROJECT_SOURCE_DIR}/content)
target_include_directories(commands PRIVATE ${PROJECT_SOURCE_DIR}/db)
target_include_directories(commands PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# add_executable(
#   commands_test
#   commands_test.cpp
#   commands.cpp
# )

# include(GoogleTest)
# gtest_discover_tests(commands_test)