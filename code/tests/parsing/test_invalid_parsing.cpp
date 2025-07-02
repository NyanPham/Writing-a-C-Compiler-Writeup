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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter7InvalidParse, "chapter_7", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_7/invalid_parse/extra_brace.c",
        "tests/chapter_7/invalid_parse/missing_brace.c",
        "tests/chapter_7/invalid_parse/missing_semicolon.c",
        "tests/chapter_7/invalid_parse/ternary_blocks.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter8InvalidParse, "chapter_8", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_8/invalid_parse/decl_as_loop_body.c",
        "tests/chapter_8/invalid_parse/do_extra_semicolon.c",
        "tests/chapter_8/invalid_parse/do_missing_semicolon.c",
        "tests/chapter_8/invalid_parse/do_while_empty_parens.c",
        "tests/chapter_8/invalid_parse/extra_for_header_clause.c",
        "tests/chapter_8/invalid_parse/invalid_for_declaration.c",
        "tests/chapter_8/invalid_parse/missing_for_header_clause.c",
        "tests/chapter_8/invalid_parse/paren_mismatch.c",
        "tests/chapter_8/invalid_parse/statement_in_condition.c",
        "tests/chapter_8/invalid_parse/while_missing_paren.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter8InvalidParseExtraCredit, "chapter_8", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_8/invalid_parse/extra_credit/compound_assignment_invalid_decl.c",
        "tests/chapter_8/invalid_parse/extra_credit/label_in_loop_header.c",
        "tests/chapter_8/invalid_parse/extra_credit/label_is_not_block.c",
        "tests/chapter_8/invalid_parse/extra_credit/switch_case_declaration.c",
        "tests/chapter_8/invalid_parse/extra_credit/switch_goto_case.c",
        "tests/chapter_8/invalid_parse/extra_credit/switch_missing_case_value.c",
        "tests/chapter_8/invalid_parse/extra_credit/switch_missing_paren.c",
        "tests/chapter_8/invalid_parse/extra_credit/switch_no_condition.c",

    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter9InvalidParse, "chapter_9", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_9/invalid_parse/call_non_identifier.c",
        "tests/chapter_9/invalid_parse/decl_wrong_closing_delim.c",
        "tests/chapter_9/invalid_parse/funcall_wrong_closing_delim.c",
        "tests/chapter_9/invalid_parse/function_call_declaration.c",
        "tests/chapter_9/invalid_parse/function_returning_function.c",
        "tests/chapter_9/invalid_parse/fun_decl_for_loop.c",
        "tests/chapter_9/invalid_parse/initialize_function_as_variable.c",
        "tests/chapter_9/invalid_parse/trailing_comma.c",
        "tests/chapter_9/invalid_parse/trailing_comma_decl.c",
        "tests/chapter_9/invalid_parse/unclosed_paren_decl.c",
        "tests/chapter_9/invalid_parse/var_init_in_param_list.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter10InvalidParse, "chapter_10", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_10/invalid_parse/extern_param.c",
        "tests/chapter_10/invalid_parse/missing_parameter_list.c",
        "tests/chapter_10/invalid_parse/missing_type_specifier.c",
        "tests/chapter_10/invalid_parse/multi_storage_class_fun.c",
        "tests/chapter_10/invalid_parse/multi_storage_class_var.c",
        "tests/chapter_10/invalid_parse/static_and_extern.c",
        "tests/chapter_10/invalid_parse/static_param.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter10InvalidParseExtraCredit, "chapter_10", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_10/invalid_parse/extra_credit/extern_label.c",
        "tests/chapter_10/invalid_parse/extra_credit/file_scope_label.c",
        "tests/chapter_10/invalid_parse/extra_credit/static_label.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter11InvalidParse, "chapter_11", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_11/invalid_parse/bad_specifiers.c",
        "tests/chapter_11/invalid_parse/empty_cast.c",
        "tests/chapter_11/invalid_parse/fun_name_long.c",
        "tests/chapter_11/invalid_parse/invalid_cast.c",
        "tests/chapter_11/invalid_parse/invalid_suffix.c",
        "tests/chapter_11/invalid_parse/long_constant_as_var.c",
        "tests/chapter_11/invalid_parse/missing_cast_parentheses.c",
        "tests/chapter_11/invalid_parse/var_name_long.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
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
TEST_CASE(Chapter12InvalidParse, "chapter_12", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_12/invalid_parse/bad_specifiers.c",
        "tests/chapter_12/invalid_parse/bad_specifiers_2.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
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
TEST_CASE(Chapter13InvalidParse, "chapter_13", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_13/invalid_parse/invalid_type_specifier.c",
        "tests/chapter_13/invalid_parse/invalid_type_specifier_2.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 14
TEST_CASE(Chapter14InvalidParse, "chapter_14", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_14/invalid_parse/abstract_function_declarator.c",
        "tests/chapter_14/invalid_parse/cast_to_declarator.c",
        "tests/chapter_14/invalid_parse/malformed_abstract_declarator.c",
        "tests/chapter_14/invalid_parse/malformed_declarator.c",
        "tests/chapter_14/invalid_parse/malformed_function_declarator.c",
        "tests/chapter_14/invalid_parse/malformed_function_declarator_2.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 15
TEST_CASE(Chapter15InvalidParse, "chapter_15", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_15/invalid_parse/array_of_functions.c",
        "tests/chapter_15/invalid_parse/array_of_functions_2.c",
        "tests/chapter_15/invalid_parse/double_declarator.c",
        "tests/chapter_15/invalid_parse/empty_initializer_list.c",
        "tests/chapter_15/invalid_parse/malformed_abstract_array_declarator.c",
        "tests/chapter_15/invalid_parse/malformed_abstract_array_declarator_2.c",
        "tests/chapter_15/invalid_parse/malformed_array_declarator.c",
        "tests/chapter_15/invalid_parse/malformed_array_declarator_2.c",
        "tests/chapter_15/invalid_parse/malformed_array_declarator_3.c",
        "tests/chapter_15/invalid_parse/malformed_type_name.c",
        "tests/chapter_15/invalid_parse/malformed_type_name_2.c",
        "tests/chapter_15/invalid_parse/mismatched_subscript.c",
        "tests/chapter_15/invalid_parse/negative_array_dimension.c",
        "tests/chapter_15/invalid_parse/parenthesized_array_of_functions.c",
        "tests/chapter_15/invalid_parse/return_array.c",
        "tests/chapter_15/invalid_parse/unclosed_initializer.c",
        "tests/chapter_15/invalid_parse/unclosed_nested_initializer.c",
        "tests/chapter_15/invalid_parse/unclosed_subscript.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 16
TEST_CASE(Chapter16InvalidParse, "chapter_16", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_16/invalid_parse/invalid_type_specifier.c",
        "tests/chapter_16/invalid_parse/invalid_type_specifier_2.c",
        "tests/chapter_16/invalid_parse/misplaced_char_literal.c",
        "tests/chapter_16/invalid_parse/string_literal_varname.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 16
TEST_CASE(Chapter16InvalidParseExtraCredit, "chapter_16", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_16/invalid_parse/extra_credit/character_const_goto.c",
        "tests/chapter_16/invalid_parse/extra_credit/character_const_label.c",
        "tests/chapter_16/invalid_parse/extra_credit/string_literal_goto.c",
        "tests/chapter_16/invalid_parse/extra_credit/string_literal_label.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 17
TEST_CASE(Chapter17InvalidParse, "chapter_17", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_17/invalid_parse/bad_specifier.c",
        "tests/chapter_17/invalid_parse/bad_specifier_2.c",
        "tests/chapter_17/invalid_parse/sizeof_cast.c",
        "tests/chapter_17/invalid_parse/sizeof_type_no_parens.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 18
TEST_CASE(Chapter18InvalidParse, "chapter_18", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_18/invalid_parse/arrow_missing_member.c",
        "tests/chapter_18/invalid_parse/dot_invalid_member.c",
        "tests/chapter_18/invalid_parse/dot_no_left_expr.c",
        "tests/chapter_18/invalid_parse/dot_operator_in_declarator.c",
        "tests/chapter_18/invalid_parse/empty_initializer_list.c",
        "tests/chapter_18/invalid_parse/misplaced_storage_class.c",
        "tests/chapter_18/invalid_parse/struct_decl_double_semicolon.c",
        "tests/chapter_18/invalid_parse/struct_decl_empty_member_list.c",
        "tests/chapter_18/invalid_parse/struct_decl_extra_semicolon.c",
        "tests/chapter_18/invalid_parse/struct_decl_kw_wrong_order.c",
        "tests/chapter_18/invalid_parse/struct_decl_missing_end_semicolon.c",
        "tests/chapter_18/invalid_parse/struct_decl_tag_kw.c",
        "tests/chapter_18/invalid_parse/struct_decl_two_kws.c",
        "tests/chapter_18/invalid_parse/struct_member_initializer.c",
        "tests/chapter_18/invalid_parse/struct_member_is_function.c",
        "tests/chapter_18/invalid_parse/struct_member_name_kw.c",
        "tests/chapter_18/invalid_parse/struct_member_no_declarator.c",
        "tests/chapter_18/invalid_parse/struct_member_no_semicolon.c",
        "tests/chapter_18/invalid_parse/struct_member_no_type.c",
        "tests/chapter_18/invalid_parse/struct_member_storage_class.c",
        "tests/chapter_18/invalid_parse/var_decl_bad_tag_1.c",
        "tests/chapter_18/invalid_parse/var_decl_bad_tag_2.c",
        "tests/chapter_18/invalid_parse/var_decl_bad_type_specifier.c",
        "tests/chapter_18/invalid_parse/var_decl_missing_struct_kw.c",
        "tests/chapter_18/invalid_parse/var_decl_two_struct_kws.c",
        "tests/chapter_18/invalid_parse/var_decl_two_tags.c",
    };
    Settings settings;
    Compiler compiler;

#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < static_cast<int>(srcFiles.size()); ++i)
    {
        const std::string &srcFile = srcFiles[i];
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter18InvalidParseExtraCredit, "chapter_18", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_18/invalid_parse/extra_credit/case_struct_decl.c",
        "tests/chapter_18/invalid_parse/extra_credit/default_kw_member_name.c",
        "tests/chapter_18/invalid_parse/extra_credit/goto_kw_struct_tag.c",
        "tests/chapter_18/invalid_parse/extra_credit/labeled_struct_decl.c",
        "tests/chapter_18/invalid_parse/extra_credit/label_inside_struct_decl.c",
        "tests/chapter_18/invalid_parse/extra_credit/struct_union.c",
        "tests/chapter_18/invalid_parse/extra_credit/two_union_kws.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_bad_type_spec.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_decl_bad_type_specifier.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_decl_empty_member_list.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_decl_extra_semicolon.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_empty_initializer.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_member_initializer.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_member_is_function.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_member_name_kw.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_member_no_declarator.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_member_no_type.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_member_storage_class.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_struct_tag.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_two_tags.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_var_bad_tag.c",
        "tests/chapter_18/invalid_parse/extra_credit/union_var_tag_paren.c",
    };
    Settings settings;
    Compiler compiler;

#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < static_cast<int>(srcFiles.size()); ++i)
    {
        const std::string &srcFile = srcFiles[i];
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// // Chapter 19
// TEST_CASE(Chapter19InvalidParse, "chapter_19", "--parse")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
//         try
//         {
//             int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
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
// TEST_CASE(Chapter20InvalidParse, "chapter_20", "--parse")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
//         try
//         {
//             int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
//             ASSERT_TRUE(status != 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }