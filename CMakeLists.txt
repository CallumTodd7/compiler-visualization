### Main
cmake_minimum_required(VERSION 3.14)
project(compiler_visualization)

set(CMAKE_CXX_STANDARD 17)

### Main application
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(src)

### Love2d Graphics

set(MEGA TRUE)

set(MEGA_FREETYPE_VER "2.8.1")
set(MEGA_SDL2_VER "2.0.10")

add_subdirectory("external/freetype-${MEGA_FREETYPE_VER}" ${CMAKE_BINARY_DIR}/freetype)
set(MEGA_FREETYPE freetype)

add_subdirectory("external/SDL2" ${CMAKE_BINARY_DIR}/SDL2)
set(MEGA_SDL2 SDL2)
set(MEGA_SDL2MAIN SDL2main)

set(MEGA_3P
  ${MEGA_FREETYPE}
  ${MEGA_SDL2}
  ${MEGA_SDL2MAIN}
  )

link_directories(${SDL_LINK_DIR})

target_include_directories(cv PRIVATE external/love-11.3/src)
target_include_directories(cv PRIVATE external/love-11.3/src/modules)
add_subdirectory(external/love-11.3)
target_link_libraries(cv liblove)
