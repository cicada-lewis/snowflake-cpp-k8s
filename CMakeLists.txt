cmake_minimum_required(VERSION 3.17)
project(snowflake_cpp_k8s)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(${GTEST_INCLUDE_DIR})
link_directories(${GTEST_LIB_DIR})

enable_testing()

add_executable(snowflake_test test/test_snowflake.cpp)
target_link_libraries(snowflake_test gtest_main gtest)

include(GoogleTest)
gtest_discover_tests(snowflake_test)

add_executable(snowflake_benchmark test/benchmark_snowflake.cpp)
target_link_libraries(snowflake_benchmark benchmark)

add_executable(main main.cpp)

