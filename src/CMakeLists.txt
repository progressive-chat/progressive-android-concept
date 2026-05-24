cmake_minimum_required(VERSION 3.16)

add_executable(progressive_android_concept main.cpp)

file(GLOB_RECURSE APP_SOURCES
    app/*.cpp
    native/*.cpp
    protocol/*.cpp
    matrix/*.cpp
    irc/*.cpp
    lemmy/*.cpp
    features/auth/*.cpp
    features/home/*.cpp
    features/irc/*.cpp
    features/lemmy/*.cpp
    features/matrix_chat/*.cpp
    ui/components/*.cpp
    ui/themes/*.cpp
    util/*.cpp
)

target_sources(progressive_android_concept PRIVATE ${APP_SOURCES})

target_include_directories(progressive_android_concept PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/progressive/include
)

target_link_libraries(progressive_android_concept PRIVATE
    progressive_native
    olm
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::Core
)
if(TARGET Qt6::Multimedia)
    target_link_libraries(progressive_android_concept PRIVATE Qt6::Multimedia)
    target_compile_definitions(progressive_android_concept PRIVATE HAS_QT_MULTIMEDIA=1)
endif()
