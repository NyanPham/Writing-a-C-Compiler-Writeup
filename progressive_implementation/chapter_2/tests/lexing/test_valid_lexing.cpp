#include "../TestFramework.h"
#include "Compiler.h"
#include "Settings.h"

#include <fstream>
#include <vector>
#include <string>
#include <iostream>

TEST_CASE(Chapter1ValidLex, "chapter_1", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_1/valid/multi_digit.c",
        "tests/chapter_1/valid/newlines.c",
        "tests/chapter_1/valid/no_newlines.c",
        "tests/chapter_1/valid/return_0.c",
        "tests/chapter_1/valid/return_2.c",
        "tests/chapter_1/valid/spaces.c",
        "tests/chapter_1/valid/tabs.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, srcFile);
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter2ValidLex, "chapter_2", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_2/valid/bitwise_int_min.c",
        "tests/chapter_2/valid/bitwise_zero.c",
        "tests/chapter_2/valid/bitwise.c",
        "tests/chapter_2/valid/neg_zero.c",
        "tests/chapter_2/valid/neg.c",
        "tests/chapter_2/valid/negate_int_max.c",
        "tests/chapter_2/valid/nested_ops_2.c",
        "tests/chapter_2/valid/nested_ops.c",
        "tests/chapter_2/valid/parens_2.c",
        "tests/chapter_2/valid/parens_3.c",
        "tests/chapter_2/valid/parens.c",
        "tests/chapter_2/valid/redundant_parens.c",

        "tests/chapter_2/invalid_parse/extra_paren.c",
        "tests/chapter_2/invalid_parse/missing_const.c",
        "tests/chapter_2/invalid_parse/missing_semicolon.c",
        "tests/chapter_2/invalid_parse/nested_missing_const.c",
        "tests/chapter_2/invalid_parse/parenthesize_operand.c",
        "tests/chapter_2/invalid_parse/unclosed_paren.c",
        "tests/chapter_2/invalid_parse/wrong_order.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, srcFile);
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}