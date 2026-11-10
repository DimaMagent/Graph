cmake_minimum_required(VERSION 3.20)
project(DependencyVisualizer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dep_visualizer
    src/main.cpp
    src/Config.hpp
)
target_include_directories(dep_visualizer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)