### Main
cmake_minimum_required(VERSION 3.14)
project(compiler_visualization)

set(CMAKE_CXX_STANDARD 17)

### Main application
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(src)
