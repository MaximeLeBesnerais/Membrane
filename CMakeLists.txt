cmake_minimum_required(VERSION 3.30)
project(Membrane)

set(CMAKE_CXX_STANDARD 20)

# Development mode option
option(DEV_MODE "Enable development mode with hot reloading" OFF)

# Dependencies
include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
        webview
        GIT_REPOSITORY https://github.com/webview/webview.git
        GIT_TAG 0.12.0)
FetchContent_MakeAvailable(webview)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH REQUIRED libssh)

# Core libraries
add_library(vfs SHARED lib/vfs/vfs.cpp)
target_include_directories(vfs PUBLIC lib/vfs)

add_library(httpserver SHARED lib/HttpServer/HttpServer.cpp)
target_include_directories(httpserver PUBLIC lib/HttpServer)
target_link_libraries(httpserver PRIVATE vfs)

add_library(FunctionRegistry SHARED lib/FunctionRegistry/FunctionRegistry.cpp)
target_include_directories(FunctionRegistry PUBLIC lib/FunctionRegistry)

add_library(Membrane_lib SHARED lib/Membrane_lib/Membrane.cpp)
target_include_directories(Membrane_lib PUBLIC lib/Membrane_lib)
target_link_libraries(Membrane_lib PRIVATE vfs httpserver FunctionRegistry webview::core)


# React build handling
if(DEV_MODE)
    # Define development mode for code - no need to start Vite here, Makefile handles it
    target_compile_definitions(Membrane_lib PUBLIC
            DEV_MODE
            VITE_DEV_SERVER_URL="http://localhost:5173"
    )

    # Create empty resource file for dev mode
    file(WRITE ${CMAKE_BINARY_DIR}/resource_init_dev.cpp
            "// Development mode placeholder\n"
            "#include \"Membrane.hpp\"\n"
            "void initialize_resources(Membrane& app) {}\n"
    )

    set(RESOURCE_INIT_SOURCE ${CMAKE_BINARY_DIR}/resource_init_dev.cpp)
else()
    # Production mode - create custom command to build React and generate resources
    add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/resource_build_stamp
            COMMAND cd ${CMAKE_SOURCE_DIR}/src-react && npm run build
            COMMAND cd ${CMAKE_SOURCE_DIR}/src-react && ./Gen.sh
            COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/resource_build_stamp
            DEPENDS
            ${CMAKE_SOURCE_DIR}/src-react/src
            COMMENT "Building React app and generating resources"
    )

    add_custom_target(react_build
            DEPENDS ${CMAKE_BINARY_DIR}/resource_build_stamp
    )

    set(RESOURCE_INIT_SOURCE ${CMAKE_SOURCE_DIR}/res/resource_init.cpp)
endif()

# Main application
add_executable(Membrane
        src/main.cpp
        lib/FunctionRegistry/FunctionRegistry.hpp
        lib/Membrane_lib/Membrane.hpp
        lib/vfs/vfs.hpp
        lib/HttpServer/HttpServer.hpp
        ${RESOURCE_INIT_SOURCE}
)

target_include_directories(Membrane PRIVATE lib)
target_include_directories(Membrane PRIVATE res)
target_link_libraries(Membrane PRIVATE
        webview::core
        nlohmann_json::nlohmann_json
        ${LIBSSH_LIBRARIES}
        # Membrane dependencies
        vfs
        httpserver
        Membrane_lib
        FunctionRegistry
)

# Set up dependencies - only for production mode
if(NOT DEV_MODE)
    add_dependencies(Membrane react_build)
endif()

# Install configuration
install(TARGETS Membrane vfs httpserver Membrane_lib FunctionRegistry
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)
