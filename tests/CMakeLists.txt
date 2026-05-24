enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Test)

set(TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/protocol/protocol_session.cpp
    ${CMAKE_SOURCE_DIR}/src/protocol/protocol_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/util/json_util.cpp
    ${CMAKE_SOURCE_DIR}/src/util/string_util.cpp
)

add_executable(progressive_tests test_main.cpp ${TEST_SOURCES})
target_include_directories(progressive_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/progressive/include
)

target_link_libraries(progressive_tests PRIVATE
    Qt6::Test
    Qt6::Network
    Qt6::Core
)

add_test(NAME progressive_tests COMMAND progressive_tests)
