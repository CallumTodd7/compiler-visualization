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
    visuals/love2dShaders.h
    visuals/love2dHelper.cpp visuals/love2dHelper.h
    visuals/HighlightableText.cpp visuals/HighlightableText.h
    visuals/Checklist.cpp visuals/Checklist.h
    visuals/Tween.cpp visuals/Tween.h
    visuals/TokenStream.cpp visuals/TokenStream.h
    visuals/ScrollManager.h visuals/Tree.cpp visuals/Tree.h)
