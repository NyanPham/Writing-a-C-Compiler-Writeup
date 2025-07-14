#include "../TestFramework.h"
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>

// Use these globals from test_compiler.cpp
extern bool g_redirectOutput;
extern std::string g_redirectToFile;

// Helper to run compiler.exe on a source file with --lex and expect failure
inline void lex_run_compiler_on_invalid(const std::string& srcFile) {
    std::string cmd = "..\\bin\\compiler.exe " + srcFile + " --lex";
    if (!g_redirectToFile.empty()) {
        cmd += " >" + g_redirectToFile + " 2>&1";
    } else if (g_redirectOutput) {
        cmd += " >nul 2>&1";
    }
    int result = std::system(cmd.c_str());
    if (result == 0) {
        std::cerr << "Expected error compiling file " << srcFile << std::endl;
    }
    ASSERT_TRUE(result != 0);
}

// Example for one test case:
TEST_CASE(Chapter1InvalidLex, "chapter_1", "--lex")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_1/invalid_lex/at_sign.c",
        "../tests/chapter_1/invalid_lex/backslash.c",
        "../tests/chapter_1/invalid_lex/backtick.c",
        "../tests/chapter_1/invalid_lex/invalid_identifier.c",
        "../tests/chapter_1/invalid_lex/invalid_identifier_2.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        lex_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter6InvalidLexExtraCredit, "chapter_6", "--lex")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_6/invalid_lex/extra_credit/bad_label.c"};
    for (const auto &srcFile : srcFiles)
    {
        lex_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter11InvalidLex, "chapter_11", "--lex")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_11/invalid_lex/invalid_suffix.c",
        "../tests/chapter_11/invalid_lex/invalid_suffix2.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        lex_run_compiler_on_invalid(srcFile);
    }
}

// Chapter 12
TEST_CASE(Chapter12InvalidLex, "chapter_12", "--lex")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_12/invalid_lex/invalid_suffix.c",
        "../tests/chapter_12/invalid_lex/invalid_suffix_2.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        lex_run_compiler_on_invalid(srcFile);
    }
}

// Chapter 13
TEST_CASE(Chapter13InvalidLex, "chapter_13", "--lex")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_13/invalid_lex/another_bad_constant.c",
        "../tests/chapter_13/invalid_lex/bad_exponent_suffix.c",
        "../tests/chapter_13/invalid_lex/malformed_const.c",
        "../tests/chapter_13/invalid_lex/malformed_exponent.c",
        "../tests/chapter_13/invalid_lex/missing_exponent.c",
        "../tests/chapter_13/invalid_lex/missing_negative_exponent.c",
        "../tests/chapter_13/invalid_lex/yet_another_bad_constant.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        lex_run_compiler_on_invalid(srcFile);
    }
}

// Chapter 16
TEST_CASE(Chapter16InvalidLex, "chapter_12", "--lex")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_16/invalid_lex/char_bad_escape_sequence.c",
        "../tests/chapter_16/invalid_lex/newline.c",
        "../tests/chapter_16/invalid_lex/string_bad_escape_sequence.c",
        "../tests/chapter_16/invalid_lex/unescaped_backslash.c",
        "../tests/chapter_16/invalid_lex/unescaped_double_quote.c",
        "../tests/chapter_16/invalid_lex/unescaped_single_quote.c",
        "../tests/chapter_16/invalid_lex/unterminated_char_constant.c",
        "../tests/chapter_16/invalid_lex/unterminated_string.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        lex_run_compiler_on_invalid(srcFile);
    }
}

// Chapter 17 // NO TESTS FOR INVALID LEX
// TEST_CASE(Chapter17InvalidLex, "chapter_17", "--lex")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler(settings);
//         try
//         {
//             int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
//             if (status == 0)
//             {
//                 std::cerr << "Expected error compiling file " << srcFile << std::endl;
//             }
//             ASSERT_TRUE(status != 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// Chapter 18
TEST_CASE(Chapter18InvalidLex, "chapter_18", "--lex")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_18/invalid_lex/dot_bad_token.c",
        "../tests/chapter_18/invalid_lex/dot_bad_token_2.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        lex_run_compiler_on_invalid(srcFile);
    }
}

// // Chapter 19
// TEST_CASE(Chapter19InvalidLex, "chapter_19", "--lex")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler(settings);
//         try
//         {
//             int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
//             if (status == 0)
//             {
//                 std::cerr << "Expected error compiling file " << srcFile << std::endl;
//             }
//             ASSERT_TRUE(status != 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// // Chapter 20
// TEST_CASE(Chapter20InvalidLex, "chapter_20", "--lex")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler(settings);
//         try
//         {
//             int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
//             if (status == 0)
//             {
//                 std::cerr << "Expected error compiling file " << srcFile << std::endl;
//             }
//             ASSERT_TRUE(status != 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }