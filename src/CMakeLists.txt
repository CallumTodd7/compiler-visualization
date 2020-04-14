add_executable(cv
    main.cpp
    ThreadSync.cpp ThreadSync.h
    Data.h Data.cpp
    compiler/Lexer.cpp compiler/Lexer.h
    compiler/StreamOverloads.cpp
    compiler/Parser.cpp compiler/Parser.h
    compiler/Types.h compiler/Types.cpp
    compiler/AST.h
    compiler/Generator.cpp compiler/Generator.h
    visuals/VisualMain.cpp visuals/VisualMain.h
    visuals/love2dShaders.h visuals/love2dHelper.h)
