cmake_minimum_required(VERSION 3.22.0)
project(progressive_native VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE PROGRESSIVE_SOURCES "src/*.cpp")
file(GLOB_RECURSE PROGRESSIVE_HEADERS "include/*.hpp" "include/*.h")

add_library(progressive STATIC ${PROGRESSIVE_SOURCES} ${PROGRESSIVE_HEADERS})

target_include_directories(progressive
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(progressive
    PUBLIC
        olm
        nlohmann_json::nlohmann_json
)

if(ANDROID)
    target_link_libraries(progressive
        PUBLIC
            android
            log
    )
    target_compile_definitions(progressive PRIVATE PROGRESSIVE_ANDROID)
elseif(APPLE)
    target_compile_definitions(progressive PRIVATE PROGRESSIVE_APPLE)
    target_link_libraries(progressive PRIVATE pthread)
elseif(WIN32)
    target_compile_definitions(progressive PRIVATE PROGRESSIVE_WINDOWS)
elseif(UNIX)
    target_compile_definitions(progressive PRIVATE PROGRESSIVE_LINUX)
    target_link_libraries(progressive PRIVATE pthread)
endif()

# OpenSSL (for crypto operations)
if(OpenSSL_FOUND OR ANDROID)
    target_link_libraries(progressive PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(progressive PRIVATE PROGRESSIVE_HAS_OPENSSL)
endif()
