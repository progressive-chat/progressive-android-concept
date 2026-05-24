cmake_minimum_required(VERSION 3.22)

add_library(olm STATIC olm_stub.cpp)
target_include_directories(olm PUBLIC include)
target_compile_features(olm PUBLIC cxx_std_20)
