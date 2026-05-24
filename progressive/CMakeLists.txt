cmake_minimum_required(VERSION 3.22)
project(progressive_native LANGUAGES CXX)

add_library(progressive_native STATIC
    src/progressive_stubs.cpp
)

target_include_directories(progressive_native PUBLIC include)

target_compile_features(progressive_native PUBLIC cxx_std_20)
