cmake_minimum_required (VERSION 3.16)
project(rt VERSION 0.0.1)

# Set compiler flags for the project
add_library(rt_compiler_flags INTERFACE)
# GoogleTest requires at least C++14
target_compile_features(rt_compiler_flags INTERFACE cxx_std_20)
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(rt_compiler_flags INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

set(CMAKE_BUILD_PARALLEL_LEVEL ${CMAKE_CPACK_PARALLEL_LEVEL})

find_package(GTest REQUIRED)
enable_testing()

add_subdirectory(content)
add_subdirectory(db)
add_subdirectory(commands)

add_executable(rt main.cpp)

get_filename_component(PROJECT_DIR_NAME ${CMAKE_SOURCE_DIR} NAME)

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink 
  $<TARGET_FILE:${PROJECT_NAME}>
  ${CMAKE_BINARY_DIR}/${PROJECT_DIR_NAME}
)

target_link_libraries(rt PUBLIC content db commands)
