#include "../TestFramework.h"
#include "Compiler.h"
#include "Settings.h"

#include <fstream>
#include <vector>
#include <string>
#include <iostream>

TEST_CASE(Chapter5InvalidSemantic, "chapter_5", "--validate")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_5/invalid_semantics/declared_after_use.c",
        "tests/chapter_5/invalid_semantics/invalid_lvalue.c",
        "tests/chapter_5/invalid_semantics/invalid_lvalue_2.c",
        "tests/chapter_5/invalid_semantics/mixed_precedence_assignment.c",
        "tests/chapter_5/invalid_semantics/redefine.c",
        "tests/chapter_5/invalid_semantics/undeclared_var.c",
        "tests/chapter_5/invalid_semantics/undeclared_var_and.c",
        "tests/chapter_5/invalid_semantics/undeclared_var_compare.c",
        "tests/chapter_5/invalid_semantics/undeclared_var_unary.c",
        "tests/chapter_5/invalid_semantics/use_then_redefine.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, srcFile);
            // Check that the compilation succeeded
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter5InvalidSemanticExtraCredit, "chapter_5", "--validate")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_5/invalid_semantics/extra_credit/compound_invalid_lvalue.c",
        "tests/chapter_5/invalid_semantics/extra_credit/compound_invalid_lvalue_2.c",
        "tests/chapter_5/invalid_semantics/extra_credit/postfix_decr_non_lvalue.c",
        "tests/chapter_5/invalid_semantics/extra_credit/postfix_incr_non_lvalue.c",
        "tests/chapter_5/invalid_semantics/extra_credit/prefix_decr_non_lvalue.c",
        "tests/chapter_5/invalid_semantics/extra_credit/prefix_incr_non_lvalue.c",
        "tests/chapter_5/invalid_semantics/extra_credit/undeclared_bitwise_op.c",
        "tests/chapter_5/invalid_semantics/extra_credit/undeclared_compound_assignment.c",
        "tests/chapter_5/invalid_semantics/extra_credit/undeclared_compound_assignment_use.c",
        "tests/chapter_5/invalid_semantics/extra_credit/undeclared_postfix_decr.c",
        "tests/chapter_5/invalid_semantics/extra_credit/undeclared_prefix_incr.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, srcFile);
            // Check that the compilation succeeded
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter6InvalidSemantic, "chapter_6", "--validate")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_6/invalid_semantics/invalid_var_in_if.c",
        "tests/chapter_6/invalid_semantics/ternary_assign.c",
        "tests/chapter_6/invalid_semantics/undeclared_var_in_ternary.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, srcFile);
            // Check that the compilation succeeded
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter6InvalidSemanticExtraCredit, "chapter_6", "--validate")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_6/invalid_semantics/extra_credit/duplicate_labels.c",
        "tests/chapter_6/invalid_semantics/extra_credit/goto_missing_label.c",
        "tests/chapter_6/invalid_semantics/extra_credit/goto_variable.c",
        "tests/chapter_6/invalid_semantics/extra_credit/undeclared_var_in_labeled_statement.c",
        "tests/chapter_6/invalid_semantics/extra_credit/use_label_as_variable.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, srcFile);
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter7InvalidSemantic, "chapter_7", "--validate")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_7/invalid_semantics/double_define.c",
        "tests/chapter_7/invalid_semantics/double_define_after_scope.c",
        "tests/chapter_7/invalid_semantics/out_of_scope.c",
        "tests/chapter_7/invalid_semantics/use_before_declare.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, srcFile);
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter7InvalidSemanticExtraCredit, "chapter_7", "--validate")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_7/invalid_semantics/extra_credit/different_labels_same_scope.c",
        "tests/chapter_7/invalid_semantics/extra_credit/duplicate_labels_different_scopes.c",
        "tests/chapter_7/invalid_semantics/extra_credit/goto_use_before_declare.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, srcFile);
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter8InvalidSemantic, "chapter_8", "--validate")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_8/invalid_semantics/break_not_in_loop.c",
        "tests/chapter_8/invalid_semantics/continue_not_in_loop.c",
        "tests/chapter_8/invalid_semantics/out_of_scope_do_loop.c",
        "tests/chapter_8/invalid_semantics/out_of_scope_loop_variable.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, srcFile);
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter8InvalidSemanticExtraCredit, "chapter_8", "--validate")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_8/invalid_semantics/extra_credit/case_continue.c",
        "tests/chapter_8/invalid_semantics/extra_credit/case_outside_switch.c",
        "tests/chapter_8/invalid_semantics/extra_credit/default_continue.c",
        "tests/chapter_8/invalid_semantics/extra_credit/default_outside_switch.c",
        "tests/chapter_8/invalid_semantics/extra_credit/different_cases_same_scope.c",
        "tests/chapter_8/invalid_semantics/extra_credit/duplicate_case.c",
        "tests/chapter_8/invalid_semantics/extra_credit/duplicate_case_in_labeled_switch.c",
        "tests/chapter_8/invalid_semantics/extra_credit/duplicate_case_in_nested_statement.c",
        "tests/chapter_8/invalid_semantics/extra_credit/duplicate_default.c",
        "tests/chapter_8/invalid_semantics/extra_credit/duplicate_default_in_nested_statement.c",
        "tests/chapter_8/invalid_semantics/extra_credit/duplicate_label_in_default.c",
        "tests/chapter_8/invalid_semantics/extra_credit/duplicate_label_in_loop.c",
        "tests/chapter_8/invalid_semantics/extra_credit/duplicate_variable_in_switch.c",
        "tests/chapter_8/invalid_semantics/extra_credit/labeled_break_outside_loop.c",
        "tests/chapter_8/invalid_semantics/extra_credit/non_constant_case.c",
        "tests/chapter_8/invalid_semantics/extra_credit/switch_continue.c",
        "tests/chapter_8/invalid_semantics/extra_credit/undeclared_variable_in_case.c",
        "tests/chapter_8/invalid_semantics/extra_credit/undeclared_variable_in_default.c",
        "tests/chapter_8/invalid_semantics/extra_credit/undeclared_var_switch_expression.c",
        "tests/chapter_8/invalid_semantics/extra_credit/undefined_label_in_case.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, srcFile);
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}
