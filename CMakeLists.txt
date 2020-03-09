# General
cmake_minimum_required(VERSION 3.15)
project(compiler_visualization)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# External dependancies
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})

## GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw-3.3)

## Corrade
set(WITH_TESTSUITE OFF CACHE BOOL "" FORCE)
add_subdirectory(external/corrade EXCLUDE_FROM_ALL)

## Magnum
set(WITH_GLFWAPPLICATION ON CACHE BOOL "" FORCE)
set(WITH_GL ON CACHE BOOL "" FORCE)
add_subdirectory(external/magnum EXCLUDE_FROM_ALL)

# Main
add_subdirectory(src)
