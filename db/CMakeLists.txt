cmake_minimum_required (VERSION 3.16)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest CONFIG REQUIRED) 

enable_testing()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LMDB REQUIRED lmdb)

add_library(
    db STATIC
    db.cpp
)

target_link_libraries(
  db PUBLIC 
  content
  ${LMDB_LIBRARIES}
  rt_compiler_flags
)

target_include_directories(db PRIVATE ${PROJECT_SOURCE_DIR}/content)
target_include_directories(db PRIVATE ${LMDB_INCLUDE_DIRS})
target_include_directories(db PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# add_executable(
#   db_test
#   db_test.cpp
#   db.cpp
# )

# target_include_directories(db_test PRIVATE ${PROJECT_SOURCE_DIR}/content)

# include(GoogleTest)
# gtest_discover_tests(db_test)