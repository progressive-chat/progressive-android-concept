cmake_minimum_required(VERSION 3.22.1)
project(progressive-chat
    VERSION 0.3.0
    DESCRIPTION "Progressive Chat — 100% C++ Android Matrix client"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── FetchContent — all deps downloaded at build time ──
include(FetchContent)

FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(olm
    GIT_REPOSITORY https://gitlab.matrix.org/matrix-org/olm.git
    GIT_TAG 3.2.16
)
FetchContent_MakeAvailable(olm)

# ── Libraries ──
add_subdirectory(matrix-sdk)
add_subdirectory(progressive)

add_subdirectory(library/attachment-viewer)
add_subdirectory(library/multipicker)
add_subdirectory(library/autocomplete)
add_subdirectory(library/diff-match-patch)
add_subdirectory(library/jsonviewer)
add_subdirectory(library/span)
add_subdirectory(library/dialpad)
add_subdirectory(library/barcodescanner)
add_subdirectory(library/textdrawable)

# ── Application shared library (loaded by Java via System.loadLibrary) ──
file(GLOB APP_SOURCES
    app/src/main/cpp/main.cpp
    app/src/main/cpp/native_activity.cpp
    app/src/main/cpp/ui_renderer.cpp
    app/src/main/cpp/ui_input_handler.cpp
    app/src/main/cpp/android_bridge.cpp
)

add_library(progressive_app SHARED ${APP_SOURCES})

target_include_directories(progressive_app PRIVATE app/src/main/cpp)

target_link_libraries(progressive_app
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
        dialpad
        barcodescanner
        textdrawable
        nlohmann_json::nlohmann_json
)

if(ANDROID)
    target_link_libraries(progressive_app PRIVATE android log GLESv3 EGL OpenSLES)
    target_compile_definitions(progressive_app PRIVATE PROGRESSIVE_ANDROID)
endif()

# Output the .so to jniLibs so Gradle picks it up automatically
if(ANDROID)
    set_target_properties(progressive_app PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY
        "${CMAKE_SOURCE_DIR}/app/libs/${ANDROID_ABI}"
    )
endif()
