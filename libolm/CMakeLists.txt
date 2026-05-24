add_library(olm STATIC "")

target_include_directories(olm PUBLIC include)

target_compile_features(olm PUBLIC cxx_std_20)
