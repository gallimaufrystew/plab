cmake_minimum_required(VERSION 3.3)
project(eventfd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp coordinator.hpp lock.hpp main.cpp test.cpp)
add_executable(eventfd ${SOURCE_FILES})