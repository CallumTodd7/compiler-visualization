# External dependancies
find_package(Magnum REQUIRED
    GL
    GlfwApplication
    Text
    MeshTools
    Primitives
    SceneGraph
    Shaders)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

## Main
add_executable(cv
    main.cpp
    Lexer.cpp Lexer.h
    StreamOverloads.cpp
    ThreadSync.cpp ThreadSync.h
    Parser.cpp Parser.h
    AST.h Types.h visuals/Application.cpp visuals/Application.h)

## Link
target_link_libraries(cv PRIVATE
    glfw
    Magnum::Magnum
    Magnum::Application
    Magnum::GL
    Magnum::Text
    Magnum::MeshTools
    Magnum::Primitives
    Magnum::SceneGraph
    Magnum::Shaders)
