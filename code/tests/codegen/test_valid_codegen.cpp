#include "../TestFramework.h"
#include "Compiler.h"
#include "Settings.h"

#include <fstream>
#include <vector>
#include <string>
#include <iostream>

TEST_CASE(Chapter2ValidCodeGen, "chapter_2", "--codegen")
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
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter3ValidCodeGen, "chapter_3", "--codegen")
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
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter3ValidCodeGenExraCredit, "chapter_3", "--codegen")
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
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter4ValidCodeGen, "chapter_4", "--codegen")
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
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter4ValidCodeGenExraCredit, "chapter_4", "--codegen")
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
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter5ValidCodeGen, "chapter_5", "--codegen")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter5ValidCodeGenExtraCredit, "chapter_5", "--codegen")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter6ValidCodeGen, "chapter_6", "--codegen")
{
    std::vector<std::string> srcFiles = {
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
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter6ValidCodeGenExtraCredit, "chapter_6", "--codegen")
{
    std::vector<std::string> srcFiles = {
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
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter7ValidCodeGen, "chapter_7", "--codegen")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter7ValidCodeGenExtraCredit, "chapter_7", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_7/valid/extra_credit/compound_subtract_in_block.c",
        "tests/chapter_7/valid/extra_credit/goto_before_declaration.c",
        "tests/chapter_7/valid/extra_credit/goto_inner_scope.c",
        "tests/chapter_7/valid/extra_credit/goto_outer_scope.c",
        "tests/chapter_7/valid/extra_credit/goto_sibling_scope.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter8ValidCodeGen, "chapter_8", "--codegen")
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

    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter8ValidCodeGenExtraCredit, "chapter_8", "--codegen")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter9ValidCodeGen, "chapter_9", "--codegen")
{
    std::vector<std::string> srcFiles = {
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
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter9ValidCodeGenExtraCredit, "chapter_9", "--codegen")
{
    std::vector<std::string> srcFiles = {
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
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter10ValidCodeGen, "chapter_10", "--codegen")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter10ValidCodeGenExtraCredit, "chapter_10", "--codegen")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter11ValidCodeGen, "chapter_11", "--codegen")
{
    std::vector<std::string> srcFiles = {
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
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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

TEST_CASE(Chapter11ValidCodeGenExtraCredit, "chapter_11", "--codegen")
{
    std::vector<std::string> srcFiles = {
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
            int status = compiler.compile(Stage::CodeGen, std::vector<std::string>{srcFile});
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
