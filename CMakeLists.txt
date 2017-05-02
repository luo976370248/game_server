cmake_minimum_required(VERSION 3.6)
project(libv_test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(/proj.mac/libuv/include)
link_directories(/proj.mac/libuv/lib/libuv)

set(SOURCE_FILES main.cpp)
add_executable(libv_test ${SOURCE_FILES})
target_link_libraries(libv_test libuv)