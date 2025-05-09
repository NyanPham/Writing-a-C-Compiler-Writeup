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