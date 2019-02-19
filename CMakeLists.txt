cmake_minimum_required(VERSION 3.8)
project(simple_server)

include_directories(src)
include_directories(lib/glog)
include_directories(example)

link_directories(lib/glog)

add_definitions("-Wall -std=c++11")

# add_executable(simple_server src/server.cc src/net.cc src/connection.cc)
# target_link_libraries(simple_server glog)

add_executable(simple_server example/simple_server.cc src/server.cc src/net.cc src/connection.cc)
target_link_libraries(simple_server glog)

# add_executable(simple_client src/client.cc)
# target_link_libraries(simple_client glog)
