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

add_library(httpserver STATIC lib/HttpServer/HttpServer.cpp)
target_include_directories(httpserver PUBLIC lib/HttpServer)
target_link_libraries(httpserver PRIVATE vfs)

add_library(Membrane_lib STATIC lib/Membrane_lib/Membrane.cpp)
target_include_directories(Membrane_lib PUBLIC lib/Membrane_lib)
target_link_libraries(Membrane_lib PRIVATE vfs httpserver webview::core)

add_executable(Membrane
        src/main.cpp
        res/aggregate.hpp
)
target_include_directories(Membrane PRIVATE lib)
target_include_directories(Membrane PRIVATE res)
target_link_libraries(Membrane PRIVATE webview::core nlohmann_json::nlohmann_json vfs httpserver Membrane_lib)
