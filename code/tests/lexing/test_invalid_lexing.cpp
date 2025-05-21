#include "../TestFramework.h"
#include "Compiler.h"
#include "Settings.h"

#include <fstream>
#include <vector>
#include <string>
#include <iostream>

TEST_CASE(Chapter1InvalidLex, "chapter_1", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_1/invalid_lex/at_sign.c",
        "tests/chapter_1/invalid_lex/backslash.c",
        "tests/chapter_1/invalid_lex/backtick.c",
        "tests/chapter_1/invalid_lex/invalid_identifier.c",
        "tests/chapter_1/invalid_lex/invalid_identifier_2.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            if (status == 0)
            {
                std::cerr << "Expected error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter6InvalidLexExtraCredit, "chapter_6", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_6/invalid_lex/extra_credit/bad_label.c"};
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            if (status == 0)
            {
                std::cerr << "Expected error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter11InvalidLex, "chapter_11", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_11/invalid_lex/invalid_suffix.c",
        "tests/chapter_11/invalid_lex/invalid_suffix2.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            if (status == 0)
            {
                std::cerr << "Expected error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 12
TEST_CASE(Chapter12InvalidLex, "chapter_12", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_12/invalid_lex/invalid_suffix.c",
        "tests/chapter_12/invalid_lex/invalid_suffix_2.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            if (status == 0)
            {
                std::cerr << "Expected error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 13
TEST_CASE(Chapter13InvalidLex, "chapter_13", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_13/invalid_lex/another_bad_constant.c",
        "tests/chapter_13/invalid_lex/bad_exponent_suffix.c",
        "tests/chapter_13/invalid_lex/malformed_const.c",
        "tests/chapter_13/invalid_lex/malformed_exponent.c",
        "tests/chapter_13/invalid_lex/missing_exponent.c",
        "tests/chapter_13/invalid_lex/missing_negative_exponent.c",
        "tests/chapter_13/invalid_lex/yet_another_bad_constant.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            if (status == 0)
            {
                std::cerr << "Expected error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// // Chapter 14
// TEST_CASE(Chapter14InvalidLex, "chapter_14", "--lex")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
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

// // Chapter 15
// TEST_CASE(Chapter15InvalidLex, "chapter_15", "--lex")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
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

// // Chapter 16
// TEST_CASE(Chapter16InvalidLex, "chapter_12", "--lex")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
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

// // Chapter 17
// TEST_CASE(Chapter17InvalidLex, "chapter_17", "--lex")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
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

// // Chapter 18
// TEST_CASE(Chapter18InvalidLex, "chapter_18", "--lex")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
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

// // Chapter 19
// TEST_CASE(Chapter19InvalidLex, "chapter_19", "--lex")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
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
//         Compiler compiler;
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