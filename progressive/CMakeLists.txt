cmake_minimum_required(VERSION 3.22)

add_library(progressive_native STATIC progressive_stubs.cpp)

target_include_directories(progressive_native PUBLIC include)

target_compile_features(progressive_native PUBLIC cxx_std_20)

target_link_libraries(progressive_native PUBLIC olm Threads::Threads)
