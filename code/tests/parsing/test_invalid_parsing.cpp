#include "../TestFramework.h"
#include "Compiler.h"
#include "Settings.h"

#include <fstream>
#include <vector>
#include <string>
#include <iostream>

TEST_CASE(Chapter1InvalidParse, "chapter_1", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_1/invalid_parse/end_before_expr.c",
        "tests/chapter_1/invalid_parse/extra_junk.c",
        "tests/chapter_1/invalid_parse/invalid_function_name.c",
        "tests/chapter_1/invalid_parse/keyword_wrong_case.c",
        "tests/chapter_1/invalid_parse/missing_type.c",
        "tests/chapter_1/invalid_parse/misspelled_keyword.c",
        "tests/chapter_1/invalid_parse/no_semicolon.c",
        "tests/chapter_1/invalid_parse/not_expression.c",
        "tests/chapter_1/invalid_parse/space_in_keyword.c",
        "tests/chapter_1/invalid_parse/switched_parens.c",
        "tests/chapter_1/invalid_parse/unclosed_brace.c",
        "tests/chapter_1/invalid_parse/unclosed_paren.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, srcFile);
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

TEST_CASE(Chapter2InvalidParse, "chapter_2", "--parse")
{
    std::vector<std::string> srcFiles = {
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
            int status = compiler.compile(Stage::Parsing, srcFile);
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

TEST_CASE(Chapter3InvalidParse, "chapter_3", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_3/invalid_parse/double_operation.c",
        "tests/chapter_3/invalid_parse/imbalanced_paren.c",
        "tests/chapter_3/invalid_parse/malformed_paren.c",
        "tests/chapter_3/invalid_parse/misplaced_semicolon.c",
        "tests/chapter_3/invalid_parse/missing_first_op.c",
        "tests/chapter_3/invalid_parse/missing_open_paren.c",
        "tests/chapter_3/invalid_parse/missing_second_op.c",
        "tests/chapter_3/invalid_parse/no_semicolon.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, srcFile);
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter3InvalidParseExtraCredit, "chapter_3", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_3/invalid_parse/extra_credit/bitwise_double_operator.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, srcFile);
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter4InvalidParse, "chapter_4", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_4/invalid_parse/missing_const.c",
        "tests/chapter_4/invalid_parse/missing_first_op.c",
        "tests/chapter_4/invalid_parse/missing_operand.c",
        "tests/chapter_4/invalid_parse/missing_second_op.c",
        "tests/chapter_4/invalid_parse/missing_semicolon.c",
        "tests/chapter_4/invalid_parse/unary_missing_semicolon.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, srcFile);
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}
