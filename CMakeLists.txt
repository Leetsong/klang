set(LLVM_LINK_COMPONENTS
    BitWriter
    Core
    ExecutionEngine
    MC
    Support
    nativecodegen
    )

add_llvm_example(klang
    main.cpp
    ast.cpp
    lexer.cpp
    parser.cpp
    codegen.cpp)
