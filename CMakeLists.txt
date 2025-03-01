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

add_executable(Membrane
        # entry poinr
        src/main.cpp
        # Membrane
        # lib
        res/about.hpp
        res/index.hpp
)
target_include_directories(Membrane PRIVATE include)
target_include_directories(Membrane PRIVATE res)
target_link_libraries(Membrane PRIVATE webview::core nlohmann_json::nlohmann_json)