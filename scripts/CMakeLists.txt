# CMakeLists.txt in project root
cmake_minimum_required(VERSION 3.30)
project(Membrane)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Detect platform and include appropriate platform file
if(WIN32)
    message(STATUS "Windows platform detected")
    include(scripts/platforms/windows/windows.cmake)
elseif(APPLE)
    message(STATUS "Apple platform detected")
    include(scripts/platforms/darwin/darwin.cmake)
elseif(UNIX AND NOT APPLE)
    message(STATUS "Linux platform detected")
    include(scripts/platforms/linux/linux.cmake)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()
