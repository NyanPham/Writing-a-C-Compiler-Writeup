cmake_minimum_required(VERSION 3.10)
project(Compiler)

# Add your source files
add_executable(compiler src/main.cpp src/Settings.cpp src/Compiler.cpp src/Token.cpp src/Lexer.cpp src/Parser.cpp src/CodeGen.cpp src/Emit.cpp)

# Add test files
add_executable(test_compiler 
    tests/lexing/test_valid_lexing.cpp 
    tests/lexing/test_invalid_lexing.cpp 
    tests/parsing/test_valid_parsing.cpp 
    tests/parsing/test_invalid_parsing.cpp 
    src/Settings.cpp 
    src/Compiler.cpp 
    src/Token.cpp 
    src/Lexer.cpp 
    src/Parser.cpp 
    src/CodeGen.cpp 
    src/Emit.cpp
)
target_include_directories(test_compiler PRIVATE tests src)