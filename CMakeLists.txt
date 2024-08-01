cmake_minimum_required(VERSION 3.10)
project(MyProject VERSION 1.0)

# 设置 C++ 标准为 C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(code ${src_dir} src/main.cpp)