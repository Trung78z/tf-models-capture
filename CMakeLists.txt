cmake_minimum_required(VERSION 3.10)
project(capture)

find_package(OpenCV REQUIRED)

add_executable(capture cap_runtime.cpp)
target_link_libraries(capture ${OpenCV_LIBS})

set(CMAKE_CXX_STANDARD 11)
