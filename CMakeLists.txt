cmake_minimum_required(VERSION 3.17)
project(EventLoop)

set(CMAKE_CXX_STANDARD 11)

add_executable(EventLoop main.cpp WorkerThread.h Fault.h Fault.cpp)