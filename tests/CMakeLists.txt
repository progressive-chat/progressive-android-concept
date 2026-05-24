cmake_minimum_required(VERSION 3.22.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Test)

file(GLOB TEST_SOURCES "*.cpp")

add_executable(progressive_tests ${TEST_SOURCES})

target_include_directories(progressive_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/progressive/include
)

target_link_libraries(progressive_tests
    PRIVATE
        progressive
        Qt6::Test
        Qt6::Core
        Qt6::Network
)

add_test(NAME progressive_tests COMMAND progressive_tests)
