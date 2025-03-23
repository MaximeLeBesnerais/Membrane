cmake_minimum_required(VERSION 3.30)
project(Membrane)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(BINARY_NAME "Membrane" CACHE STRING "Set the binary name")

# Development mode option
option(DEV_MODE "Enable development mode with hot reloading" OFF)
option(BUILD_TESTS "Build tests for library components" OFF)

# Set variables for cache locations
set(DEPS_CACHE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps" CACHE PATH "Dependency cache directory")
file(MAKE_DIRECTORY ${DEPS_CACHE_DIR})

# Dependencies
include(FetchContent)
find_package(PkgConfig REQUIRED)

# # Check dependency cache
# set(JSON_FOUND FALSE)
# find_package(nlohmann_json QUIET)
# if (nlohmann_json_FOUND)
#     message(STATUS "Found nlohmann/json as system library - ${nlohmann_json_INCLUDE_DIRS}")
#   set(JSON_FOUND TRUE)
# endif()

# if (NOT JSON_FOUND)
#   if(EXISTS "${DEPS_CACHE_DIR}/json")
#     message(STATUS "Using cached nlohmann/json")
#     add_subdirectory("${DEPS_CACHE_DIR}/json" "${CMAKE_BINARY_DIR}/json-build" EXCLUDE_FROM_ALL)
#     set(JSON_FOUND TRUE)
#   endif()
# endif()

# if(NOT JSON_FOUND)
#   message(STATUS "Fetching nlohmann/json")
#   FetchContent_Declare(
#     json
#     URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
#     SOURCE_DIR "${DEPS_CACHE_DIR}/json"
#   )
#   FetchContent_MakeAvailable(json)
# endif()

set(WEBVIEW_FOUND FALSE)
find_package(webview QUIET)
if (webview_FOUND)
  message(STATUS "Found webview as system library - ${WEBVIEW_INCLUDE_DIRS}")
  set(WEBVIEW_FOUND TRUE)
  add_library(webview::core ALIAS webview)
endif()

if(NOT WEBVIEW_FOUND)
  if(EXISTS "${DEPS_CACHE_DIR}/webview")
    message(STATUS "Using cached webview")
    add_subdirectory("${DEPS_CACHE_DIR}/webview" "${CMAKE_BINARY_DIR}/webview-build" EXCLUDE_FROM_ALL)
    if(NOT TARGET webview::core)
      add_library(webview::core ALIAS webview)
    endif()
    set(WEBVIEW_FOUND TRUE)
  endif()
endif()

if(NOT WEBVIEW_FOUND)
  message(STATUS "Fetching webview")
  FetchContent_Declare(
    webview
    GIT_REPOSITORY https://github.com/webview/webview.git
    GIT_TAG 0.12.0
    SOURCE_DIR "${DEPS_CACHE_DIR}/webview"
  )
  FetchContent_MakeAvailable(webview)
  if(NOT TARGET webview::core)
    add_library(webview::core ALIAS webview)
  endif()
endif()

# Check dependency cache
set(MINIZ_FOUND FALSE)
add_compile_definitions(MINIZ_NO_ZLIB_COMPATIBLE_NAMES)

if (EXISTS "${DEPS_CACHE_DIR}/miniz")
  message(STATUS "Using cached miniz")
  add_subdirectory("${DEPS_CACHE_DIR}/miniz" "${CMAKE_BINARY_DIR}/miniz-build" EXCLUDE_FROM_ALL)
  set(MINIZ_FOUND TRUE)
endif()

if (NOT MINIZ_FOUND)
  message(STATUS "Fetching miniz 3.0.2")
  FetchContent_Declare(
          miniz
          GIT_REPOSITORY https://github.com/richgel999/miniz.git
          GIT_TAG 3.0.2
          SOURCE_DIR "${DEPS_CACHE_DIR}/miniz"
  )
  FetchContent_MakeAvailable(miniz)
endif()

# Set up Google Test if BUILD_TESTS is enabled
if (BUILD_TESTS)
  set(GTEST_FOUND FALSE)
  find_package(GTest QUIET)
  if (GTest_FOUND)
    message(STATUS "Found Google Test as system library")
    set(GTEST_FOUND TRUE)
  endif()

  if (NOT GTEST_FOUND)
    if(EXISTS "${DEPS_CACHE_DIR}/googletest")
      message(STATUS "Using cached Google Test")
      add_subdirectory("${DEPS_CACHE_DIR}/googletest" "${CMAKE_BINARY_DIR}/googletest-build" EXCLUDE_FROM_ALL)
      set(GTEST_FOUND TRUE)
    else()
      message(STATUS "Fetching Google Test")
      FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.16.0
        SOURCE_DIR "${DEPS_CACHE_DIR}/googletest"
      )
      FetchContent_MakeAvailable(googletest)
    endif()
  endif()

  # Find libcurl for HTTP client in tests
  find_package(CURL REQUIRED)
  include_directories(${CURL_INCLUDE_DIRS})
endif()

# Core include directories
set(MEMBRANE_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/Json
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/vfs
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/HttpServer
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/FunctionRegistry
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/Membrane_lib
  ${CMAKE_CURRENT_SOURCE_DIR}/res
)

# Use object libraries for faster incremental builds
add_library(json INTERFACE)
target_include_directories(json INTERFACE ${MEMBRANE_INCLUDES})

add_library(vfs OBJECT lib/vfs/vfs.cpp)
target_include_directories(vfs PUBLIC ${MEMBRANE_INCLUDES})

add_library(httpserver OBJECT lib/HttpServer/HttpServer.cpp)
target_include_directories(httpserver PUBLIC ${MEMBRANE_INCLUDES})
target_link_libraries(httpserver PRIVATE vfs)

add_library(FunctionRegistry OBJECT lib/FunctionRegistry/FunctionRegistry.cpp)
target_include_directories(FunctionRegistry PUBLIC ${MEMBRANE_INCLUDES})

add_library(Membrane_lib OBJECT 
  lib/Membrane_lib/Membrane.cpp 
  lib/Membrane_lib/Membrane.setup.cpp
  lib/Membrane_lib/MembraneUtils.cpp
)
target_include_directories(Membrane_lib PUBLIC ${MEMBRANE_INCLUDES})
target_include_directories(Membrane_lib PRIVATE ${DEPS_CACHE_DIR}/miniz)
target_link_libraries(Membrane_lib PRIVATE vfs httpserver FunctionRegistry webview::core miniz json)

# Setup tests for each library component
if (BUILD_TESTS)
  # Create test directories if they don't exist
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/vfs/tests)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/HttpServer/tests)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/FunctionRegistry/tests)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/Membrane_lib/tests)

  # Helper function to set up test targets
  function(add_lib_test LIB_NAME LIB_PATH)
    # Check if test file exists
    set(TEST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_PATH}/tests/${LIB_NAME}Test.cpp")
    if(EXISTS "${TEST_FILE}")
      # Create test executable
      add_executable(${LIB_NAME}_test "${TEST_FILE}")
      target_include_directories(${LIB_NAME}_test PRIVATE ${MEMBRANE_INCLUDES})
      
      # Link with appropriate libraries
      target_link_libraries(${LIB_NAME}_test PRIVATE 
        ${LIB_NAME}
        gtest 
        gtest_main
        pthread
        ${CURL_LIBRARIES}
      )
      
      # Add as a test
      add_test(NAME ${LIB_NAME}_test COMMAND ${LIB_NAME}_test)
    endif()
  endfunction()

  # Enable testing
  enable_testing()
  
  # Add test targets for each component
  add_lib_test(vfs "vfs")
  add_lib_test(httpserver "HttpServer")
  add_lib_test(FunctionRegistry "FunctionRegistry")
  add_lib_test(Membrane_lib "Membrane_lib")
  
  # Top level test target that runs all tests
  add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS 
      vfs_test 
      httpserver_test 
      FunctionRegistry_test 
      Membrane_lib_test
    COMMENT "Running all tests"
  )
endif()

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
  if(EXISTS "${CMAKE_SOURCE_DIR}/src-react/dist/index.html" OR EXISTS "${CMAKE_SOURCE_DIR}/src-react/dist/dist.zip")
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/res/resource_init.cpp" OR
       "${CMAKE_SOURCE_DIR}/src-react/dist/index.html" IS_NEWER_THAN "${CMAKE_SOURCE_DIR}/res/resource_init.cpp" OR
         "${CMAKE_SOURCE_DIR}/src-react/dist/dist.zip" IS_NEWER_THAN "${CMAKE_SOURCE_DIR}/res/resource_init.cpp")
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
add_executable( ${BINARY_NAME}
  src/main.cpp
  ${RESOURCE_INIT_SOURCE}
  $<TARGET_OBJECTS:vfs>
  $<TARGET_OBJECTS:httpserver>
  $<TARGET_OBJECTS:FunctionRegistry>
  $<TARGET_OBJECTS:Membrane_lib>
)

target_compile_definitions(${BINARY_NAME} PRIVATE
  BIN_NAME_MEMBRANE="${BINARY_NAME}"
)
target_include_directories(${BINARY_NAME} PRIVATE ${MEMBRANE_INCLUDES})
target_link_libraries(${BINARY_NAME} PRIVATE
  webview::core
  miniz
)

# Install configuration
install(TARGETS ${BINARY_NAME}
  RUNTIME DESTINATION bin
)