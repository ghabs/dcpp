# Basic CMake project
cmake_minimum_required(VERSION 2.8.11)

find_package(Boost 1.59 REQUIRED COMPONENTS system filesystem unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "(GNU|Clang)")
    set(CMAKE_CXX_FLAGS "-std=c++11 -pthread")
endif()

add_executable(storage_tests "storage_tests.cpp" "../storage/storage.h")
target_link_libraries(storage_tests ${Boost_LIBRARIES})
