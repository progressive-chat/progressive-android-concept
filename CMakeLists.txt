cmake_minimum_required(VERSION 3.22.1)
project(progressive-chat
    VERSION 0.3.0
    DESCRIPTION "Progressive Chat — 100% C++ Android Matrix/IRC/Lemmy client"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── FetchContent for build-time dependencies ──
include(FetchContent)

# nlohmann/json
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# OpenSSL (via FetchContent or find_package on Android)
if(ANDROID)
    find_package(OpenSSL QUIET)
    if(NOT OpenSSL_FOUND)
        message(STATUS "OpenSSL not found via find_package; FetchContent fallback not supported on Android NDK.")
        message(STATUS "Ensure OpenSSL prebuilt libs are in the NDK sysroot or provide OPENSSL_ROOT_DIR.")
    endif()
else()
    find_package(OpenSSL REQUIRED)
endif()

# ── libolm (bundled vendored copy) ──
add_subdirectory(libolm)

# ── Matrix SDK C++ library (equivalent to matrix-sdk-android) ──
add_subdirectory(matrix-sdk)

# ── Progressive core library ──
add_subdirectory(progressive)

# ── Library modules ──
add_subdirectory(library/attachment-viewer)
add_subdirectory(library/multipicker)
add_subdirectory(library/autocomplete)
add_subdirectory(library/diff-match-patch)
add_subdirectory(library/jsonviewer)
add_subdirectory(library/span)
add_subdirectory(library/dialpad)
add_subdirectory(library/barcodescanner)
add_subdirectory(library/textdrawable)

# ── Main application ──
add_subdirectory(app)
