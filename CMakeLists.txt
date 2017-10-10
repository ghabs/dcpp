# Get the exercise name from the current directory (changed to server)
get_filename_component(exercise ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# Basic CMake project
cmake_minimum_required(VERSION 2.8.11)

# Name the project after the exercise
project(${exercise} CXX)

find_package(Boost 1.59 REQUIRED COMPONENTS system filesystem)

include_directories(${Boost_INCLUDE_DIRS})

# Name the project after the exercise
project(${exercise} CXX)

# Enable C++11 features on gcc/clang
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "(GNU|Clang)")
    set(CMAKE_CXX_FLAGS "-std=c++11 -pthread")
endif()

# Get a source filename from the exercise name by replacing -'s with _'s
string(REPLACE "-" "_" file ${exercise})

# Implementation could be only a header
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${file}.cpp)
    set(exercise_cpp ${file}.cpp)
else()
    set(exercise_cpp "")
endif()

# Build executable from sources and headers
add_executable(${exercise} ${exercise_cpp} ${file}.h)
add_library(socket "socket/socket.cpp" "socket/socket.h")

target_link_libraries(${exercise} ${Boost_LIBRARIES})
target_link_libraries(${exercise} socket)
