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
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter3ValidLex, "chapter_3", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_3/valid/add.c",
        "tests/chapter_3/valid/associativity.c",
        "tests/chapter_3/valid/associativity_2.c",
        "tests/chapter_3/valid/associativity_3.c",
        "tests/chapter_3/valid/associativity_and_precedence.c",
        "tests/chapter_3/valid/div_neg.c",
        "tests/chapter_3/valid/div.c",
        "tests/chapter_3/valid/mod.c",
        "tests/chapter_3/valid/mult.c",
        "tests/chapter_3/valid/parens.c",
        "tests/chapter_3/valid/precedence.c",
        "tests/chapter_3/valid/sub_neg.c",
        "tests/chapter_3/valid/sub.c",
        "tests/chapter_3/valid/unop_add.c",
        "tests/chapter_3/valid/unop_parens.c",

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
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter3ValidLexExtraCredit, "chapter_3", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_3/valid/extra_credit/bitwise_and.c",
        "tests/chapter_3/valid/extra_credit/bitwise_or.c",
        "tests/chapter_3/valid/extra_credit/bitwise_precedence.c",
        "tests/chapter_3/valid/extra_credit/bitwise_shift_associativity.c",
        "tests/chapter_3/valid/extra_credit/bitwise_shift_associativity_2.c",
        "tests/chapter_3/valid/extra_credit/bitwise_shift_precedence.c",
        "tests/chapter_3/valid/extra_credit/bitwise_shiftl.c",
        "tests/chapter_3/valid/extra_credit/bitwise_shiftr.c",
        "tests/chapter_3/valid/extra_credit/bitwise_shiftr_negative.c",
        "tests/chapter_3/valid/extra_credit/bitwise_variable_shift_count.c",
        "tests/chapter_3/valid/extra_credit/bitwise_xor.c",

        "tests/chapter_3/invalid_parse/extra_credit/bitwise_double_operator.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter4ValidLex, "chapter_4", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_4/valid/and_false.c",
        "tests/chapter_4/valid/and_short_circuit.c",
        "tests/chapter_4/valid/and_true.c",
        "tests/chapter_4/valid/associativity.c",
        "tests/chapter_4/valid/compare_arithmetic_results.c",
        "tests/chapter_4/valid/eq_false.c",
        "tests/chapter_4/valid/eq_precedence.c",
        "tests/chapter_4/valid/eq_true.c",
        "tests/chapter_4/valid/ge_false.c",
        "tests/chapter_4/valid/ge_true.c",
        "tests/chapter_4/valid/gt_false.c",
        "tests/chapter_4/valid/gt_true.c",
        "tests/chapter_4/valid/le_false.c",
        "tests/chapter_4/valid/le_true.c",
        "tests/chapter_4/valid/lt_false.c",
        "tests/chapter_4/valid/lt_true.c",
        "tests/chapter_4/valid/multi_short_circuit.c",
        "tests/chapter_4/valid/ne_false.c",
        "tests/chapter_4/valid/ne_true.c",
        "tests/chapter_4/valid/nested_ops.c",
        "tests/chapter_4/valid/not_sum_2.c",
        "tests/chapter_4/valid/not_sum.c",
        "tests/chapter_4/valid/not_zero.c",
        "tests/chapter_4/valid/not.c",
        "tests/chapter_4/valid/operate_on_booleans.c",
        "tests/chapter_4/valid/or_false.c",
        "tests/chapter_4/valid/or_short_circuit.c",
        "tests/chapter_4/valid/or_true.c",
        "tests/chapter_4/valid/precedence_2.c",
        "tests/chapter_4/valid/precedence_3.c",
        "tests/chapter_4/valid/precedence_4.c",
        "tests/chapter_4/valid/precedence_5.c",
        "tests/chapter_4/valid/precedence.c",

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
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter4ValidLexExtraCredit, "chapter_4", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_4/valid/extra_credit/bitwise_and_precedence.c",
        "tests/chapter_4/valid/extra_credit/bitwise_or_precedence.c",
        "tests/chapter_4/valid/extra_credit/bitwise_shift_precedence.c",
        "tests/chapter_4/valid/extra_credit/bitwise_xor_precedence.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter5ValidLex, "chapter_5", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_5/valid/add_variables.c",
        "tests/chapter_5/valid/allocate_temps_and_vars.c",
        "tests/chapter_5/valid/assign.c",
        "tests/chapter_5/valid/assignment_in_initializer.c",
        "tests/chapter_5/valid/assignment_lowest_precedence.c",
        "tests/chapter_5/valid/assign_val_in_initializer.c",
        "tests/chapter_5/valid/empty_function_body.c",
        "tests/chapter_5/valid/exp_then_declaration.c",
        "tests/chapter_5/valid/local_var_missing_return.c",
        "tests/chapter_5/valid/mixed_precedence_assignment.c",
        "tests/chapter_5/valid/non_short_circuit_or.c",
        "tests/chapter_5/valid/null_statement.c",
        "tests/chapter_5/valid/null_then_return.c",
        "tests/chapter_5/valid/return_var.c",
        "tests/chapter_5/valid/short_circuit_and_fail.c",
        "tests/chapter_5/valid/short_circuit_or.c",
        "tests/chapter_5/valid/unused_exp.c",
        "tests/chapter_5/valid/use_assignment_result.c",
        "tests/chapter_5/valid/use_val_in_own_initializer.c",

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
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter5ValidLexExtraCredit, "chapter_5", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_5/valid/extra_credit/bitwise_in_initializer.c",
        "tests/chapter_5/valid/extra_credit/bitwise_ops_vars.c",
        "tests/chapter_5/valid/extra_credit/bitwise_shiftl_variable.c",
        "tests/chapter_5/valid/extra_credit/bitwise_shiftr_assign.c",
        "tests/chapter_5/valid/extra_credit/compound_assignment_chained.c",
        "tests/chapter_5/valid/extra_credit/compound_assignment_lowest_precedence.c",
        "tests/chapter_5/valid/extra_credit/compound_assignment_use_result.c",
        "tests/chapter_5/valid/extra_credit/compound_bitwise_and.c",
        "tests/chapter_5/valid/extra_credit/compound_bitwise_assignment_lowest_precedence.c",
        "tests/chapter_5/valid/extra_credit/compound_bitwise_chained.c",
        "tests/chapter_5/valid/extra_credit/compound_bitwise_or.c",
        "tests/chapter_5/valid/extra_credit/compound_bitwise_shiftl.c",
        "tests/chapter_5/valid/extra_credit/compound_bitwise_shiftr.c",
        "tests/chapter_5/valid/extra_credit/compound_bitwise_xor.c",
        "tests/chapter_5/valid/extra_credit/compound_divide.c",
        "tests/chapter_5/valid/extra_credit/compound_minus.c",
        "tests/chapter_5/valid/extra_credit/compound_mod.c",
        "tests/chapter_5/valid/extra_credit/compound_multiply.c",
        "tests/chapter_5/valid/extra_credit/compound_plus.c",
        "tests/chapter_5/valid/extra_credit/incr_expression_statement.c",
        "tests/chapter_5/valid/extra_credit/incr_in_binary_expr.c",
        "tests/chapter_5/valid/extra_credit/incr_parenthesized.c",
        "tests/chapter_5/valid/extra_credit/postfix_incr_and_decr.c",
        "tests/chapter_5/valid/extra_credit/postfix_precedence.c",
        "tests/chapter_5/valid/extra_credit/prefix_incr_and_decr.c",

        "tests/chapter_5/invalid_parse/extra_credit/binary_decrement.c",
        "tests/chapter_5/invalid_parse/extra_credit/binary_increment.c",
        "tests/chapter_5/invalid_parse/extra_credit/compound_initializer.c",
        "tests/chapter_5/invalid_parse/extra_credit/increment_declaration.c",

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
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter6ValidLex, "chapter_6", "--lex")
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

        "tests/chapter_6/invalid_semantics/invalid_var_in_if.c",
        "tests/chapter_6/invalid_semantics/ternary_assign.c",
        "tests/chapter_6/invalid_semantics/undeclared_var_in_ternary.c",

        "tests/chapter_6/valid/assign_ternary.c",
        "tests/chapter_6/valid/binary_condition.c",
        "tests/chapter_6/valid/binary_false_condition.c",
        "tests/chapter_6/valid/else.c",
        "tests/chapter_6/valid/if_nested.c",
        "tests/chapter_6/valid/if_nested_2.c",
        "tests/chapter_6/valid/if_nested_3.c",
        "tests/chapter_6/valid/if_nested_4.c",
        "tests/chapter_6/valid/if_nested_5.c",
        "tests/chapter_6/valid/if_not_taken.c",
        "tests/chapter_6/valid/if_null_body.c",
        "tests/chapter_6/valid/if_taken.c",
        "tests/chapter_6/valid/lh_assignment.c",
        "tests/chapter_6/valid/multiple_if.c",
        "tests/chapter_6/valid/nested_ternary.c",
        "tests/chapter_6/valid/nested_ternary_2.c",
        "tests/chapter_6/valid/rh_assignment.c",
        "tests/chapter_6/valid/ternary.c",
        "tests/chapter_6/valid/ternary_middle_assignment.c",
        "tests/chapter_6/valid/ternary_middle_binop.c",
        "tests/chapter_6/valid/ternary_precedence.c",
        "tests/chapter_6/valid/ternary_rh_binop.c",
        "tests/chapter_6/valid/ternary_short_circuit.c",
        "tests/chapter_6/valid/ternary_short_circuit_2.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter6ValidLexExtraCredit, "chapter_6", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_6/invalid_parse/extra_credit/goto_without_label.c",
        "tests/chapter_6/invalid_parse/extra_credit/kw_label.c",
        "tests/chapter_6/invalid_parse/extra_credit/label_declaration.c",
        "tests/chapter_6/invalid_parse/extra_credit/label_expression_clause.c",
        "tests/chapter_6/invalid_parse/extra_credit/label_outside_function.c",
        "tests/chapter_6/invalid_parse/extra_credit/label_without_statement.c",
        "tests/chapter_6/invalid_parse/extra_credit/parenthesized_label.c",

        "tests/chapter_6/invalid_semantics/extra_credit/duplicate_labels.c",
        "tests/chapter_6/invalid_semantics/extra_credit/goto_missing_label.c",
        "tests/chapter_6/invalid_semantics/extra_credit/goto_variable.c",
        "tests/chapter_6/invalid_semantics/extra_credit/undeclared_var_in_labeled_statement.c",
        "tests/chapter_6/invalid_semantics/extra_credit/use_label_as_variable.c",

        "tests/chapter_6/valid/extra_credit/bitwise_ternary.c",
        "tests/chapter_6/valid/extra_credit/compound_assign_ternary.c",
        "tests/chapter_6/valid/extra_credit/compound_if_expression.c",
        "tests/chapter_6/valid/extra_credit/goto_after_declaration.c",
        "tests/chapter_6/valid/extra_credit/goto_backwards.c",
        "tests/chapter_6/valid/extra_credit/goto_label.c",
        "tests/chapter_6/valid/extra_credit/goto_label_and_var.c",
        "tests/chapter_6/valid/extra_credit/goto_label_main.c",
        "tests/chapter_6/valid/extra_credit/goto_label_main_2.c",
        "tests/chapter_6/valid/extra_credit/goto_nested_label.c",
        "tests/chapter_6/valid/extra_credit/label_all_statements.c",
        "tests/chapter_6/valid/extra_credit/label_token.c",
        "tests/chapter_6/valid/extra_credit/lh_compound_assignment.c",
        "tests/chapter_6/valid/extra_credit/postfix_if.c",
        "tests/chapter_6/valid/extra_credit/postfix_in_ternary.c",
        "tests/chapter_6/valid/extra_credit/prefix_if.c",
        "tests/chapter_6/valid/extra_credit/prefix_in_ternary.c",
        "tests/chapter_6/valid/extra_credit/unused_label.c",
        "tests/chapter_6/valid/extra_credit/whitespace_after_label.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter7ValidLex, "chapter_7", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_7/valid/assign_to_self.c",
        "tests/chapter_7/valid/assign_to_self_2.c",
        "tests/chapter_7/valid/declaration_only.c",
        "tests/chapter_7/valid/empty_blocks.c",
        "tests/chapter_7/valid/hidden_then_visible.c",
        "tests/chapter_7/valid/hidden_variable.c",
        "tests/chapter_7/valid/inner_uninitialized.c",
        "tests/chapter_7/valid/multiple_vars_same_name.c",
        "tests/chapter_7/valid/nested_if.c",
        "tests/chapter_7/valid/similar_var_names.c",
        "tests/chapter_7/valid/use_in_inner_scope.c",

        "tests/chapter_7/invalid_parse/extra_brace.c",
        "tests/chapter_7/invalid_parse/missing_brace.c",
        "tests/chapter_7/invalid_parse/missing_semicolon.c",
        "tests/chapter_7/invalid_parse/ternary_blocks.c",

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
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter7ValidLexExtraCredit, "chapter_7", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_7/valid/extra_credit/compound_subtract_in_block.c",
        "tests/chapter_7/valid/extra_credit/goto_before_declaration.c",
        "tests/chapter_7/valid/extra_credit/goto_inner_scope.c",
        "tests/chapter_7/valid/extra_credit/goto_outer_scope.c",
        "tests/chapter_7/valid/extra_credit/goto_sibling_scope.c",

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
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter8ValidLex, "chapter_8", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_8/valid/break.c",
        "tests/chapter_8/valid/break_immediate.c",
        "tests/chapter_8/valid/continue.c",
        "tests/chapter_8/valid/continue_empty_post.c",
        "tests/chapter_8/valid/do_while.c",
        "tests/chapter_8/valid/do_while_break_immediate.c",
        "tests/chapter_8/valid/empty_expression.c",
        "tests/chapter_8/valid/empty_loop_body.c",
        "tests/chapter_8/valid/for.c",
        "tests/chapter_8/valid/for_absent_condition.c",
        "tests/chapter_8/valid/for_absent_post.c",
        "tests/chapter_8/valid/for_decl.c",
        "tests/chapter_8/valid/for_nested_shadow.c",
        "tests/chapter_8/valid/for_shadow.c",
        "tests/chapter_8/valid/multi_break.c",
        "tests/chapter_8/valid/multi_continue_same_loop.c",
        "tests/chapter_8/valid/nested_break.c",
        "tests/chapter_8/valid/nested_continue.c",
        "tests/chapter_8/valid/nested_loop.c",
        "tests/chapter_8/valid/null_for_header.c",
        "tests/chapter_8/valid/while.c",

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
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter8ValidLexExtraCredit, "chapter_8", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_8/valid/extra_credit/case_block.c",
        "tests/chapter_8/valid/extra_credit/compound_assignment_controlling_expression.c",
        "tests/chapter_8/valid/extra_credit/compound_assignment_for_loop.c",
        "tests/chapter_8/valid/extra_credit/duffs_device.c",
        "tests/chapter_8/valid/extra_credit/goto_bypass_condition.c",
        "tests/chapter_8/valid/extra_credit/goto_bypass_init_exp.c",
        "tests/chapter_8/valid/extra_credit/goto_bypass_post_exp.c",
        "tests/chapter_8/valid/extra_credit/label_loops_breaks_and_continues.c",
        "tests/chapter_8/valid/extra_credit/label_loop_body.c",
        "tests/chapter_8/valid/extra_credit/loop_header_postfix_and_prefix.c",
        "tests/chapter_8/valid/extra_credit/loop_in_switch.c",
        "tests/chapter_8/valid/extra_credit/post_exp_incr.c",
        "tests/chapter_8/valid/extra_credit/switch.c",
        "tests/chapter_8/valid/extra_credit/switch_assign_in_condition.c",
        "tests/chapter_8/valid/extra_credit/switch_break.c",
        "tests/chapter_8/valid/extra_credit/switch_decl.c",
        "tests/chapter_8/valid/extra_credit/switch_default.c",
        "tests/chapter_8/valid/extra_credit/switch_default_fallthrough.c",
        "tests/chapter_8/valid/extra_credit/switch_default_not_last.c",
        "tests/chapter_8/valid/extra_credit/switch_default_only.c",
        "tests/chapter_8/valid/extra_credit/switch_empty.c",
        "tests/chapter_8/valid/extra_credit/switch_fallthrough.c",
        "tests/chapter_8/valid/extra_credit/switch_goto_mid_case.c",
        "tests/chapter_8/valid/extra_credit/switch_in_loop.c",
        "tests/chapter_8/valid/extra_credit/switch_nested_cases.c",
        "tests/chapter_8/valid/extra_credit/switch_nested_not_taken.c",
        "tests/chapter_8/valid/extra_credit/switch_nested_switch.c",
        "tests/chapter_8/valid/extra_credit/switch_not_taken.c",
        "tests/chapter_8/valid/extra_credit/switch_no_case.c",
        "tests/chapter_8/valid/extra_credit/switch_single_case.c",
        "tests/chapter_8/valid/extra_credit/switch_with_continue.c",
        "tests/chapter_8/valid/extra_credit/switch_with_continue_2.c",
        "tests/chapter_8/invalid_parse/extra_credit/compound_assignment_invalid_decl.c",
        "tests/chapter_8/invalid_parse/extra_credit/label_in_loop_header.c",
        "tests/chapter_8/invalid_parse/extra_credit/label_is_not_block.c",
        "tests/chapter_8/invalid_parse/extra_credit/switch_case_declaration.c",
        "tests/chapter_8/invalid_parse/extra_credit/switch_goto_case.c",
        "tests/chapter_8/invalid_parse/extra_credit/switch_missing_case_value.c",
        "tests/chapter_8/invalid_parse/extra_credit/switch_missing_paren.c",
        "tests/chapter_8/invalid_parse/extra_credit/switch_no_condition.c",
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
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter9ValidLex, "chapter_9", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_9/invalid_declarations/assign_to_fun_call.c",
        "tests/chapter_9/invalid_declarations/decl_params_with_same_name.c",
        "tests/chapter_9/invalid_declarations/nested_function_definition.c",
        "tests/chapter_9/invalid_declarations/params_with_same_name.c",
        "tests/chapter_9/invalid_declarations/redefine_fun_as_var.c",
        "tests/chapter_9/invalid_declarations/redefine_parameter.c",
        "tests/chapter_9/invalid_declarations/redefine_var_as_fun.c",
        "tests/chapter_9/invalid_declarations/undeclared_fun.c",
        "tests/chapter_9/invalid_declarations/wrong_parameter_names.c",

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

        "tests/chapter_9/invalid_types/assign_fun_to_variable.c",
        "tests/chapter_9/invalid_types/assign_value_to_function.c",
        "tests/chapter_9/invalid_types/call_variable_as_function.c",
        "tests/chapter_9/invalid_types/conflicting_function_declarations.c",
        "tests/chapter_9/invalid_types/conflicting_local_function_declaration.c",
        "tests/chapter_9/invalid_types/divide_by_function.c",
        "tests/chapter_9/invalid_types/multiple_function_definitions.c",
        "tests/chapter_9/invalid_types/multiple_function_definitions_2.c",
        "tests/chapter_9/invalid_types/too_few_args.c",
        "tests/chapter_9/invalid_types/too_many_args.c",

        "tests/chapter_9/valid/arguments_in_registers/dont_clobber_edx.c",
        "tests/chapter_9/valid/arguments_in_registers/expression_args.c",
        "tests/chapter_9/valid/arguments_in_registers/fibonacci.c",
        "tests/chapter_9/valid/arguments_in_registers/forward_decl_multi_arg.c",
        "tests/chapter_9/valid/arguments_in_registers/hello_world.c",
        "tests/chapter_9/valid/arguments_in_registers/parameters_are_preserved.c",
        "tests/chapter_9/valid/arguments_in_registers/parameter_shadows_function.c",
        "tests/chapter_9/valid/arguments_in_registers/parameter_shadows_own_function.c",
        "tests/chapter_9/valid/arguments_in_registers/param_shadows_local_var.c",
        "tests/chapter_9/valid/arguments_in_registers/single_arg.c",

        "tests/chapter_9/valid/libraries/addition.c",
        "tests/chapter_9/valid/libraries/addition_client.c",
        "tests/chapter_9/valid/libraries/many_args.c",
        "tests/chapter_9/valid/libraries/many_args_client.c",
        "tests/chapter_9/valid/libraries/system_call.c",
        "tests/chapter_9/valid/libraries/system_call_client.c",
        "tests/chapter_9/valid/libraries/no_function_calls/division.c",
        "tests/chapter_9/valid/libraries/no_function_calls/division_client.c",
        "tests/chapter_9/valid/libraries/no_function_calls/local_stack_variables.c",
        "tests/chapter_9/valid/libraries/no_function_calls/local_stack_variables_client.c",

        "tests/chapter_9/valid/no_arguments/forward_decl.c",
        "tests/chapter_9/valid/no_arguments/function_shadows_variable.c",
        "tests/chapter_9/valid/no_arguments/multiple_declarations.c",
        "tests/chapter_9/valid/no_arguments/no_return_value.c",
        "tests/chapter_9/valid/no_arguments/precedence.c",
        "tests/chapter_9/valid/no_arguments/use_function_in_expression.c",
        "tests/chapter_9/valid/no_arguments/variable_shadows_function.c",

        "tests/chapter_9/valid/stack_arguments/call_putchar.c",
        "tests/chapter_9/valid/stack_arguments/lots_of_arguments.c",
        "tests/chapter_9/valid/stack_arguments/stack_alignment.c",
        //"tests/chapter_9/valid/stack_arguments/stack_alignment_check_linux.s",
        //"tests/chapter_9/valid/stack_arguments/stack_alignment_check_osx.s",
        "tests/chapter_9/valid/stack_arguments/test_for_memory_leaks.c",

    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter9ValidLexExtraCredit, "chapter_9", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_9/invalid_declarations/extra_credit/call_label_as_function.c",
        "tests/chapter_9/invalid_declarations/extra_credit/compound_assign_to_fun_call.c",
        "tests/chapter_9/invalid_declarations/extra_credit/decrement_fun_call.c",
        "tests/chapter_9/invalid_declarations/extra_credit/increment_fun_call.c",

        "tests/chapter_9/invalid_labels/extra_credit/goto_cross_function.c",
        "tests/chapter_9/invalid_labels/extra_credit/goto_function.c",

        "tests/chapter_9/invalid_types/extra_credit/bitwise_op_function.c",
        "tests/chapter_9/invalid_types/extra_credit/compound_assign_function_lhs.c",
        "tests/chapter_9/invalid_types/extra_credit/compound_assign_function_rhs.c",
        "tests/chapter_9/invalid_types/extra_credit/postfix_incr_fun_name.c",
        "tests/chapter_9/invalid_types/extra_credit/prefix_decr_fun_name.c",
        "tests/chapter_9/invalid_types/extra_credit/switch_on_function.c",

        "tests/chapter_9/valid/extra_credit/compound_assign_function_result.c",
        "tests/chapter_9/valid/extra_credit/dont_clobber_ecx.c",
        "tests/chapter_9/valid/extra_credit/goto_label_multiple_functions.c",
        "tests/chapter_9/valid/extra_credit/goto_shared_name.c",
        "tests/chapter_9/valid/extra_credit/label_naming_scheme.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter10ValidLex, "chapter_10", "--lex")
{
    std::vector<std::string> srcFiles = {
        //"tests/chapter_10/valid/data_on_page_boundary_linux.s",
        //"tests/chapter_10/valid/data_on_page_boundary_osx.s",
        "tests/chapter_10/valid/distinct_local_and_extern.c",
        "tests/chapter_10/valid/extern_block_scope_variable.c",
        "tests/chapter_10/valid/multiple_static_file_scope_vars.c",
        "tests/chapter_10/valid/multiple_static_local.c",
        "tests/chapter_10/valid/push_arg_on_page_boundary.c",
        "tests/chapter_10/valid/shadow_static_local_var.c",
        "tests/chapter_10/valid/static_local_multiple_scopes.c",
        "tests/chapter_10/valid/static_local_uninitialized.c",
        "tests/chapter_10/valid/static_recursive_call.c",
        "tests/chapter_10/valid/static_then_extern.c",
        "tests/chapter_10/valid/static_variables_in_expressions.c",
        "tests/chapter_10/valid/tentative_definition.c",
        "tests/chapter_10/valid/type_before_storage_class.c",
        "tests/chapter_10/valid/libraries/external_linkage_function.c",
        "tests/chapter_10/valid/libraries/external_linkage_function_client.c",
        "tests/chapter_10/valid/libraries/external_tentative_var.c",
        "tests/chapter_10/valid/libraries/external_tentative_var_client.c",
        "tests/chapter_10/valid/libraries/external_variable.c",
        "tests/chapter_10/valid/libraries/external_variable_client.c",
        "tests/chapter_10/valid/libraries/external_var_scoping.c",
        "tests/chapter_10/valid/libraries/external_var_scoping_client.c",
        "tests/chapter_10/valid/libraries/internal_hides_external_linkage.c",
        "tests/chapter_10/valid/libraries/internal_hides_external_linkage_client.c",
        "tests/chapter_10/valid/libraries/internal_linkage_function.c",
        "tests/chapter_10/valid/libraries/internal_linkage_function_client.c",
        "tests/chapter_10/valid/libraries/internal_linkage_var.c",
        "tests/chapter_10/valid/libraries/internal_linkage_var_client.c",
        "tests/chapter_10/invalid_parse/extern_param.c",
        "tests/chapter_10/invalid_parse/missing_parameter_list.c",
        "tests/chapter_10/invalid_parse/missing_type_specifier.c",
        "tests/chapter_10/invalid_parse/multi_storage_class_fun.c",
        "tests/chapter_10/invalid_parse/multi_storage_class_var.c",
        "tests/chapter_10/invalid_parse/static_and_extern.c",
        "tests/chapter_10/invalid_parse/static_param.c",
        "tests/chapter_10/invalid_declarations/conflicting_local_declarations.c",
        "tests/chapter_10/invalid_declarations/extern_follows_local_var.c",
        "tests/chapter_10/invalid_declarations/extern_follows_static_local_var.c",
        "tests/chapter_10/invalid_declarations/local_var_follows_extern.c",
        "tests/chapter_10/invalid_declarations/out_of_scope_extern_var.c",
        "tests/chapter_10/invalid_declarations/redefine_param_as_identifier_with_linkage.c",
        "tests/chapter_10/invalid_declarations/undeclared_global_variable.c",
        "tests/chapter_10/invalid_types/conflicting_function_linkage.c",
        "tests/chapter_10/invalid_types/conflicting_function_linkage_2.c",
        "tests/chapter_10/invalid_types/conflicting_global_definitions.c",
        "tests/chapter_10/invalid_types/conflicting_variable_linkage.c",
        "tests/chapter_10/invalid_types/conflicting_variable_linkage_2.c",
        "tests/chapter_10/invalid_types/extern_for_loop_counter.c",
        "tests/chapter_10/invalid_types/extern_variable_initializer.c",
        "tests/chapter_10/invalid_types/non_constant_static_initializer.c",
        "tests/chapter_10/invalid_types/non_constant_static_local_initializer.c",
        "tests/chapter_10/invalid_types/redeclare_file_scope_var_as_fun.c",
        "tests/chapter_10/invalid_types/redeclare_fun_as_file_scope_var.c",
        "tests/chapter_10/invalid_types/redeclare_fun_as_var.c",
        "tests/chapter_10/invalid_types/static_block_scope_function_declaration.c",
        "tests/chapter_10/invalid_types/static_for_loop_counter.c",
        "tests/chapter_10/invalid_types/use_file_scope_variable_as_fun.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter10ValidLexExtraCredit, "chapter_10", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_10/valid/extra_credit/bitwise_ops_file_scope_vars.c",
        "tests/chapter_10/valid/extra_credit/compound_assignment_static_var.c",
        "tests/chapter_10/valid/extra_credit/goto_skip_static_initializer.c",
        "tests/chapter_10/valid/extra_credit/increment_global_vars.c",
        "tests/chapter_10/valid/extra_credit/label_file_scope_var_same_name.c",
        "tests/chapter_10/valid/extra_credit/label_static_var_same_name.c",
        "tests/chapter_10/valid/extra_credit/switch_on_extern.c",
        "tests/chapter_10/valid/extra_credit/switch_skip_extern_decl.c",
        "tests/chapter_10/valid/extra_credit/switch_skip_static_initializer.c",
        "tests/chapter_10/valid/extra_credit/libraries/same_label_same_fun.c",
        "tests/chapter_10/valid/extra_credit/libraries/same_label_same_fun_client.c",
        "tests/chapter_10/invalid_parse/extra_credit/extern_label.c",
        "tests/chapter_10/invalid_parse/extra_credit/file_scope_label.c",
        "tests/chapter_10/invalid_parse/extra_credit/static_label.c",
        "tests/chapter_10/invalid_labels/extra_credit/goto_global_var.c",
        "tests/chapter_10/invalid_types/extra_credit/static_var_case.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter11ValidLex, "chapter_11", "--lex")
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

        "tests/chapter_11/invalid_types/call_long_as_function.c",
        "tests/chapter_11/invalid_types/cast_lvalue.c",
        "tests/chapter_11/invalid_types/conflicting_function_types.c",
        "tests/chapter_11/invalid_types/conflicting_global_types.c",
        "tests/chapter_11/invalid_types/conflicting_variable_types.c",

        "tests/chapter_11/valid/explicit_casts/sign_extend.c",
        "tests/chapter_11/valid/explicit_casts/truncate.c",

        "tests/chapter_11/valid/implicit_casts/common_type.c",
        "tests/chapter_11/valid/implicit_casts/convert_by_assignment.c",
        "tests/chapter_11/valid/implicit_casts/convert_function_arguments.c",
        "tests/chapter_11/valid/implicit_casts/convert_static_initializer.c",
        "tests/chapter_11/valid/implicit_casts/long_constants.c",

        "tests/chapter_11/valid/libraries/long_args.c",
        "tests/chapter_11/valid/libraries/long_args_client.c",
        "tests/chapter_11/valid/libraries/long_global_var.c",
        "tests/chapter_11/valid/libraries/long_global_var_client.c",
        "tests/chapter_11/valid/libraries/maintain_stack_alignment.c",
        "tests/chapter_11/valid/libraries/maintain_stack_alignment_client.c",
        "tests/chapter_11/valid/libraries/return_long.c",
        "tests/chapter_11/valid/libraries/return_long_client.c",

        "tests/chapter_11/valid/long_expressions/arithmetic_ops.c",
        "tests/chapter_11/valid/long_expressions/assign.c",
        "tests/chapter_11/valid/long_expressions/comparisons.c",
        "tests/chapter_11/valid/long_expressions/large_constants.c",
        "tests/chapter_11/valid/long_expressions/logical.c",
        "tests/chapter_11/valid/long_expressions/long_and_int_locals.c",
        "tests/chapter_11/valid/long_expressions/long_args.c",
        "tests/chapter_11/valid/long_expressions/multi_op.c",
        "tests/chapter_11/valid/long_expressions/return_long.c",
        "tests/chapter_11/valid/long_expressions/rewrite_large_multiply_regression.c",
        "tests/chapter_11/valid/long_expressions/simple.c",
        "tests/chapter_11/valid/long_expressions/static_long.c",
        "tests/chapter_11/valid/long_expressions/type_specifiers.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter11ValidLexExtraCredit, "chapter_11", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_11/invalid_labels/extra_credit/bitshift_duplicate_cases.c",
        "tests/chapter_11/invalid_labels/extra_credit/switch_duplicate_cases.c",
        "tests/chapter_11/invalid_labels/extra_credit/switch_duplicate_cases_2.c",
        "tests/chapter_11/valid/extra_credit/bitshift.c",
        "tests/chapter_11/valid/extra_credit/bitwise_long_op.c",
        "tests/chapter_11/valid/extra_credit/compound_assign_to_int.c",
        "tests/chapter_11/valid/extra_credit/compound_assign_to_long.c",
        "tests/chapter_11/valid/extra_credit/compound_bitshift.c",
        "tests/chapter_11/valid/extra_credit/compound_bitwise.c",
        "tests/chapter_11/valid/extra_credit/increment_long.c",
        "tests/chapter_11/valid/extra_credit/switch_int.c",
        "tests/chapter_11/valid/extra_credit/switch_long.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 12
TEST_CASE(Chapter12ValidLex, "chapter_12", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_12/invalid_parse/bad_specifiers.c",
        "tests/chapter_12/invalid_parse/bad_specifiers_2.c",
        "tests/chapter_12/invalid_types/conflicting_signed_unsigned.c",
        "tests/chapter_12/invalid_types/conflicting_uint_ulong.c",
        "tests/chapter_12/valid/explicit_casts/chained_casts.c",
        "tests/chapter_12/valid/explicit_casts/extension.c",
        "tests/chapter_12/valid/explicit_casts/rewrite_movz_regression.c",
        "tests/chapter_12/valid/explicit_casts/round_trip_casts.c",
        "tests/chapter_12/valid/explicit_casts/same_size_conversion.c",
        "tests/chapter_12/valid/explicit_casts/truncate.c",
        "tests/chapter_12/valid/implicit_casts/common_type.c",
        "tests/chapter_12/valid/implicit_casts/convert_by_assignment.c",
        "tests/chapter_12/valid/implicit_casts/promote_constants.c",
        "tests/chapter_12/valid/implicit_casts/static_initializers.c",
        "tests/chapter_12/valid/libraries/unsigned_args.c",
        "tests/chapter_12/valid/libraries/unsigned_args_client.c",
        "tests/chapter_12/valid/libraries/unsigned_global_var.c",
        "tests/chapter_12/valid/libraries/unsigned_global_var_client.c",
        "tests/chapter_12/valid/type_specifiers/signed_type_specifiers.c",
        "tests/chapter_12/valid/type_specifiers/unsigned_type_specifiers.c",
        "tests/chapter_12/valid/unsigned_expressions/arithmetic_ops.c",
        "tests/chapter_12/valid/unsigned_expressions/arithmetic_wraparound.c",
        "tests/chapter_12/valid/unsigned_expressions/comparisons.c",
        "tests/chapter_12/valid/unsigned_expressions/locals.c",
        "tests/chapter_12/valid/unsigned_expressions/logical.c",
        "tests/chapter_12/valid/unsigned_expressions/simple.c",
        "tests/chapter_12/valid/unsigned_expressions/static_variables.c",

    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter12ValidLexExtraCredit, "chapter_12", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_12/invalid_labels/extra_credit/switch_duplicate_cases.c",

        "tests/chapter_12/valid/extra_credit/bitwise_unsigned_ops.c",
        "tests/chapter_12/valid/extra_credit/bitwise_unsigned_shift.c",
        "tests/chapter_12/valid/extra_credit/compound_assign_uint.c",
        "tests/chapter_12/valid/extra_credit/compound_bitshift.c",
        "tests/chapter_12/valid/extra_credit/compound_bitwise.c",
        "tests/chapter_12/valid/extra_credit/postfix_precedence.c",
        "tests/chapter_12/valid/extra_credit/switch_uint.c",
        "tests/chapter_12/valid/extra_credit/unsigned_incr_decr.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 13
TEST_CASE(Chapter13ValidLex, "chapter_13", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_13/invalid_parse/invalid_type_specifier.c",
        "tests/chapter_13/invalid_parse/invalid_type_specifier_2.c",

        "tests/chapter_13/invalid_types/complement_double.c",
        "tests/chapter_13/invalid_types/mod_double.c",
        "tests/chapter_13/invalid_types/mod_double_2.c",

        "tests/chapter_13/valid/constants/constant_doubles.c",
        "tests/chapter_13/valid/constants/round_constants.c",
        "tests/chapter_13/valid/explicit_casts/cvttsd2si_rewrite.c",
        "tests/chapter_13/valid/explicit_casts/double_to_signed.c",
        "tests/chapter_13/valid/explicit_casts/double_to_unsigned.c",
        "tests/chapter_13/valid/explicit_casts/rewrite_cvttsd2si_regression.c",
        "tests/chapter_13/valid/explicit_casts/signed_to_double.c",
        "tests/chapter_13/valid/explicit_casts/unsigned_to_double.c",

        "tests/chapter_13/valid/floating_expressions/arithmetic_ops.c",
        "tests/chapter_13/valid/floating_expressions/comparisons.c",
        "tests/chapter_13/valid/floating_expressions/logical.c",
        "tests/chapter_13/valid/floating_expressions/loop_controlling_expression.c",
        "tests/chapter_13/valid/floating_expressions/simple.c",
        "tests/chapter_13/valid/floating_expressions/static_initialized_double.c",
        "tests/chapter_13/valid/function_calls/double_and_int_parameters.c",
        "tests/chapter_13/valid/function_calls/double_and_int_params_recursive.c",
        "tests/chapter_13/valid/function_calls/double_parameters.c",
        "tests/chapter_13/valid/function_calls/push_xmm.c",
        "tests/chapter_13/valid/function_calls/return_double.c",
        "tests/chapter_13/valid/function_calls/standard_library_call.c",
        "tests/chapter_13/valid/function_calls/use_arg_after_fun_call.c",
        "tests/chapter_13/valid/implicit_casts/common_type.c",
        "tests/chapter_13/valid/implicit_casts/complex_arithmetic_common_type.c",
        "tests/chapter_13/valid/implicit_casts/convert_for_assignment.c",
        "tests/chapter_13/valid/implicit_casts/static_initializers.c",
        "tests/chapter_13/valid/libraries/double_and_int_params_recursive.c",
        "tests/chapter_13/valid/libraries/double_and_int_params_recursive_client.c",
        "tests/chapter_13/valid/libraries/double_parameters.c",
        "tests/chapter_13/valid/libraries/double_parameters_client.c",
        "tests/chapter_13/valid/libraries/double_params_and_result.c",
        "tests/chapter_13/valid/libraries/double_params_and_result_client.c",
        "tests/chapter_13/valid/libraries/extern_double.c",
        "tests/chapter_13/valid/libraries/extern_double_client.c",
        "tests/chapter_13/valid/libraries/use_arg_after_fun_call.c",
        "tests/chapter_13/valid/libraries/use_arg_after_fun_call_client.c",
        "tests/chapter_13/valid/special_values/infinity.c",
        "tests/chapter_13/valid/special_values/negative_zero.c",
        "tests/chapter_13/valid/special_values/subnormal_not_zero.c",

        //"tests/chapter_13/helper_libs/nan.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter13ValidLexExtraCredit, "chapter_13", "--lex")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_13/invalid_types/extra_credit/bitwise_and.c",
        "tests/chapter_13/invalid_types/extra_credit/bitwise_or.c",
        "tests/chapter_13/invalid_types/extra_credit/bitwise_shift_double.c",
        "tests/chapter_13/invalid_types/extra_credit/bitwise_shift_double_2.c",
        "tests/chapter_13/invalid_types/extra_credit/bitwise_xor.c",
        "tests/chapter_13/invalid_types/extra_credit/compound_bitwise_and.c",
        "tests/chapter_13/invalid_types/extra_credit/compound_bitwise_xor.c",
        "tests/chapter_13/invalid_types/extra_credit/compound_left_bitshift.c",
        "tests/chapter_13/invalid_types/extra_credit/compound_mod.c",
        "tests/chapter_13/invalid_types/extra_credit/compound_mod_2.c",
        "tests/chapter_13/invalid_types/extra_credit/compound_right_bitshift.c",
        "tests/chapter_13/invalid_types/extra_credit/switch_double_case.c",
        "tests/chapter_13/invalid_types/extra_credit/switch_on_double.c",

        "tests/chapter_13/valid/extra_credit/compound_assign.c",
        "tests/chapter_13/valid/extra_credit/compound_assign_implicit_cast.c",
        "tests/chapter_13/valid/extra_credit/incr_and_decr.c",
        "tests/chapter_13/valid/extra_credit/nan.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// // Chapter 14
// TEST_CASE(Chapter14ValidLex, "chapter_14", "--lex")
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
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// TEST_CASE(Chapter14ValidLexExtraCredit, "chapter_14", "--lex")
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
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// // Chapter 15
// TEST_CASE(Chapter15ValidLex, "chapter_15", "--lex")
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
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// TEST_CASE(Chapter15ValidLexExtraCredit, "chapter_15", "--lex")
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
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// // Chapter 16
// TEST_CASE(Chapter16ValidLex, "chapter_16", "--lex")
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
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// TEST_CASE(Chapter16ValidLexExtraCredit, "chapter_16", "--lex")
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
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// // Chapter 17
// TEST_CASE(Chapter17ValidLex, "chapter_17", "--lex")
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
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// TEST_CASE(Chapter17ValidLexExtraCredit, "chapter_17", "--lex")
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
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// // Chapter 18
// TEST_CASE(Chapter18ValidLex, "chapter_18", "--lex")
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
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// TEST_CASE(Chapter18ValidLexExtraCredit, "chapter_18", "--lex")
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
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// // Chapter 19
// TEST_CASE(Chapter19ValidLex, "chapter_19", "--lex")
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
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// TEST_CASE(Chapter19ValidLexExtraCredit, "chapter_19", "--lex")
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
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// // Chapter 20
// TEST_CASE(Chapter20ValidLex, "chapter_20", "--lex")
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
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// TEST_CASE(Chapter20ValidLexExtraCredit, "chapter_20", "--lex")
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
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }