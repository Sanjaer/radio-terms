cmake_minimum_required (VERSION 3.16)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest CONFIG REQUIRED) 

enable_testing()

add_library(
  content STATIC
  content.cpp
)

target_link_libraries(
  content PUBLIC 
  rt_compiler_flags
)

target_include_directories(
    content PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# add_executable(
#   content_test
#   content_test.cpp
#   content.cpp
# )

# include(GoogleTest)
# gtest_discover_tests(content_test)