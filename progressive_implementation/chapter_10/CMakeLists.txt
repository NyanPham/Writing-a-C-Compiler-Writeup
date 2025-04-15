cmake_minimum_required(VERSION 3.10)
project(Compiler)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files for the compiler
set(COMPILER_SOURCES
    src/main.cpp
    src/Settings.cpp
    src/Compiler.cpp
    src/UniqueIds.cpp
    src/Token.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/semantic_analysis/IdentifierResolution.cpp
    src/semantic_analysis/ValidateLabels.cpp
    src/semantic_analysis/LoopLabeling.cpp
    src/semantic_analysis/CollectSwitchCases.cpp
    src/semantic_analysis/TypeChecker.cpp
    src/TackyGen.cpp
    src/backend/CodeGen.cpp
    src/backend/ReplacePseudos.cpp
    src/backend/InstructionFixup.cpp
    src/Emit.cpp
)

# Add source files for the tests
set(TEST_SOURCES
    tests/test_compiler.cpp
    tests/lexing/test_valid_lexing.cpp
    tests/lexing/test_invalid_lexing.cpp
    tests/parsing/test_valid_parsing.cpp
    tests/parsing/test_invalid_parsing.cpp
    tests/semantic_analysis/test_valid_semantic_analysis.cpp
    tests/semantic_analysis/test_invalid_semantic_analysis.cpp
    tests/tackygen/test_valid_tackygen.cpp
    tests/codegen/test_valid_codegen.cpp
)

# Include directories
include_directories(
    src
    src/backend
    src/semantic_analysis
    tests
)

# Add the compiler executable
add_executable(compiler ${COMPILER_SOURCES})

# Add the test executable
add_executable(test_compiler ${TEST_SOURCES} ${COMPILER_SOURCES})

# Set the output directories
set_target_properties(compiler PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(test_compiler PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)