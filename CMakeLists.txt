cmake_minimum_required(VERSION 3.17)
project(EventLoop)

#set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xpreprocessor -fopenmp -std=c++11")

add_executable(EventLoop main.cpp WorkerThread.h Fault.h Fault.cpp)

find_package(OpenMP REQUIRED)
target_link_libraries(EventLoop PRIVATE OpenMP::OpenMP_CXX)