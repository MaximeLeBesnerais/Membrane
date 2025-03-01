cmake_minimum_required(VERSION 3.30)
project(Membrane)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
        webview
        GIT_REPOSITORY https://github.com/webview/webview.git
        GIT_TAG 0.12.0)
FetchContent_MakeAvailable(webview)

add_library(vfs STATIC lib/vfs/vfs.cpp)
target_include_directories(vfs PUBLIC lib/vfs)

add_executable(Membrane
        # entry point
        src/main.cpp
        # Membrane
        # lib
        res/headers/about.hpp
        res/headers/index.hpp
        res/aggregate.hpp
)
target_include_directories(Membrane PRIVATE lib)
target_include_directories(Membrane PRIVATE res)
target_link_libraries(Membrane PRIVATE webview::core nlohmann_json::nlohmann_json vfs)
