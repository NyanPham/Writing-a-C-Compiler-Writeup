#include "../TestFramework.h"
#include "Compiler.h"
#include "Settings.h"

#include <fstream>
#include <vector>
#include <string>
#include <iostream>

TEST_CASE(Chapter1ValidParse, "chapter_1", "--parse")
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            // Check that the compilation succeeded
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter2ValidParse, "chapter_2", "--parse")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            // Check that the compilation succeeded
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter3ValidParse, "chapter_3", "--parse")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter3ValidParseExtraCredit, "chapter_3", "--parse")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter4ValidParse, "chapter_4", "--parse")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter4ValidParseExtraCredit, "chapter_4", "--parse")
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter5ValidParse, "chapter_5", "--parse")
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter5ValidParseExtraCredit, "chapter_5", "--parse")
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter6ValidParse, "chapter_6", "--parse")
{
    std::vector<std::string> srcFiles = {
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter6ValidParseExtraCredit, "chapter_6", "--parse")
{
    std::vector<std::string> srcFiles = {
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter7ValidParse, "chapter_7", "--parse")
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter7ValidParseExtraCredit, "chapter_7", "--parse")
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter8ValidParse, "chapter_8", "--parse")
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter8ValidParseExtraCredit, "chapter_8", "--parse")
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter9ValidParse, "chapter_9", "--parse")
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter9ValidParseExtraCredit, "chapter_9", "--parse")
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter10ValidParse, "chapter_10", "--parse")
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter10ValidParseExtraCredit, "chapter_10", "--parse")
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
        "tests/chapter_10/invalid_labels/extra_credit/goto_global_var.c",
        "tests/chapter_10/invalid_types/extra_credit/static_var_case.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter11ValidParse, "chapter_11", "--parse")
{
    std::vector<std::string> srcFiles = {

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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter11ValidParseExtraCredit, "chapter_11", "--parse")
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
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
TEST_CASE(Chapter12ValidParse, "chapter_12", "--parse")
{
    std::vector<std::string> srcFiles = {
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter12ValidParseExtraCredit, "chapter_12", "--parse")
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
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
TEST_CASE(Chapter13ValidParse, "chapter_13", "--parse")
{
    std::vector<std::string> srcFiles = {
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter13ValidParseExtraCredit, "chapter_13", "--parse")
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
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 14
TEST_CASE(Chapter14ValidParse, "chapter_14", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_14/invalid_types/address_of_address.c",
        "tests/chapter_14/invalid_types/address_of_assignment.c",
        "tests/chapter_14/invalid_types/address_of_constant.c",
        "tests/chapter_14/invalid_types/address_of_ternary.c",
        "tests/chapter_14/invalid_types/assign_int_to_pointer.c",
        "tests/chapter_14/invalid_types/assign_int_var_to_pointer.c",
        "tests/chapter_14/invalid_types/assign_to_address.c",
        "tests/chapter_14/invalid_types/assign_wrong_pointer_type.c",
        "tests/chapter_14/invalid_types/bad_null_pointer_constant.c",
        "tests/chapter_14/invalid_types/cast_double_to_pointer.c",
        "tests/chapter_14/invalid_types/cast_pointer_to_double.c",
        "tests/chapter_14/invalid_types/compare_mixed_pointer_types.c",
        "tests/chapter_14/invalid_types/compare_pointer_to_ulong.c",
        "tests/chapter_14/invalid_types/complement_pointer.c",
        "tests/chapter_14/invalid_types/dereference_non_pointer.c",
        "tests/chapter_14/invalid_types/divide_pointer.c",
        "tests/chapter_14/invalid_types/invalid_pointer_initializer.c",
        "tests/chapter_14/invalid_types/invalid_static_initializer.c",
        "tests/chapter_14/invalid_types/multiply_pointers.c",
        "tests/chapter_14/invalid_types/multiply_pointers_2.c",
        "tests/chapter_14/invalid_types/negate_pointer.c",
        "tests/chapter_14/invalid_types/pass_pointer_as_int.c",
        "tests/chapter_14/invalid_types/return_wrong_pointer_type.c",
        "tests/chapter_14/invalid_types/ternary_mixed_pointer_types.c",

        "tests/chapter_14/valid/casts/cast_between_pointer_types.c",
        "tests/chapter_14/valid/casts/null_pointer_conversion.c",
        "tests/chapter_14/valid/casts/pointer_int_casts.c",
        "tests/chapter_14/valid/comparisons/compare_pointers.c",
        "tests/chapter_14/valid/comparisons/compare_to_null.c",
        "tests/chapter_14/valid/comparisons/pointers_as_conditions.c",
        "tests/chapter_14/valid/declarators/abstract_declarators.c",
        "tests/chapter_14/valid/declarators/declarators.c",
        "tests/chapter_14/valid/declarators/declare_pointer_in_for_loop.c",
        "tests/chapter_14/valid/dereference/address_of_dereference.c",
        "tests/chapter_14/valid/dereference/dereference_expression_result.c",
        "tests/chapter_14/valid/dereference/multilevel_indirection.c",
        "tests/chapter_14/valid/dereference/read_through_pointers.c",
        "tests/chapter_14/valid/dereference/simple.c",
        "tests/chapter_14/valid/dereference/static_var_indirection.c",
        "tests/chapter_14/valid/dereference/update_through_pointers.c",

        "tests/chapter_14/valid/function_calls/address_of_argument.c",
        "tests/chapter_14/valid/function_calls/return_pointer.c",
        "tests/chapter_14/valid/function_calls/update_value_through_pointer_parameter.c",
        "tests/chapter_14/valid/libraries/global_pointer.c",
        "tests/chapter_14/valid/libraries/global_pointer_client.c",
        "tests/chapter_14/valid/libraries/static_pointer.c",
        "tests/chapter_14/valid/libraries/static_pointer_client.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter14ValidParseExtraCredit, "chapter_14", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_14/invalid_types/extra_credit/bitwise_and_pointer.c",
        "tests/chapter_14/invalid_types/extra_credit/bitwise_compound_assign_to_pointer.c",
        "tests/chapter_14/invalid_types/extra_credit/bitwise_compound_assign_with_pointer.c",
        "tests/chapter_14/invalid_types/extra_credit/bitwise_lshift_pointer.c",
        "tests/chapter_14/invalid_types/extra_credit/bitwise_or_pointer.c",
        "tests/chapter_14/invalid_types/extra_credit/bitwise_rshift_pointer.c",
        "tests/chapter_14/invalid_types/extra_credit/bitwise_xor_pointer.c",
        "tests/chapter_14/invalid_types/extra_credit/compound_assignment_not_lval.c",
        "tests/chapter_14/invalid_types/extra_credit/compound_assign_thru_ptr_not_lval.c",
        "tests/chapter_14/invalid_types/extra_credit/compound_divide_pointer.c",
        "tests/chapter_14/invalid_types/extra_credit/compound_mod_pointer.c",
        "tests/chapter_14/invalid_types/extra_credit/compound_multiply_pointer.c",
        "tests/chapter_14/invalid_types/extra_credit/postfix_decr_not_lvalue.c",
        "tests/chapter_14/invalid_types/extra_credit/prefix_incr_not_lvalue.c",
        "tests/chapter_14/invalid_types/extra_credit/switch_on_pointer.c",

        "tests/chapter_14/invalid_declarations/extra_credit/addr_of_label.c",
        "tests/chapter_14/invalid_declarations/extra_credit/deref_label.c",

        "tests/chapter_14/valid/extra_credit/bitshift_dereferenced_ptrs.c",
        "tests/chapter_14/valid/extra_credit/bitwise_ops_with_dereferenced_ptrs.c",
        "tests/chapter_14/valid/extra_credit/compound_assign_conversion.c",
        "tests/chapter_14/valid/extra_credit/compound_assign_through_pointer.c",
        "tests/chapter_14/valid/extra_credit/compound_bitwise_dereferenced_ptrs.c",
        "tests/chapter_14/valid/extra_credit/eval_compound_lhs_once.c",
        "tests/chapter_14/valid/extra_credit/incr_and_decr_through_pointer.c",
        "tests/chapter_14/valid/extra_credit/switch_dereferenced_pointer.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 15
TEST_CASE(Chapter15ValidParse, "chapter_15", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_15/invalid_types/add_two_pointers.c",
        "tests/chapter_15/invalid_types/assign_incompatible_pointer_types.c",
        "tests/chapter_15/invalid_types/assign_to_array.c",
        "tests/chapter_15/invalid_types/assign_to_array_2.c",
        "tests/chapter_15/invalid_types/assign_to_array_3.c",
        "tests/chapter_15/invalid_types/bad_arg_type.c",
        "tests/chapter_15/invalid_types/cast_to_array_type.c",
        "tests/chapter_15/invalid_types/cast_to_array_type_2.c",
        "tests/chapter_15/invalid_types/cast_to_array_type_3.c",
        "tests/chapter_15/invalid_types/compare_different_pointer_types.c",
        "tests/chapter_15/invalid_types/compare_explicit_and_implict_addr.c",
        "tests/chapter_15/invalid_types/compare_pointer_to_int.c",
        "tests/chapter_15/invalid_types/compare_pointer_to_zero.c",
        "tests/chapter_15/invalid_types/compound_initializer_for_scalar.c",
        "tests/chapter_15/invalid_types/compound_initializer_for_static_scalar.c",
        "tests/chapter_15/invalid_types/compound_initializer_too_long_static.c",
        "tests/chapter_15/invalid_types/compound_inititializer_too_long.c",
        "tests/chapter_15/invalid_types/conflicting_array_declarations.c",
        "tests/chapter_15/invalid_types/conflicting_function_declarations.c",
        "tests/chapter_15/invalid_types/double_subscript.c",
        "tests/chapter_15/invalid_types/function_returns_array.c",
        "tests/chapter_15/invalid_types/incompatible_elem_type_compound_init.c",
        "tests/chapter_15/invalid_types/incompatible_elem_type_static_compound_init.c",
        "tests/chapter_15/invalid_types/null_ptr_array_initializer.c",
        "tests/chapter_15/invalid_types/null_ptr_static_array_initializer.c",
        "tests/chapter_15/invalid_types/scalar_initializer_for_array.c",
        "tests/chapter_15/invalid_types/scalar_initializer_for_static_array.c",
        "tests/chapter_15/invalid_types/static_non_const_array.c",
        "tests/chapter_15/invalid_types/subscript_both_pointers.c",
        "tests/chapter_15/invalid_types/subscript_non_ptr.c",
        "tests/chapter_15/invalid_types/sub_different_pointer_types.c",
        "tests/chapter_15/invalid_types/sub_double_from_ptr.c",
        "tests/chapter_15/invalid_types/sub_ptr_from_int.c",

        "tests/chapter_15/valid/allocation/test_alignment.c",
        "tests/chapter_15/valid/casts/cast_array_of_pointers.c",
        "tests/chapter_15/valid/casts/implicit_and_explicit_conversions.c",
        "tests/chapter_15/valid/casts/multi_dim_casts.c",
        "tests/chapter_15/valid/declarators/array_as_argument.c",
        "tests/chapter_15/valid/declarators/big_array.c",
        "tests/chapter_15/valid/declarators/equivalent_declarators.c",
        "tests/chapter_15/valid/declarators/for_loop_array.c",
        "tests/chapter_15/valid/declarators/return_nested_array.c",

        "tests/chapter_15/valid/initialization/automatic.c",
        "tests/chapter_15/valid/initialization/automatic_nested.c",
        "tests/chapter_15/valid/initialization/static.c",
        "tests/chapter_15/valid/initialization/static_nested.c",
        "tests/chapter_15/valid/initialization/trailing_comma_initializer.c",
        "tests/chapter_15/valid/libraries/global_array.c",
        "tests/chapter_15/valid/libraries/global_array_client.c",
        "tests/chapter_15/valid/libraries/return_pointer_to_array.c",
        "tests/chapter_15/valid/libraries/return_pointer_to_array_client.c",
        "tests/chapter_15/valid/libraries/set_array_val.c",
        "tests/chapter_15/valid/libraries/set_array_val_client.c",
        "tests/chapter_15/valid/pointer_arithmetic/add_dereference_and_assign.c",
        "tests/chapter_15/valid/pointer_arithmetic/compare.c",
        "tests/chapter_15/valid/pointer_arithmetic/pointer_add.c",
        "tests/chapter_15/valid/pointer_arithmetic/pointer_diff.c",
        "tests/chapter_15/valid/subscripting/addition_subscript_equivalence.c",
        "tests/chapter_15/valid/subscripting/array_of_pointers_to_arrays.c",
        "tests/chapter_15/valid/subscripting/complex_operands.c",
        "tests/chapter_15/valid/subscripting/simple.c",
        "tests/chapter_15/valid/subscripting/simple_subscripts.c",
        "tests/chapter_15/valid/subscripting/subscript_nested.c",
        "tests/chapter_15/valid/subscripting/subscript_pointer.c",
        "tests/chapter_15/valid/subscripting/subscript_precedence.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter15ValidParseExtraCredit, "chapter_15", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_15/invalid_types/extra_credit/compound_add_double_to_pointer.c",
        "tests/chapter_15/invalid_types/extra_credit/compound_add_two_pointers.c",
        "tests/chapter_15/invalid_types/extra_credit/compound_assign_to_array.c",
        "tests/chapter_15/invalid_types/extra_credit/compound_assign_to_nested_array.c",
        "tests/chapter_15/invalid_types/extra_credit/compound_sub_pointer_from_int.c",
        "tests/chapter_15/invalid_types/extra_credit/postfix_incr_array.c",
        "tests/chapter_15/invalid_types/extra_credit/postfix_incr_nested_array.c",
        "tests/chapter_15/invalid_types/extra_credit/prefix_decr_array.c",
        "tests/chapter_15/invalid_types/extra_credit/prefix_decr_nested_array.c",
        "tests/chapter_15/invalid_types/extra_credit/switch_on_array.c",

        "tests/chapter_15/valid/extra_credit/bitwise_subscript.c",
        "tests/chapter_15/valid/extra_credit/compound_assign_and_increment.c",
        "tests/chapter_15/valid/extra_credit/compound_assign_array_of_pointers.c",
        "tests/chapter_15/valid/extra_credit/compound_assign_to_nested_subscript.c",
        "tests/chapter_15/valid/extra_credit/compound_assign_to_subscripted_val.c",
        "tests/chapter_15/valid/extra_credit/compound_bitwise_subscript.c",
        "tests/chapter_15/valid/extra_credit/compound_lval_evaluated_once.c",
        "tests/chapter_15/valid/extra_credit/compound_nested_pointer_assignment.c",
        "tests/chapter_15/valid/extra_credit/compound_pointer_assignment.c",
        "tests/chapter_15/valid/extra_credit/incr_and_decr_nested_pointers.c",
        "tests/chapter_15/valid/extra_credit/incr_and_decr_pointers.c",
        "tests/chapter_15/valid/extra_credit/incr_decr_subscripted_vals.c",
        "tests/chapter_15/valid/extra_credit/postfix_prefix_precedence.c",

    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 16
TEST_CASE(Chapter16ValidParse, "chapter_16", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_16/invalid_types/assign_to_string_literal.c",
        "tests/chapter_16/invalid_types/char_and_schar_conflict.c",
        "tests/chapter_16/invalid_types/char_and_uchar_conflict.c",
        "tests/chapter_16/invalid_types/compound_initializer_for_pointer.c",
        "tests/chapter_16/invalid_types/implicit_conversion_between_char_pointers.c",
        "tests/chapter_16/invalid_types/implicit_conversion_pointers_to_different_size_arrays.c",
        "tests/chapter_16/invalid_types/negate_char_pointer.c",
        "tests/chapter_16/invalid_types/string_initializer_for_multidim_array.c",
        "tests/chapter_16/invalid_types/string_initializer_too_long.c",
        "tests/chapter_16/invalid_types/string_initializer_too_long_nested.c",
        "tests/chapter_16/invalid_types/string_initializer_too_long_nested_static.c",
        "tests/chapter_16/invalid_types/string_initializer_too_long_static.c",
        "tests/chapter_16/invalid_types/string_initializer_wrong_type.c",
        "tests/chapter_16/invalid_types/string_initializer_wrong_type_nested.c",
        "tests/chapter_16/invalid_types/string_initializer_wrong_type_nested_static.c",
        "tests/chapter_16/invalid_types/string_literal_is_plain_char_pointer.c",
        "tests/chapter_16/invalid_types/string_literal_is_plain_char_pointer_static.c",

        "tests/chapter_16/valid/chars/access_through_char_pointer.c",
        "tests/chapter_16/valid/chars/chained_casts.c",
        "tests/chapter_16/valid/chars/char_arguments.c",
        "tests/chapter_16/valid/chars/char_expressions.c",
        "tests/chapter_16/valid/chars/common_type.c",
        "tests/chapter_16/valid/chars/convert_by_assignment.c",
        // "tests/chapter_16/valid/chars/data_on_page_boundary_linux.s",
        // "tests/chapter_16/valid/chars/data_on_page_boundary_osx.s",
        "tests/chapter_16/valid/chars/explicit_casts.c",
        "tests/chapter_16/valid/chars/integer_promotion.c",
        "tests/chapter_16/valid/chars/partial_initialization.c",
        "tests/chapter_16/valid/chars/push_arg_on_page_boundary.c",
        "tests/chapter_16/valid/chars/return_char.c",
        "tests/chapter_16/valid/chars/rewrite_movz_regression.c",
        "tests/chapter_16/valid/chars/static_initializers.c",
        "tests/chapter_16/valid/chars/type_specifiers.c",
        "tests/chapter_16/valid/char_constants/char_constant_operations.c",
        "tests/chapter_16/valid/char_constants/control_characters.c",
        "tests/chapter_16/valid/char_constants/escape_sequences.c",
        "tests/chapter_16/valid/char_constants/return_char_constant.c",

        "tests/chapter_16/valid/libraries/char_arguments.c",
        "tests/chapter_16/valid/libraries/char_arguments_client.c",
        "tests/chapter_16/valid/libraries/global_char.c",
        "tests/chapter_16/valid/libraries/global_char_client.c",
        "tests/chapter_16/valid/libraries/return_char.c",
        "tests/chapter_16/valid/libraries/return_char_client.c",
        "tests/chapter_16/valid/strings_as_initializers/adjacent_strings_in_initializer.c",
        "tests/chapter_16/valid/strings_as_initializers/array_init_special_chars.c",
        "tests/chapter_16/valid/strings_as_initializers/literals_and_compound_initializers.c",
        "tests/chapter_16/valid/strings_as_initializers/partial_initialize_via_string.c",
        "tests/chapter_16/valid/strings_as_initializers/simple.c",
        "tests/chapter_16/valid/strings_as_initializers/terminating_null_bytes.c",
        "tests/chapter_16/valid/strings_as_initializers/test_alignment.c",
        "tests/chapter_16/valid/strings_as_initializers/transfer_by_eightbyte.c",
        "tests/chapter_16/valid/strings_as_initializers/write_to_array.c",
        "tests/chapter_16/valid/strings_as_lvalues/addr_of_string.c",
        "tests/chapter_16/valid/strings_as_lvalues/adjacent_strings.c",
        "tests/chapter_16/valid/strings_as_lvalues/array_of_strings.c",
        "tests/chapter_16/valid/strings_as_lvalues/cast_string_pointer.c",
        "tests/chapter_16/valid/strings_as_lvalues/empty_string.c",
        "tests/chapter_16/valid/strings_as_lvalues/pointer_operations.c",
        "tests/chapter_16/valid/strings_as_lvalues/simple.c",
        "tests/chapter_16/valid/strings_as_lvalues/standard_library_calls.c",
        "tests/chapter_16/valid/strings_as_lvalues/strings_in_function_calls.c",
        "tests/chapter_16/valid/strings_as_lvalues/string_special_characters.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter16ValidParseExtraCredit, "chapter_16", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_16/invalid_types/extra_credit/bitwise_operation_on_string.c",
        "tests/chapter_16/invalid_types/extra_credit/bit_shift_string.c",
        "tests/chapter_16/invalid_types/extra_credit/case_statement_string.c",
        "tests/chapter_16/invalid_types/extra_credit/compound_assign_from_string.c",
        "tests/chapter_16/invalid_types/extra_credit/compound_assign_to_string.c",
        "tests/chapter_16/invalid_types/extra_credit/postfix_incr_string.c",
        "tests/chapter_16/invalid_types/extra_credit/prefix_incr_string.c",
        "tests/chapter_16/invalid_types/extra_credit/switch_on_string.c",
        "tests/chapter_16/invalid_labels/extra_credit/duplicate_case_char_const.c",

        "tests/chapter_16/valid/extra_credit/bitshift_chars.c",
        "tests/chapter_16/valid/extra_credit/bitwise_ops_character_constants.c",
        "tests/chapter_16/valid/extra_credit/bitwise_ops_chars.c",
        "tests/chapter_16/valid/extra_credit/char_consts_as_cases.c",
        "tests/chapter_16/valid/extra_credit/compound_assign_chars.c",
        "tests/chapter_16/valid/extra_credit/compound_bitwise_ops_chars.c",
        "tests/chapter_16/valid/extra_credit/incr_decr_chars.c",
        "tests/chapter_16/valid/extra_credit/incr_decr_unsigned_chars.c",
        "tests/chapter_16/valid/extra_credit/promote_switch_cond.c",
        "tests/chapter_16/valid/extra_credit/promote_switch_cond_2.c",
        "tests/chapter_16/valid/extra_credit/switch_on_char_const.c",

    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 17
TEST_CASE(Chapter17ValidParse, "chapter_17", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_17/invalid_types/incomplete_types/add_void_pointer.c",
        "tests/chapter_17/invalid_types/incomplete_types/sizeof_function.c",
        "tests/chapter_17/invalid_types/incomplete_types/sizeof_void.c",
        "tests/chapter_17/invalid_types/incomplete_types/sizeof_void_array.c",
        "tests/chapter_17/invalid_types/incomplete_types/sizeof_void_expression.c",
        "tests/chapter_17/invalid_types/incomplete_types/subscript_void.c",
        "tests/chapter_17/invalid_types/incomplete_types/subscript_void_pointer_conditional.c",
        "tests/chapter_17/invalid_types/incomplete_types/sub_void_pointer.c",
        "tests/chapter_17/invalid_types/incomplete_types/void_array.c",
        "tests/chapter_17/invalid_types/incomplete_types/void_array_in_cast.c",
        "tests/chapter_17/invalid_types/incomplete_types/void_array_in_param_type.c",
        "tests/chapter_17/invalid_types/incomplete_types/void_array_nested_in_declaration.c",
        "tests/chapter_17/invalid_types/incomplete_types/void_array_pointer_in_declaration.c",
        "tests/chapter_17/invalid_types/incomplete_types/void_array_pointer_in_param_type.c",
        "tests/chapter_17/invalid_types/pointer_conversions/compare_void_ptr_to_int.c",
        "tests/chapter_17/invalid_types/pointer_conversions/compare_void_to_other_pointer.c",
        "tests/chapter_17/invalid_types/pointer_conversions/convert_ulong_to_void_ptr.c",
        "tests/chapter_17/invalid_types/pointer_conversions/convert_void_ptr_to_int.c",
        "tests/chapter_17/invalid_types/pointer_conversions/usual_arithmetic_conversions_ptr.c",
        "tests/chapter_17/invalid_types/scalar_expressions/and_void.c",
        "tests/chapter_17/invalid_types/scalar_expressions/cast_void.c",
        "tests/chapter_17/invalid_types/scalar_expressions/not_void.c",
        "tests/chapter_17/invalid_types/scalar_expressions/or_void.c",
        "tests/chapter_17/invalid_types/scalar_expressions/void_condition_do_loop.c",
        "tests/chapter_17/invalid_types/scalar_expressions/void_condition_for_loop.c",
        "tests/chapter_17/invalid_types/scalar_expressions/void_condition_while_loop.c",
        "tests/chapter_17/invalid_types/scalar_expressions/void_if_condition.c",
        "tests/chapter_17/invalid_types/scalar_expressions/void_ternary_condition.c",
        "tests/chapter_17/invalid_types/void/assign_to_void_lvalue.c",
        "tests/chapter_17/invalid_types/void/assign_to_void_var.c",
        "tests/chapter_17/invalid_types/void/assign_void_rval.c",
        "tests/chapter_17/invalid_types/void/define_void.c",
        "tests/chapter_17/invalid_types/void/initialized_void.c",
        "tests/chapter_17/invalid_types/void/mismatched_conditional.c",
        "tests/chapter_17/invalid_types/void/negate_void.c",
        "tests/chapter_17/invalid_types/void/non_void_return.c",
        "tests/chapter_17/invalid_types/void/no_return_value.c",
        "tests/chapter_17/invalid_types/void/return_void_as_pointer.c",
        "tests/chapter_17/invalid_types/void/subscript_void.c",
        "tests/chapter_17/invalid_types/void/void_compare.c",
        "tests/chapter_17/invalid_types/void/void_equality.c",
        "tests/chapter_17/invalid_types/void/void_fun_params.c",

        "tests/chapter_17/valid/libraries/pass_alloced_memory.c",
        "tests/chapter_17/valid/libraries/pass_alloced_memory_client.c",
        "tests/chapter_17/valid/libraries/sizeof_extern.c",
        "tests/chapter_17/valid/libraries/sizeof_extern_client.c",
        "tests/chapter_17/valid/libraries/test_for_memory_leaks.c",
        "tests/chapter_17/valid/libraries/test_for_memory_leaks_client.c",
        "tests/chapter_17/valid/sizeof/simple.c",
        "tests/chapter_17/valid/sizeof/sizeof_array.c",
        "tests/chapter_17/valid/sizeof/sizeof_basic_types.c",
        "tests/chapter_17/valid/sizeof/sizeof_consts.c",
        "tests/chapter_17/valid/sizeof/sizeof_derived_types.c",
        "tests/chapter_17/valid/sizeof/sizeof_expressions.c",
        "tests/chapter_17/valid/sizeof/sizeof_not_evaluated.c",
        "tests/chapter_17/valid/sizeof/sizeof_result_is_ulong.c",
        "tests/chapter_17/valid/void/cast_to_void.c",
        "tests/chapter_17/valid/void/ternary.c",
        "tests/chapter_17/valid/void/void_for_loop.c",
        "tests/chapter_17/valid/void/void_function.c",
        "tests/chapter_17/valid/void_pointer/array_of_pointers_to_void.c",
        "tests/chapter_17/valid/void_pointer/common_pointer_type.c",
        "tests/chapter_17/valid/void_pointer/conversion_by_assignment.c",
        "tests/chapter_17/valid/void_pointer/explicit_cast.c",
        "tests/chapter_17/valid/void_pointer/memory_management_functions.c",
        "tests/chapter_17/valid/void_pointer/simple.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter17ValidParseExtraCredit, "chapter_17", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_17/invalid_types/extra_credit/bitshift_void.c",
        "tests/chapter_17/invalid_types/extra_credit/bitwise_void.c",
        "tests/chapter_17/invalid_types/extra_credit/compound_add_void_pointer.c",
        "tests/chapter_17/invalid_types/extra_credit/compound_sub_void_pointer.c",
        "tests/chapter_17/invalid_types/extra_credit/compound_void_rval.c",
        "tests/chapter_17/invalid_types/extra_credit/compound_void_rval_add.c",
        "tests/chapter_17/invalid_types/extra_credit/compound_void_rval_bitshift.c",
        "tests/chapter_17/invalid_types/extra_credit/postfix_decr_void.c",
        "tests/chapter_17/invalid_types/extra_credit/postfix_decr_void_pointer.c",
        "tests/chapter_17/invalid_types/extra_credit/postfix_incr_void_pointer.c",
        "tests/chapter_17/invalid_types/extra_credit/prefix_decr_void_pointer.c",
        "tests/chapter_17/invalid_types/extra_credit/prefix_incr_void.c",
        "tests/chapter_17/invalid_types/extra_credit/prefix_incr_void_pointer.c",
        "tests/chapter_17/invalid_types/extra_credit/switch_void.c",

        "tests/chapter_17/valid/extra_credit/sizeof_bitwise.c",
        "tests/chapter_17/valid/extra_credit/sizeof_compound.c",
        "tests/chapter_17/valid/extra_credit/sizeof_compound_bitwise.c",
        "tests/chapter_17/valid/extra_credit/sizeof_incr.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// Chapter 18
TEST_CASE(Chapter18ValidParse, "chapter_18", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_18/invalid_struct_tags/array_of_undeclared.c",
        "tests/chapter_18/invalid_struct_tags/cast_undeclared.c",
        "tests/chapter_18/invalid_struct_tags/deref_undeclared.c",
        "tests/chapter_18/invalid_struct_tags/file_scope_var_type_undeclared.c",
        "tests/chapter_18/invalid_struct_tags/for_loop_scope.c",
        "tests/chapter_18/invalid_struct_tags/for_loop_scope_2.c",
        "tests/chapter_18/invalid_struct_tags/member_type_undeclared.c",
        "tests/chapter_18/invalid_struct_tags/param_undeclared.c",
        "tests/chapter_18/invalid_struct_tags/return_type_undeclared.c",
        "tests/chapter_18/invalid_struct_tags/sizeof_undeclared.c",
        "tests/chapter_18/invalid_struct_tags/var_type_undeclared.c",
        "tests/chapter_18/invalid_types/incompatible_types/assign_different_pointer_type.c",
        "tests/chapter_18/invalid_types/incompatible_types/assign_different_struct_type.c",
        "tests/chapter_18/invalid_types/incompatible_types/branch_mismatch.c",
        "tests/chapter_18/invalid_types/incompatible_types/branch_mismatch_2.c",
        "tests/chapter_18/invalid_types/incompatible_types/compare_different_struct_pointers.c",
        "tests/chapter_18/invalid_types/incompatible_types/return_wrong_struct_type.c",
        "tests/chapter_18/invalid_types/incompatible_types/struct_param_mismatch.c",
        "tests/chapter_18/invalid_types/incompatible_types/struct_pointer_param_mismatch.c",
        "tests/chapter_18/invalid_types/initializers/compound_initializer_too_long.c",
        "tests/chapter_18/invalid_types/initializers/initialize_nested_static_struct_member_wrong_type.c",
        "tests/chapter_18/invalid_types/initializers/initialize_static_struct_with_zero.c",
        "tests/chapter_18/invalid_types/initializers/initialize_struct_member_wrong_type.c",
        "tests/chapter_18/invalid_types/initializers/initialize_struct_with_scalar.c",
        "tests/chapter_18/invalid_types/initializers/initialize_struct_wrong_type.c",
        "tests/chapter_18/invalid_types/initializers/init_struct_with_string.c",
        "tests/chapter_18/invalid_types/initializers/nested_compound_initializer_too_long.c",
        "tests/chapter_18/invalid_types/initializers/nested_static_compound_initializer_too_long.c",
        "tests/chapter_18/invalid_types/initializers/nested_struct_initializer_wrong_type.c",
        "tests/chapter_18/invalid_types/initializers/non_constant_static_elem_init.c",
        "tests/chapter_18/invalid_types/initializers/non_constant_static_init.c",
        "tests/chapter_18/invalid_types/initializers/static_initializer_too_long.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/assign_to_incomplete_var.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/cast_incomplete_struct.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/deref_incomplete_struct_pointer.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_arg_funcall.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_array_element.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_local_var.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_param.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_ptr_addition.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_ptr_subtraction.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_return_type_funcall.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_return_type_fun_def.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_struct_conditional.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_struct_full_expr.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_struct_member.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_subscript.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_tentative_def.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/initialize_incomplete.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/sizeof_incomplete.c",
        "tests/chapter_18/invalid_types/invalid_incomplete_structs/sizeof_incomplete_expr.c",
        "tests/chapter_18/invalid_types/invalid_lvalues/address_of_non_lvalue.c",
        "tests/chapter_18/invalid_types/invalid_lvalues/assign_nested_non_lvalue.c",
        "tests/chapter_18/invalid_types/invalid_lvalues/assign_to_array.c",
        "tests/chapter_18/invalid_types/invalid_lvalues/assign_to_non_lvalue.c",
        "tests/chapter_18/invalid_types/invalid_member_operators/arrow_pointer_to_non_struct.c",
        "tests/chapter_18/invalid_types/invalid_member_operators/bad_member.c",
        "tests/chapter_18/invalid_types/invalid_member_operators/bad_pointer_member.c",
        "tests/chapter_18/invalid_types/invalid_member_operators/member_of_non_struct.c",
        "tests/chapter_18/invalid_types/invalid_member_operators/member_pointer_non_struct_pointer.c",
        "tests/chapter_18/invalid_types/invalid_member_operators/nested_arrow_pointer_to_non_struct.c",
        "tests/chapter_18/invalid_types/invalid_member_operators/postfix_precedence.c",
        "tests/chapter_18/invalid_types/invalid_struct_declaration/duplicate_member_name.c",
        "tests/chapter_18/invalid_types/invalid_struct_declaration/duplicate_struct_declaration.c",
        "tests/chapter_18/invalid_types/invalid_struct_declaration/incomplete_member.c",
        "tests/chapter_18/invalid_types/invalid_struct_declaration/invalid_array_member.c",
        "tests/chapter_18/invalid_types/invalid_struct_declaration/invalid_self_reference.c",
        "tests/chapter_18/invalid_types/invalid_struct_declaration/void_member.c",
        "tests/chapter_18/invalid_types/scalar_required/and_struct.c",
        "tests/chapter_18/invalid_types/scalar_required/assign_null_ptr_to_struct.c",
        "tests/chapter_18/invalid_types/scalar_required/assign_scalar_to_struct.c",
        "tests/chapter_18/invalid_types/scalar_required/cast_struct_to_scalar.c",
        "tests/chapter_18/invalid_types/scalar_required/cast_to_struct.c",
        "tests/chapter_18/invalid_types/scalar_required/compare_structs.c",
        "tests/chapter_18/invalid_types/scalar_required/not_struct.c",
        "tests/chapter_18/invalid_types/scalar_required/pass_struct_as_scalar_param.c",
        "tests/chapter_18/invalid_types/scalar_required/struct_as_int.c",
        "tests/chapter_18/invalid_types/scalar_required/struct_controlling_expression.c",
        "tests/chapter_18/invalid_types/scalar_required/subscript_struct.c",
        "tests/chapter_18/invalid_types/tag_resolution/address_of_wrong_type.c",
        "tests/chapter_18/invalid_types/tag_resolution/conflicting_fun_param_types.c",
        "tests/chapter_18/invalid_types/tag_resolution/conflicting_fun_ret_types.c",
        "tests/chapter_18/invalid_types/tag_resolution/distinct_struct_types.c",
        "tests/chapter_18/invalid_types/tag_resolution/incomplete_shadows_complete.c",
        "tests/chapter_18/invalid_types/tag_resolution/incomplete_shadows_complete_cast.c",
        "tests/chapter_18/invalid_types/tag_resolution/invalid_shadow_self_reference.c",
        "tests/chapter_18/invalid_types/tag_resolution/member_name_wrong_scope.c",
        "tests/chapter_18/invalid_types/tag_resolution/member_name_wrong_scope_nested.c",
        "tests/chapter_18/invalid_types/tag_resolution/mismatched_return_type.c",
        "tests/chapter_18/invalid_types/tag_resolution/shadowed_tag_branch_mismatch.c",
        "tests/chapter_18/invalid_types/tag_resolution/shadow_struct.c",
        //"tests/chapter_18/valid/no_structure_parameters/README.md",
        "tests/chapter_18/valid/no_structure_parameters/libraries/array_of_structs.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/array_of_structs.h",
        "tests/chapter_18/valid/no_structure_parameters/libraries/array_of_structs_client.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/global_struct.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/global_struct.h",
        "tests/chapter_18/valid/no_structure_parameters/libraries/global_struct_client.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/opaque_struct.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/opaque_struct_client.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/param_struct_pointer.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/param_struct_pointer.h",
        "tests/chapter_18/valid/no_structure_parameters/libraries/param_struct_pointer_client.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/return_struct_pointer.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/return_struct_pointer.h",
        "tests/chapter_18/valid/no_structure_parameters/libraries/return_struct_pointer_client.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/initializers/auto_struct_initializers.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/initializers/auto_struct_initializers.h",
        "tests/chapter_18/valid/no_structure_parameters/libraries/initializers/auto_struct_initializers_client.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/initializers/nested_auto_struct_initializers.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/initializers/nested_auto_struct_initializers.h",
        "tests/chapter_18/valid/no_structure_parameters/libraries/initializers/nested_auto_struct_initializers_client.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/initializers/nested_static_struct_initializers.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/initializers/nested_static_struct_initializers.h",
        "tests/chapter_18/valid/no_structure_parameters/libraries/initializers/nested_static_struct_initializers_client.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/initializers/static_struct_initializers.c",
        "tests/chapter_18/valid/no_structure_parameters/libraries/initializers/static_struct_initializers.h",
        "tests/chapter_18/valid/no_structure_parameters/libraries/initializers/static_struct_initializers_client.c",
        "tests/chapter_18/valid/no_structure_parameters/parse_and_lex/postfix_precedence.c",
        "tests/chapter_18/valid/no_structure_parameters/parse_and_lex/space_around_struct_member.c",
        "tests/chapter_18/valid/no_structure_parameters/parse_and_lex/struct_member_looks_like_const.c",
        "tests/chapter_18/valid/no_structure_parameters/parse_and_lex/trailing_comma.c",
        "tests/chapter_18/valid/no_structure_parameters/scalar_member_access/arrow.c",
        "tests/chapter_18/valid/no_structure_parameters/scalar_member_access/dot.c",
        "tests/chapter_18/valid/no_structure_parameters/scalar_member_access/linked_list.c",
        "tests/chapter_18/valid/no_structure_parameters/scalar_member_access/nested_struct.c",
        "tests/chapter_18/valid/no_structure_parameters/scalar_member_access/static_structs.c",
        "tests/chapter_18/valid/no_structure_parameters/semantic_analysis/cast_struct_to_void.c",
        "tests/chapter_18/valid/no_structure_parameters/semantic_analysis/incomplete_structs.c",
        "tests/chapter_18/valid/no_structure_parameters/semantic_analysis/namespaces.c",
        "tests/chapter_18/valid/no_structure_parameters/semantic_analysis/resolve_tags.c",
        "tests/chapter_18/valid/no_structure_parameters/size_and_offset_calculations/member_comparisons.c",
        "tests/chapter_18/valid/no_structure_parameters/size_and_offset_calculations/member_offsets.c",
        "tests/chapter_18/valid/no_structure_parameters/size_and_offset_calculations/sizeof_exps.c",
        "tests/chapter_18/valid/no_structure_parameters/size_and_offset_calculations/sizeof_type.c",
        "tests/chapter_18/valid/no_structure_parameters/size_and_offset_calculations/struct_sizes.h",
        "tests/chapter_18/valid/no_structure_parameters/smoke_tests/simple.c",
        "tests/chapter_18/valid/no_structure_parameters/smoke_tests/static_vs_auto.c",
        "tests/chapter_18/valid/no_structure_parameters/struct_copy/copy_struct.c",
        "tests/chapter_18/valid/no_structure_parameters/struct_copy/copy_struct_through_pointer.c",
        "tests/chapter_18/valid/no_structure_parameters/struct_copy/copy_struct_with_arrow_operator.c",
        "tests/chapter_18/valid/no_structure_parameters/struct_copy/copy_struct_with_dot_operator.c",
        "tests/chapter_18/valid/no_structure_parameters/struct_copy/stack_clobber.c",
        "tests/chapter_18/valid/no_structure_parameters/struct_copy/structs.h",
        //"tests/chapter_18/valid/parameters/data_on_page_boundary_linux.s",
        //"tests/chapter_18/valid/parameters/data_on_page_boundary_osx.s",
        "tests/chapter_18/valid/parameters/incomplete_param_type.c",
        "tests/chapter_18/valid/parameters/pass_args_on_page_boundary.c",
        "tests/chapter_18/valid/parameters/simple.c",
        "tests/chapter_18/valid/parameters/stack_clobber.c",
        "tests/chapter_18/valid/parameters/libraries/classify_params.c",
        "tests/chapter_18/valid/parameters/libraries/classify_params.h",
        "tests/chapter_18/valid/parameters/libraries/classify_params_client.c",
        "tests/chapter_18/valid/parameters/libraries/modify_param.c",
        "tests/chapter_18/valid/parameters/libraries/modify_param.h",
        "tests/chapter_18/valid/parameters/libraries/modify_param_client.c",
        "tests/chapter_18/valid/parameters/libraries/param_calling_conventions.c",
        "tests/chapter_18/valid/parameters/libraries/param_calling_conventions.h",
        "tests/chapter_18/valid/parameters/libraries/param_calling_conventions_client.c",
        "tests/chapter_18/valid/parameters/libraries/pass_struct.c",
        "tests/chapter_18/valid/parameters/libraries/pass_struct.h",
        "tests/chapter_18/valid/parameters/libraries/pass_struct_client.c",
        "tests/chapter_18/valid/parameters/libraries/struct_sizes.c",
        "tests/chapter_18/valid/parameters/libraries/struct_sizes.h",
        "tests/chapter_18/valid/parameters/libraries/struct_sizes_client.c",
        //"tests/chapter_18/valid/params_and_returns/big_data_on_page_boundary_linux.s",
        //"tests/chapter_18/valid/params_and_returns/big_data_on_page_boundary_osx.s",
        //"tests/chapter_18/valid/params_and_returns/data_on_page_boundary_linux.s",
        //"tests/chapter_18/valid/params_and_returns/data_on_page_boundary_osx.s",
        "tests/chapter_18/valid/params_and_returns/ignore_retval.c",
        "tests/chapter_18/valid/params_and_returns/return_big_struct_on_page_boundary.c",
        "tests/chapter_18/valid/params_and_returns/return_incomplete_type.c",
        //"tests/chapter_18/valid/params_and_returns/return_space_address_overlap_linux.s",
        //"tests/chapter_18/valid/params_and_returns/return_space_address_overlap_osx.s",
        "tests/chapter_18/valid/params_and_returns/return_space_overlap.c",
        "tests/chapter_18/valid/params_and_returns/return_struct_on_page_boundary.c",
        "tests/chapter_18/valid/params_and_returns/simple.c",
        "tests/chapter_18/valid/params_and_returns/stack_clobber.c",
        "tests/chapter_18/valid/params_and_returns/temporary_lifetime.c",
        "tests/chapter_18/valid/params_and_returns/libraries/access_retval_members.c",
        "tests/chapter_18/valid/params_and_returns/libraries/access_retval_members.h",
        "tests/chapter_18/valid/params_and_returns/libraries/access_retval_members_client.c",
        "tests/chapter_18/valid/params_and_returns/libraries/missing_retval.c",
        "tests/chapter_18/valid/params_and_returns/libraries/missing_retval.h",
        "tests/chapter_18/valid/params_and_returns/libraries/missing_retval_client.c",
        "tests/chapter_18/valid/params_and_returns/libraries/return_calling_conventions.c",
        "tests/chapter_18/valid/params_and_returns/libraries/return_calling_conventions.h",
        "tests/chapter_18/valid/params_and_returns/libraries/return_calling_conventions_client.c",
        "tests/chapter_18/valid/params_and_returns/libraries/retval_struct_sizes.c",
        "tests/chapter_18/valid/params_and_returns/libraries/retval_struct_sizes.h",
        "tests/chapter_18/valid/params_and_returns/libraries/retval_struct_sizes_client.c",
    };
    Settings settings;
    Compiler compiler;

#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < static_cast<int>(srcFiles.size()); ++i)
    {
        const std::string &srcFile = srcFiles[i];
        try
        {
            std::cout << "Compiling the file " << srcFile << " ..." << std::endl;
            int status = compiler.compile(Stage::Parsing, std::vector<std::string>{srcFile});
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter18ValidParseExtraCredit, "chapter_18", "--parse")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_18/invalid_struct_tags/extra_credit/sizeof_undeclared_union.c",
        "tests/chapter_18/invalid_struct_tags/extra_credit/var_undeclared_union_type.c",

        // //"tests/chapter_18/invalid_types/extra_credit/README.md",
        "tests/chapter_18/invalid_types/extra_credit/bad_union_member_access/nested_non_member.c",
        "tests/chapter_18/invalid_types/extra_credit/bad_union_member_access/union_bad_member.c",
        "tests/chapter_18/invalid_types/extra_credit/bad_union_member_access/union_bad_pointer_member.c",
        "tests/chapter_18/invalid_types/extra_credit/incompatible_union_types/assign_different_union_type.c",
        "tests/chapter_18/invalid_types/extra_credit/incompatible_union_types/assign_scalar_to_union.c",
        "tests/chapter_18/invalid_types/extra_credit/incompatible_union_types/return_type_mismatch.c",
        "tests/chapter_18/invalid_types/extra_credit/incompatible_union_types/union_branch_mismatch.c",
        "tests/chapter_18/invalid_types/extra_credit/incompatible_union_types/union_pointer_branch_mismatch.c",
        "tests/chapter_18/invalid_types/extra_credit/incomplete_unions/define_incomplete_union.c",
        "tests/chapter_18/invalid_types/extra_credit/incomplete_unions/sizeof_incomplete_union_type.c",
        "tests/chapter_18/invalid_types/extra_credit/invalid_union_lvalues/address_of_non_lvalue_union_member.c",
        "tests/chapter_18/invalid_types/extra_credit/invalid_union_lvalues/assign_non_lvalue_union_member.c",
        "tests/chapter_18/invalid_types/extra_credit/other_features/bitwise_op_structure.c",
        "tests/chapter_18/invalid_types/extra_credit/other_features/compound_assign_struct_rval.c",
        "tests/chapter_18/invalid_types/extra_credit/other_features/compound_assign_to_nested_struct.c",
        "tests/chapter_18/invalid_types/extra_credit/other_features/compound_assign_to_struct.c",
        "tests/chapter_18/invalid_types/extra_credit/other_features/duplicate_struct_types_after_label.c",
        "tests/chapter_18/invalid_types/extra_credit/other_features/postfix_decr_struct_arrow.c",
        "tests/chapter_18/invalid_types/extra_credit/other_features/postfix_incr_struct.c",
        "tests/chapter_18/invalid_types/extra_credit/other_features/prefix_decr_struct.c",
        "tests/chapter_18/invalid_types/extra_credit/other_features/prefix_incr_nested_struct.c",
        "tests/chapter_18/invalid_types/extra_credit/other_features/switch_on_struct.c",
        "tests/chapter_18/invalid_types/extra_credit/scalar_required/cast_between_unions.c",
        "tests/chapter_18/invalid_types/extra_credit/scalar_required/cast_union_to_int.c",
        "tests/chapter_18/invalid_types/extra_credit/scalar_required/compare_unions.c",
        "tests/chapter_18/invalid_types/extra_credit/scalar_required/switch_on_union.c",
        "tests/chapter_18/invalid_types/extra_credit/scalar_required/union_as_controlling_expression.c",
        "tests/chapter_18/invalid_types/extra_credit/union_initializers/initializer_too_long.c",
        "tests/chapter_18/invalid_types/extra_credit/union_initializers/nested_init_wrong_type.c",
        "tests/chapter_18/invalid_types/extra_credit/union_initializers/nested_union_init_too_long.c",
        "tests/chapter_18/invalid_types/extra_credit/union_initializers/scalar_union_initializer.c",
        "tests/chapter_18/invalid_types/extra_credit/union_initializers/static_aggregate_init_wrong_type.c",
        "tests/chapter_18/invalid_types/extra_credit/union_initializers/static_nested_init_not_const.c",
        "tests/chapter_18/invalid_types/extra_credit/union_initializers/static_nested_init_too_long.c",
        "tests/chapter_18/invalid_types/extra_credit/union_initializers/static_scalar_union_initializer.c",
        "tests/chapter_18/invalid_types/extra_credit/union_initializers/static_too_long.c",
        "tests/chapter_18/invalid_types/extra_credit/union_initializers/static_union_init_not_constant.c",
        "tests/chapter_18/invalid_types/extra_credit/union_initializers/static_union_init_wrong_type.c",
        "tests/chapter_18/invalid_types/extra_credit/union_initializers/union_init_wrong_type.c",
        "tests/chapter_18/invalid_types/extra_credit/union_struct_conflicts/conflicting_tag_declarations.c",
        "tests/chapter_18/invalid_types/extra_credit/union_struct_conflicts/conflicting_tag_decl_and_use.c",
        "tests/chapter_18/invalid_types/extra_credit/union_struct_conflicts/conflicting_tag_decl_and_use_self_reference.c",
        "tests/chapter_18/invalid_types/extra_credit/union_struct_conflicts/struct_shadowed_by_union.c",
        "tests/chapter_18/invalid_types/extra_credit/union_struct_conflicts/tag_decl_conflicts_with_def.c",
        "tests/chapter_18/invalid_types/extra_credit/union_struct_conflicts/tag_def_conflicts_with_decl.c",
        "tests/chapter_18/invalid_types/extra_credit/union_struct_conflicts/union_shadowed_by_incomplete_struct.c",
        "tests/chapter_18/invalid_types/extra_credit/union_tag_resolution/address_of_wrong_union_type.c",
        "tests/chapter_18/invalid_types/extra_credit/union_tag_resolution/compare_struct_and_union_ptrs.c",
        "tests/chapter_18/invalid_types/extra_credit/union_tag_resolution/conflicting_param_union_types.c",
        "tests/chapter_18/invalid_types/extra_credit/union_tag_resolution/distinct_union_types.c",
        "tests/chapter_18/invalid_types/extra_credit/union_tag_resolution/union_type_shadows_struct.c",
        "tests/chapter_18/invalid_types/extra_credit/union_tag_resolution/union_wrong_member.c",
        "tests/chapter_18/invalid_types/extra_credit/union_type_declarations/array_of_incomplete_union_type.c",
        "tests/chapter_18/invalid_types/extra_credit/union_type_declarations/duplicate_union_def.c",
        "tests/chapter_18/invalid_types/extra_credit/union_type_declarations/incomplete_union_member.c",
        "tests/chapter_18/invalid_types/extra_credit/union_type_declarations/member_name_conflicts.c",
        "tests/chapter_18/invalid_types/extra_credit/union_type_declarations/union_self_reference.c",
        // "tests/chapter_18/valid/extra_credit/README.md",
        // "tests/chapter_18/valid/extra_credit/union_types.h",
        "tests/chapter_18/valid/extra_credit/libraries/classify_unions.c",
        "tests/chapter_18/valid/extra_credit/libraries/classify_unions_client.c",
        "tests/chapter_18/valid/extra_credit/libraries/param_passing.c",
        "tests/chapter_18/valid/extra_credit/libraries/param_passing_client.c",
        "tests/chapter_18/valid/extra_credit/libraries/static_union_inits.c",
        // "tests/chapter_18/valid/extra_credit/libraries/static_union_inits.h",
        "tests/chapter_18/valid/extra_credit/libraries/static_union_inits_client.c",
        "tests/chapter_18/valid/extra_credit/libraries/union_inits.c",
        // "tests/chapter_18/valid/extra_credit/libraries/union_inits.h",
        "tests/chapter_18/valid/extra_credit/libraries/union_inits_client.c",
        // "tests/chapter_18/valid/extra_credit/libraries/union_lib.h",
        "tests/chapter_18/valid/extra_credit/libraries/union_retvals.c",
        "tests/chapter_18/valid/extra_credit/libraries/union_retvals_client.c",
        "tests/chapter_18/valid/extra_credit/member_access/nested_union_access.c",
        "tests/chapter_18/valid/extra_credit/member_access/static_union_access.c",
        "tests/chapter_18/valid/extra_credit/member_access/union_init_and_member_access.c",
        "tests/chapter_18/valid/extra_credit/member_access/union_temp_lifetime.c",
        "tests/chapter_18/valid/extra_credit/other_features/bitwise_ops_struct_members.c",
        "tests/chapter_18/valid/extra_credit/other_features/compound_assign_struct_members.c",
        "tests/chapter_18/valid/extra_credit/other_features/decr_arrow_lexing.c",
        "tests/chapter_18/valid/extra_credit/other_features/incr_struct_members.c",
        "tests/chapter_18/valid/extra_credit/other_features/label_tag_member_namespace.c",
        "tests/chapter_18/valid/extra_credit/other_features/struct_decl_in_switch_statement.c",
        "tests/chapter_18/valid/extra_credit/semantic_analysis/cast_union_to_void.c",
        "tests/chapter_18/valid/extra_credit/semantic_analysis/decl_shadows_decl.c",
        "tests/chapter_18/valid/extra_credit/semantic_analysis/incomplete_union_types.c",
        "tests/chapter_18/valid/extra_credit/semantic_analysis/redeclare_union.c",
        "tests/chapter_18/valid/extra_credit/semantic_analysis/struct_shadows_union.c",
        "tests/chapter_18/valid/extra_credit/semantic_analysis/union_members_same_type.c",
        "tests/chapter_18/valid/extra_credit/semantic_analysis/union_namespace.c",
        "tests/chapter_18/valid/extra_credit/semantic_analysis/union_self_pointer.c",
        "tests/chapter_18/valid/extra_credit/semantic_analysis/union_shadows_struct.c",
        "tests/chapter_18/valid/extra_credit/size_and_offset/compare_union_pointers.c",
        "tests/chapter_18/valid/extra_credit/size_and_offset/union_sizes.c",
        "tests/chapter_18/valid/extra_credit/union_copy/assign_to_union.c",
        "tests/chapter_18/valid/extra_credit/union_copy/copy_non_scalar_members.c",
        "tests/chapter_18/valid/extra_credit/union_copy/copy_thru_pointer.c",
        "tests/chapter_18/valid/extra_credit/union_copy/unions_in_conditionals.c",
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
            if (status != 0)
            {
                std::cerr << "Error compiling file " << srcFile << std::endl;
            }
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// // Chapter 19
// TEST_CASE(Chapter19ValidParse, "chapter_19", "--parse")
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
//             if (status != 0)
//             {
//                 std::cerr << "Error compiling file " << srcFile << std::endl;
//             }
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// TEST_CASE(Chapter19ValidParseExtraCredit, "chapter_19", "--parse")
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
//             if (status != 0)
//             {
//                 std::cerr << "Error compiling file " << srcFile << std::endl;
//             }
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
// TEST_CASE(Chapter20ValidParse, "chapter_20", "--parse")
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
//             if (status != 0)
//             {
//                 std::cerr << "Error compiling file " << srcFile << std::endl;
//             }
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// TEST_CASE(Chapter20ValidParseExtraCredit, "chapter_20", "--parse")
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
//             if (status != 0)
//             {
//                 std::cerr << "Error compiling file " << srcFile << std::endl;
//             }
//             ASSERT_TRUE(status == 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }
