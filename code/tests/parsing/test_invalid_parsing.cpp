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

TEST_CASE(Chapter5InvalidParse, "chapter_5", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_5/invalid_parse/compound_invalid_operator.c",
        "tests/chapter_5/invalid_parse/declare_keyword_as_var.c",
        "tests/chapter_5/invalid_parse/invalid_specifier.c",
        "tests/chapter_5/invalid_parse/invalid_type.c",
        "tests/chapter_5/invalid_parse/invalid_variable_name.c",
        "tests/chapter_5/invalid_parse/malformed_compound_assignment.c",
        "tests/chapter_5/invalid_parse/malformed_decrement.c",
        "tests/chapter_5/invalid_parse/malformed_increment.c",
        "tests/chapter_5/invalid_parse/malformed_less_equal.c",
        "tests/chapter_5/invalid_parse/malformed_not_equal.c",
        "tests/chapter_5/invalid_parse/missing_semicolon.c",
        "tests/chapter_5/invalid_parse/return_in_assignment.c",
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

TEST_CASE(Chapter5InvalidParseExtraCredit, "chapter_5", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_5/invalid_parse/extra_credit/binary_decrement.c",
        "tests/chapter_5/invalid_parse/extra_credit/binary_increment.c",
        "tests/chapter_5/invalid_parse/extra_credit/compound_initializer.c",
        "tests/chapter_5/invalid_parse/extra_credit/increment_declaration.c",
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

TEST_CASE(Chapter6InvalidParse, "chapter_6", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_6/invalid_parse/declaration_as_statement.c",
        "tests/chapter_6/invalid_parse/empty_if_body.c",
        "tests/chapter_6/invalid_parse/if_assignment.c",
        "tests/chapter_6/invalid_parse/if_no_parens.c",
        "tests/chapter_6/invalid_parse/incomplete_ternary.c",
        "tests/chapter_6/invalid_parse/malformed_ternary.c",
        "tests/chapter_6/invalid_parse/malformed_ternary_2.c",
        "tests/chapter_6/invalid_parse/mismatched_nesting.c",
        "tests/chapter_6/invalid_parse/wrong_ternary_delimiter.c",
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

TEST_CASE(Chapter6InvalidParseExtraCredit, "chapter_6", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_6/invalid_parse/extra_credit/goto_without_label.c",
        "tests/chapter_6/invalid_parse/extra_credit/kw_label.c",
        "tests/chapter_6/invalid_parse/extra_credit/label_declaration.c",
        "tests/chapter_6/invalid_parse/extra_credit/label_expression_clause.c",
        "tests/chapter_6/invalid_parse/extra_credit/label_outside_function.c",
        "tests/chapter_6/invalid_parse/extra_credit/label_without_statement.c",
        "tests/chapter_6/invalid_parse/extra_credit/parenthesized_label.c",
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
