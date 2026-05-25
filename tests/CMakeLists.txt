cmake_minimum_required(VERSION 3.22.0)
project(progressive_tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE TEST_SOURCES "*.cpp")

add_executable(progressive_tests ${TEST_SOURCES})

target_include_directories(progressive_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/progressive/include
    ${CMAKE_SOURCE_DIR}/matrix-sdk/src/main/cpp
)

target_link_libraries(progressive_tests
    PRIVATE
        progressive
        matrix_sdk
        olm
        attachment_viewer
        multipicker
        autocomplete
        diff_match_patch
        jsonviewer
        span
        nlohmann_json::nlohmann_json
)

if(ANDROID)
    target_link_libraries(progressive_tests PRIVATE android log)
endif()

enable_testing()
add_test(NAME progressive_tests COMMAND progressive_tests)
