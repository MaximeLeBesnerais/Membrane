cmake_minimum_required(VERSION 3.30)
project(Membrane)

set(CMAKE_CXX_STANDARD 20)

# Development mode option
option(DEV_MODE "Enable development mode with hot reloading" OFF)

# Set variables for cache locations
set(DEPS_CACHE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps" CACHE PATH "Dependency cache directory")
file(MAKE_DIRECTORY ${DEPS_CACHE_DIR})

# Dependencies
include(FetchContent)

# Check dependency cache
if(EXISTS "${DEPS_CACHE_DIR}/json.hash")
  message(STATUS "Using cached nlohmann/json")
else()
  message(STATUS "Fetching nlohmann/json")
endif()

FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  SOURCE_DIR "${DEPS_CACHE_DIR}/json"
)

if(EXISTS "${DEPS_CACHE_DIR}/webview.hash")
  message(STATUS "Using cached webview")
else()
  message(STATUS "Fetching webview")
endif()

FetchContent_Declare(
  webview
  GIT_REPOSITORY https://github.com/webview/webview.git
  GIT_TAG 0.12.0
  SOURCE_DIR "${DEPS_CACHE_DIR}/webview"
)

FetchContent_MakeAvailable(json webview)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH REQUIRED libssh)

# Core include directories
set(MEMBRANE_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/vfs
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/HttpServer
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/FunctionRegistry
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/Membrane_lib
  ${CMAKE_CURRENT_SOURCE_DIR}/res
)

# Use object libraries for faster incremental builds
add_library(vfs OBJECT lib/vfs/vfs.cpp)
target_include_directories(vfs PUBLIC ${MEMBRANE_INCLUDES})

add_library(httpserver OBJECT lib/HttpServer/HttpServer.cpp)
target_include_directories(httpserver PUBLIC ${MEMBRANE_INCLUDES})
target_link_libraries(httpserver PRIVATE vfs)

add_library(FunctionRegistry OBJECT lib/FunctionRegistry/FunctionRegistry.cpp)
target_include_directories(FunctionRegistry PUBLIC ${MEMBRANE_INCLUDES})

add_library(Membrane_lib OBJECT lib/Membrane_lib/Membrane.cpp)
target_include_directories(Membrane_lib PUBLIC ${MEMBRANE_INCLUDES})
target_link_libraries(Membrane_lib PRIVATE vfs httpserver FunctionRegistry webview::core)

# React build handling
if(DEV_MODE)
  # Define development mode for code
  target_compile_definitions(Membrane_lib PUBLIC
    DEV_MODE
    VITE_DEV_SERVER_URL="http://localhost:3000"
  )

  # Create empty resource file for dev mode
  file(WRITE ${CMAKE_BINARY_DIR}/resource_init_dev.cpp
    "// Development mode placeholder\n"
    "#include \"Membrane.hpp\"\n"
    "void initialize_resources(Membrane& app) {}\n"
  )

  set(RESOURCE_INIT_SOURCE ${CMAKE_BINARY_DIR}/resource_init_dev.cpp)
else()
  # Check if React build exists
  if(EXISTS "${CMAKE_SOURCE_DIR}/src-react/dist/index.html")
    # Generate resources only if we need to
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/res/resource_init.cpp" OR 
       "${CMAKE_SOURCE_DIR}/src-react/dist/index.html" IS_NEWER_THAN "${CMAKE_SOURCE_DIR}/res/resource_init.cpp")
      message(STATUS "Generating resource files from React build")
      execute_process(
        COMMAND ${CMAKE_COMMAND} -E env bash ./Gen.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src-react
        RESULT_VARIABLE GEN_RESULT
      )
      if(NOT GEN_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to generate resource files")
      endif()
    else()
      message(STATUS "Using existing resource files")
    endif()
  else()
    message(STATUS "React build not found, will be built by build script")
    # Make sure resources directory exists
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/res)
    
    # Create a default resource file if not already present
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/res/resource_init.cpp")
      file(WRITE ${CMAKE_SOURCE_DIR}/res/resource_init.cpp
        "// Default resource initialization\n"
        "#include \"Membrane.hpp\"\n"
        "void initialize_resources(Membrane& app) {}\n"
      )
      file(WRITE ${CMAKE_SOURCE_DIR}/res/resource_init.hpp
        "#ifndef MEMBRANE_RESOURCE_INIT_HPP\n"
        "#define MEMBRANE_RESOURCE_INIT_HPP\n"
        "class Membrane;\n"
        "void initialize_resources(Membrane& app);\n"
        "#endif // MEMBRANE_RESOURCE_INIT_HPP\n"
      )
    endif()
  endif()

  set(RESOURCE_INIT_SOURCE ${CMAKE_SOURCE_DIR}/res/resource_init.cpp)
endif()

# Main application - combine object libraries for faster incremental builds
add_executable(Membrane
  src/main.cpp
  ${RESOURCE_INIT_SOURCE}
  $<TARGET_OBJECTS:vfs>
  $<TARGET_OBJECTS:httpserver>
  $<TARGET_OBJECTS:FunctionRegistry>
  $<TARGET_OBJECTS:Membrane_lib>
)

target_include_directories(Membrane PRIVATE ${MEMBRANE_INCLUDES})
target_link_libraries(Membrane PRIVATE
  webview::core
  nlohmann_json::nlohmann_json
  ${LIBSSH_LIBRARIES}
)

# Install configuration
install(TARGETS Membrane
  RUNTIME DESTINATION bin
)