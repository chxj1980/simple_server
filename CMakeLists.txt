cmake_minimum_required(VERSION 3.8)
project(simple_server)

include_directories(src)
include_directories(lib/glog)
include_directories(example)

link_directories(lib/glog)

add_definitions("-Wall -std=c++11")

file(GLOB_RECURSE HERA_SOURCES "src/*.cc")
add_executable(simple_server example/simple_server.cc ${HERA_SOURCES})
target_link_libraries(simple_server glog)
