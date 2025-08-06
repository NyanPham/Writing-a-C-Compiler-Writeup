#include "../TestFramework.h"
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>

// Use these globals from test_compiler.cpp
extern bool g_redirectOutput;
extern std::string g_redirectToFile;
extern bool g_printProcess;

inline void print_compile_process(const char *stage, const char *validity, const std::string &srcFile)
{
    if (g_printProcess)
    {
        std::cout << stage << " " << validity << " compiler on: " << srcFile << '\n';
    }
}

// Helper to run compiler.exe on a source file with --codegen and output control
inline void codegen_run_compiler_on(const std::string &srcFile, const bool optimize = false)
{
    std::string cmd = "..\\bin\\compiler.exe " + srcFile + " --codegen" + (optimize ? " --optimize" : "");
    if (!g_redirectToFile.empty())
    {
        cmd += " >" + g_redirectToFile + " 2>&1";
    }
    else if (g_redirectOutput)
    {
        cmd += " >nul 2>&1";
    }
    int result = std::system(cmd.c_str());
    ASSERT_EQ(result, 0);
}

TEST_CASE(Chapter2ValidCodeGen, "chapter_2", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_2/valid/bitwise_int_min.c",
        "../tests/chapter_2/valid/bitwise_zero.c",
        "../tests/chapter_2/valid/bitwise.c",
        "../tests/chapter_2/valid/neg_zero.c",
        "../tests/chapter_2/valid/neg.c",
        "../tests/chapter_2/valid/negate_int_max.c",
        "../tests/chapter_2/valid/nested_ops_2.c",
        "../tests/chapter_2/valid/nested_ops.c",
        "../tests/chapter_2/valid/parens_2.c",
        "../tests/chapter_2/valid/parens_3.c",
        "../tests/chapter_2/valid/parens.c",
        "../tests/chapter_2/valid/redundant_parens.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter3ValidCodeGen, "chapter_3", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_3/valid/add.c",
        "../tests/chapter_3/valid/associativity.c",
        "../tests/chapter_3/valid/associativity_2.c",
        "../tests/chapter_3/valid/associativity_3.c",
        "../tests/chapter_3/valid/associativity_and_precedence.c",
        "../tests/chapter_3/valid/div_neg.c",
        "../tests/chapter_3/valid/div.c",
        "../tests/chapter_3/valid/mod.c",
        "../tests/chapter_3/valid/mult.c",
        "../tests/chapter_3/valid/parens.c",
        "../tests/chapter_3/valid/precedence.c",
        "../tests/chapter_3/valid/sub_neg.c",
        "../tests/chapter_3/valid/sub.c",
        "../tests/chapter_3/valid/unop_add.c",
        "../tests/chapter_3/valid/unop_parens.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter3ValidCodeGenExraCredit, "chapter_3", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_3/valid/extra_credit/bitwise_and.c",
        "../tests/chapter_3/valid/extra_credit/bitwise_or.c",
        "../tests/chapter_3/valid/extra_credit/bitwise_precedence.c",
        "../tests/chapter_3/valid/extra_credit/bitwise_shift_associativity.c",
        "../tests/chapter_3/valid/extra_credit/bitwise_shift_associativity_2.c",
        "../tests/chapter_3/valid/extra_credit/bitwise_shift_precedence.c",
        "../tests/chapter_3/valid/extra_credit/bitwise_shiftl.c",
        "../tests/chapter_3/valid/extra_credit/bitwise_shiftr.c",
        "../tests/chapter_3/valid/extra_credit/bitwise_shiftr_negative.c",
        "../tests/chapter_3/valid/extra_credit/bitwise_variable_shift_count.c",
        "../tests/chapter_3/valid/extra_credit/bitwise_xor.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter4ValidCodeGen, "chapter_4", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_4/valid/and_false.c",
        "../tests/chapter_4/valid/and_short_circuit.c",
        "../tests/chapter_4/valid/and_true.c",
        "../tests/chapter_4/valid/associativity.c",
        "../tests/chapter_4/valid/compare_arithmetic_results.c",
        "../tests/chapter_4/valid/eq_false.c",
        "../tests/chapter_4/valid/eq_precedence.c",
        "../tests/chapter_4/valid/eq_true.c",
        "../tests/chapter_4/valid/ge_false.c",
        "../tests/chapter_4/valid/ge_true.c",
        "../tests/chapter_4/valid/gt_false.c",
        "../tests/chapter_4/valid/gt_true.c",
        "../tests/chapter_4/valid/le_false.c",
        "../tests/chapter_4/valid/le_true.c",
        "../tests/chapter_4/valid/lt_false.c",
        "../tests/chapter_4/valid/lt_true.c",
        "../tests/chapter_4/valid/multi_short_circuit.c",
        "../tests/chapter_4/valid/ne_false.c",
        "../tests/chapter_4/valid/ne_true.c",
        "../tests/chapter_4/valid/nested_ops.c",
        "../tests/chapter_4/valid/not_sum_2.c",
        "../tests/chapter_4/valid/not_sum.c",
        "../tests/chapter_4/valid/not_zero.c",
        "../tests/chapter_4/valid/not.c",
        "../tests/chapter_4/valid/operate_on_booleans.c",
        "../tests/chapter_4/valid/or_false.c",
        "../tests/chapter_4/valid/or_short_circuit.c",
        "../tests/chapter_4/valid/or_true.c",
        "../tests/chapter_4/valid/precedence_2.c",
        "../tests/chapter_4/valid/precedence_3.c",
        "../tests/chapter_4/valid/precedence_4.c",
        "../tests/chapter_4/valid/precedence_5.c",
        "../tests/chapter_4/valid/precedence.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter4ValidCodeGenExraCredit, "chapter_4", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_4/valid/extra_credit/bitwise_and_precedence.c",
        "../tests/chapter_4/valid/extra_credit/bitwise_or_precedence.c",
        "../tests/chapter_4/valid/extra_credit/bitwise_shift_precedence.c",
        "../tests/chapter_4/valid/extra_credit/bitwise_xor_precedence.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter5ValidCodeGen, "chapter_5", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_5/valid/add_variables.c",
        "../tests/chapter_5/valid/allocate_temps_and_vars.c",
        "../tests/chapter_5/valid/assign.c",
        "../tests/chapter_5/valid/assignment_in_initializer.c",
        "../tests/chapter_5/valid/assignment_lowest_precedence.c",
        "../tests/chapter_5/valid/assign_val_in_initializer.c",
        "../tests/chapter_5/valid/empty_function_body.c",
        "../tests/chapter_5/valid/exp_then_declaration.c",
        "../tests/chapter_5/valid/local_var_missing_return.c",
        "../tests/chapter_5/valid/mixed_precedence_assignment.c",
        "../tests/chapter_5/valid/non_short_circuit_or.c",
        "../tests/chapter_5/valid/null_statement.c",
        "../tests/chapter_5/valid/null_then_return.c",
        "../tests/chapter_5/valid/return_var.c",
        "../tests/chapter_5/valid/short_circuit_and_fail.c",
        "../tests/chapter_5/valid/short_circuit_or.c",
        "../tests/chapter_5/valid/unused_exp.c",
        "../tests/chapter_5/valid/use_assignment_result.c",
        "../tests/chapter_5/valid/use_val_in_own_initializer.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter5ValidCodeGenExtraCredit, "chapter_5", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_5/valid/extra_credit/bitwise_in_initializer.c",
        "../tests/chapter_5/valid/extra_credit/bitwise_ops_vars.c",
        "../tests/chapter_5/valid/extra_credit/bitwise_shiftl_variable.c",
        "../tests/chapter_5/valid/extra_credit/bitwise_shiftr_assign.c",
        "../tests/chapter_5/valid/extra_credit/compound_assignment_chained.c",
        "../tests/chapter_5/valid/extra_credit/compound_assignment_lowest_precedence.c",
        "../tests/chapter_5/valid/extra_credit/compound_assignment_use_result.c",
        "../tests/chapter_5/valid/extra_credit/compound_bitwise_and.c",
        "../tests/chapter_5/valid/extra_credit/compound_bitwise_assignment_lowest_precedence.c",
        "../tests/chapter_5/valid/extra_credit/compound_bitwise_chained.c",
        "../tests/chapter_5/valid/extra_credit/compound_bitwise_or.c",
        "../tests/chapter_5/valid/extra_credit/compound_bitwise_shiftl.c",
        "../tests/chapter_5/valid/extra_credit/compound_bitwise_shiftr.c",
        "../tests/chapter_5/valid/extra_credit/compound_bitwise_xor.c",
        "../tests/chapter_5/valid/extra_credit/compound_divide.c",
        "../tests/chapter_5/valid/extra_credit/compound_minus.c",
        "../tests/chapter_5/valid/extra_credit/compound_mod.c",
        "../tests/chapter_5/valid/extra_credit/compound_multiply.c",
        "../tests/chapter_5/valid/extra_credit/compound_plus.c",
        "../tests/chapter_5/valid/extra_credit/incr_expression_statement.c",
        "../tests/chapter_5/valid/extra_credit/incr_in_binary_expr.c",
        "../tests/chapter_5/valid/extra_credit/incr_parenthesized.c",
        "../tests/chapter_5/valid/extra_credit/postfix_incr_and_decr.c",
        "../tests/chapter_5/valid/extra_credit/postfix_precedence.c",
        "../tests/chapter_5/valid/extra_credit/prefix_incr_and_decr.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter6ValidCodeGen, "chapter_6", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_6/valid/assign_ternary.c",
        "../tests/chapter_6/valid/binary_condition.c",
        "../tests/chapter_6/valid/binary_false_condition.c",
        "../tests/chapter_6/valid/else.c",
        "../tests/chapter_6/valid/if_nested.c",
        "../tests/chapter_6/valid/if_nested_2.c",
        "../tests/chapter_6/valid/if_nested_3.c",
        "../tests/chapter_6/valid/if_nested_4.c",
        "../tests/chapter_6/valid/if_nested_5.c",
        "../tests/chapter_6/valid/if_not_taken.c",
        "../tests/chapter_6/valid/if_null_body.c",
        "../tests/chapter_6/valid/if_taken.c",
        "../tests/chapter_6/valid/lh_assignment.c",
        "../tests/chapter_6/valid/multiple_if.c",
        "../tests/chapter_6/valid/nested_ternary.c",
        "../tests/chapter_6/valid/nested_ternary_2.c",
        "../tests/chapter_6/valid/rh_assignment.c",
        "../tests/chapter_6/valid/ternary.c",
        "../tests/chapter_6/valid/ternary_middle_assignment.c",
        "../tests/chapter_6/valid/ternary_middle_binop.c",
        "../tests/chapter_6/valid/ternary_precedence.c",
        "../tests/chapter_6/valid/ternary_rh_binop.c",
        "../tests/chapter_6/valid/ternary_short_circuit.c",
        "../tests/chapter_6/valid/ternary_short_circuit_2.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter6ValidCodeGenExtraCredit, "chapter_6", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_6/valid/extra_credit/bitwise_ternary.c",
        "../tests/chapter_6/valid/extra_credit/compound_assign_ternary.c",
        "../tests/chapter_6/valid/extra_credit/compound_if_expression.c",
        "../tests/chapter_6/valid/extra_credit/goto_after_declaration.c",
        "../tests/chapter_6/valid/extra_credit/goto_backwards.c",
        "../tests/chapter_6/valid/extra_credit/goto_label.c",
        "../tests/chapter_6/valid/extra_credit/goto_label_and_var.c",
        "../tests/chapter_6/valid/extra_credit/goto_label_main.c",
        "../tests/chapter_6/valid/extra_credit/goto_label_main_2.c",
        "../tests/chapter_6/valid/extra_credit/goto_nested_label.c",
        "../tests/chapter_6/valid/extra_credit/label_all_statements.c",
        "../tests/chapter_6/valid/extra_credit/label_token.c",
        "../tests/chapter_6/valid/extra_credit/lh_compound_assignment.c",
        "../tests/chapter_6/valid/extra_credit/postfix_if.c",
        "../tests/chapter_6/valid/extra_credit/postfix_in_ternary.c",
        "../tests/chapter_6/valid/extra_credit/prefix_if.c",
        "../tests/chapter_6/valid/extra_credit/prefix_in_ternary.c",
        "../tests/chapter_6/valid/extra_credit/unused_label.c",
        "../tests/chapter_6/valid/extra_credit/whitespace_after_label.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter7ValidCodeGen, "chapter_7", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_7/valid/assign_to_self.c",
        "../tests/chapter_7/valid/assign_to_self_2.c",
        "../tests/chapter_7/valid/declaration_only.c",
        "../tests/chapter_7/valid/empty_blocks.c",
        "../tests/chapter_7/valid/hidden_then_visible.c",
        "../tests/chapter_7/valid/hidden_variable.c",
        "../tests/chapter_7/valid/inner_uninitialized.c",
        "../tests/chapter_7/valid/multiple_vars_same_name.c",
        "../tests/chapter_7/valid/nested_if.c",
        "../tests/chapter_7/valid/similar_var_names.c",
        "../tests/chapter_7/valid/use_in_inner_scope.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter7ValidCodeGenExtraCredit, "chapter_7", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_7/valid/extra_credit/compound_subtract_in_block.c",
        "../tests/chapter_7/valid/extra_credit/goto_before_declaration.c",
        "../tests/chapter_7/valid/extra_credit/goto_inner_scope.c",
        "../tests/chapter_7/valid/extra_credit/goto_outer_scope.c",
        "../tests/chapter_7/valid/extra_credit/goto_sibling_scope.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter8ValidCodeGen, "chapter_8", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_8/valid/break.c",
        "../tests/chapter_8/valid/break_immediate.c",
        "../tests/chapter_8/valid/continue.c",
        "../tests/chapter_8/valid/continue_empty_post.c",
        "../tests/chapter_8/valid/do_while.c",
        "../tests/chapter_8/valid/do_while_break_immediate.c",
        "../tests/chapter_8/valid/empty_expression.c",
        "../tests/chapter_8/valid/empty_loop_body.c",
        "../tests/chapter_8/valid/for.c",
        "../tests/chapter_8/valid/for_absent_condition.c",
        "../tests/chapter_8/valid/for_absent_post.c",
        "../tests/chapter_8/valid/for_decl.c",
        "../tests/chapter_8/valid/for_nested_shadow.c",
        "../tests/chapter_8/valid/for_shadow.c",
        "../tests/chapter_8/valid/multi_break.c",
        "../tests/chapter_8/valid/multi_continue_same_loop.c",
        "../tests/chapter_8/valid/nested_break.c",
        "../tests/chapter_8/valid/nested_continue.c",
        "../tests/chapter_8/valid/nested_loop.c",
        "../tests/chapter_8/valid/null_for_header.c",
        "../tests/chapter_8/valid/while.c",

    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter8ValidCodeGenExtraCredit, "chapter_8", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_8/valid/extra_credit/case_block.c",
        "../tests/chapter_8/valid/extra_credit/compound_assignment_controlling_expression.c",
        "../tests/chapter_8/valid/extra_credit/compound_assignment_for_loop.c",
        "../tests/chapter_8/valid/extra_credit/duffs_device.c",
        "../tests/chapter_8/valid/extra_credit/goto_bypass_condition.c",
        "../tests/chapter_8/valid/extra_credit/goto_bypass_init_exp.c",
        "../tests/chapter_8/valid/extra_credit/goto_bypass_post_exp.c",
        "../tests/chapter_8/valid/extra_credit/label_loops_breaks_and_continues.c",
        "../tests/chapter_8/valid/extra_credit/label_loop_body.c",
        "../tests/chapter_8/valid/extra_credit/loop_header_postfix_and_prefix.c",
        "../tests/chapter_8/valid/extra_credit/loop_in_switch.c",
        "../tests/chapter_8/valid/extra_credit/post_exp_incr.c",
        "../tests/chapter_8/valid/extra_credit/switch.c",
        "../tests/chapter_8/valid/extra_credit/switch_assign_in_condition.c",
        "../tests/chapter_8/valid/extra_credit/switch_break.c",
        "../tests/chapter_8/valid/extra_credit/switch_decl.c",
        "../tests/chapter_8/valid/extra_credit/switch_default.c",
        "../tests/chapter_8/valid/extra_credit/switch_default_fallthrough.c",
        "../tests/chapter_8/valid/extra_credit/switch_default_not_last.c",
        "../tests/chapter_8/valid/extra_credit/switch_default_only.c",
        "../tests/chapter_8/valid/extra_credit/switch_empty.c",
        "../tests/chapter_8/valid/extra_credit/switch_fallthrough.c",
        "../tests/chapter_8/valid/extra_credit/switch_goto_mid_case.c",
        "../tests/chapter_8/valid/extra_credit/switch_in_loop.c",
        "../tests/chapter_8/valid/extra_credit/switch_nested_cases.c",
        "../tests/chapter_8/valid/extra_credit/switch_nested_not_taken.c",
        "../tests/chapter_8/valid/extra_credit/switch_nested_switch.c",
        "../tests/chapter_8/valid/extra_credit/switch_not_taken.c",
        "../tests/chapter_8/valid/extra_credit/switch_no_case.c",
        "../tests/chapter_8/valid/extra_credit/switch_single_case.c",
        "../tests/chapter_8/valid/extra_credit/switch_with_continue.c",
        "../tests/chapter_8/valid/extra_credit/switch_with_continue_2.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter9ValidCodeGen, "chapter_9", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_9/valid/arguments_in_registers/dont_clobber_edx.c",
        "../tests/chapter_9/valid/arguments_in_registers/expression_args.c",
        "../tests/chapter_9/valid/arguments_in_registers/fibonacci.c",
        "../tests/chapter_9/valid/arguments_in_registers/forward_decl_multi_arg.c",
        "../tests/chapter_9/valid/arguments_in_registers/hello_world.c",
        "../tests/chapter_9/valid/arguments_in_registers/parameters_are_preserved.c",
        "../tests/chapter_9/valid/arguments_in_registers/parameter_shadows_function.c",
        "../tests/chapter_9/valid/arguments_in_registers/parameter_shadows_own_function.c",
        "../tests/chapter_9/valid/arguments_in_registers/param_shadows_local_var.c",
        "../tests/chapter_9/valid/arguments_in_registers/single_arg.c",

        "../tests/chapter_9/valid/libraries/addition.c",
        "../tests/chapter_9/valid/libraries/addition_client.c",
        "../tests/chapter_9/valid/libraries/many_args.c",
        "../tests/chapter_9/valid/libraries/many_args_client.c",
        "../tests/chapter_9/valid/libraries/system_call.c",
        "../tests/chapter_9/valid/libraries/system_call_client.c",
        "../tests/chapter_9/valid/libraries/no_function_calls/division.c",
        "../tests/chapter_9/valid/libraries/no_function_calls/division_client.c",
        "../tests/chapter_9/valid/libraries/no_function_calls/local_stack_variables.c",
        "../tests/chapter_9/valid/libraries/no_function_calls/local_stack_variables_client.c",

        "../tests/chapter_9/valid/no_arguments/forward_decl.c",
        "../tests/chapter_9/valid/no_arguments/function_shadows_variable.c",
        "../tests/chapter_9/valid/no_arguments/multiple_declarations.c",
        "../tests/chapter_9/valid/no_arguments/no_return_value.c",
        "../tests/chapter_9/valid/no_arguments/precedence.c",
        "../tests/chapter_9/valid/no_arguments/use_function_in_expression.c",
        "../tests/chapter_9/valid/no_arguments/variable_shadows_function.c",

        "../tests/chapter_9/valid/stack_arguments/call_putchar.c",
        "../tests/chapter_9/valid/stack_arguments/lots_of_arguments.c",
        "../tests/chapter_9/valid/stack_arguments/stack_alignment.c",
        //"../tests/chapter_9/valid/stack_arguments/stack_alignment_check_linux.s",
        //"../tests/chapter_9/valid/stack_arguments/stack_alignment_check_osx.s",
        "../tests/chapter_9/valid/stack_arguments/test_for_memory_leaks.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter9ValidCodeGenExtraCredit, "chapter_9", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_9/valid/extra_credit/compound_assign_function_result.c",
        "../tests/chapter_9/valid/extra_credit/dont_clobber_ecx.c",
        "../tests/chapter_9/valid/extra_credit/goto_label_multiple_functions.c",
        "../tests/chapter_9/valid/extra_credit/goto_shared_name.c",
        "../tests/chapter_9/valid/extra_credit/label_naming_scheme.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter10ValidCodeGen, "chapter_10", "--codegen")
{
    std::vector<std::string> srcFiles = {
        //"../tests/chapter_10/valid/data_on_page_boundary_linux.s",
        //"../tests/chapter_10/valid/data_on_page_boundary_osx.s",
        "../tests/chapter_10/valid/distinct_local_and_extern.c",
        "../tests/chapter_10/valid/extern_block_scope_variable.c",
        "../tests/chapter_10/valid/multiple_static_file_scope_vars.c",
        "../tests/chapter_10/valid/multiple_static_local.c",
        "../tests/chapter_10/valid/push_arg_on_page_boundary.c",
        "../tests/chapter_10/valid/shadow_static_local_var.c",
        "../tests/chapter_10/valid/static_local_multiple_scopes.c",
        "../tests/chapter_10/valid/static_local_uninitialized.c",
        "../tests/chapter_10/valid/static_recursive_call.c",
        "../tests/chapter_10/valid/static_then_extern.c",
        "../tests/chapter_10/valid/static_variables_in_expressions.c",
        "../tests/chapter_10/valid/tentative_definition.c",
        "../tests/chapter_10/valid/type_before_storage_class.c",
        "../tests/chapter_10/valid/libraries/external_linkage_function.c",
        "../tests/chapter_10/valid/libraries/external_linkage_function_client.c",
        "../tests/chapter_10/valid/libraries/external_tentative_var.c",
        "../tests/chapter_10/valid/libraries/external_tentative_var_client.c",
        "../tests/chapter_10/valid/libraries/external_variable.c",
        "../tests/chapter_10/valid/libraries/external_variable_client.c",
        "../tests/chapter_10/valid/libraries/external_var_scoping.c",
        "../tests/chapter_10/valid/libraries/external_var_scoping_client.c",
        "../tests/chapter_10/valid/libraries/internal_hides_external_linkage.c",
        "../tests/chapter_10/valid/libraries/internal_hides_external_linkage_client.c",
        "../tests/chapter_10/valid/libraries/internal_linkage_function.c",
        "../tests/chapter_10/valid/libraries/internal_linkage_function_client.c",
        "../tests/chapter_10/valid/libraries/internal_linkage_var.c",
        "../tests/chapter_10/valid/libraries/internal_linkage_var_client.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter10ValidCodeGenExtraCredit, "chapter_10", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_10/valid/extra_credit/bitwise_ops_file_scope_vars.c",
        "../tests/chapter_10/valid/extra_credit/compound_assignment_static_var.c",
        "../tests/chapter_10/valid/extra_credit/goto_skip_static_initializer.c",
        "../tests/chapter_10/valid/extra_credit/increment_global_vars.c",
        "../tests/chapter_10/valid/extra_credit/label_file_scope_var_same_name.c",
        "../tests/chapter_10/valid/extra_credit/label_static_var_same_name.c",
        "../tests/chapter_10/valid/extra_credit/switch_on_extern.c",
        "../tests/chapter_10/valid/extra_credit/switch_skip_extern_decl.c",
        "../tests/chapter_10/valid/extra_credit/switch_skip_static_initializer.c",
        "../tests/chapter_10/valid/extra_credit/libraries/same_label_same_fun.c",
        "../tests/chapter_10/valid/extra_credit/libraries/same_label_same_fun_client.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter11ValidCodeGen, "chapter_11", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_11/valid/explicit_casts/sign_extend.c",
        "../tests/chapter_11/valid/explicit_casts/truncate.c",

        "../tests/chapter_11/valid/implicit_casts/common_type.c",
        "../tests/chapter_11/valid/implicit_casts/convert_by_assignment.c",
        "../tests/chapter_11/valid/implicit_casts/convert_function_arguments.c",
        "../tests/chapter_11/valid/implicit_casts/convert_static_initializer.c",
        "../tests/chapter_11/valid/implicit_casts/long_constants.c",

        "../tests/chapter_11/valid/libraries/long_args.c",
        "../tests/chapter_11/valid/libraries/long_args_client.c",
        "../tests/chapter_11/valid/libraries/long_global_var.c",
        "../tests/chapter_11/valid/libraries/long_global_var_client.c",
        "../tests/chapter_11/valid/libraries/maintain_stack_alignment.c",
        "../tests/chapter_11/valid/libraries/maintain_stack_alignment_client.c",
        "../tests/chapter_11/valid/libraries/return_long.c",
        "../tests/chapter_11/valid/libraries/return_long_client.c",

        "../tests/chapter_11/valid/long_expressions/arithmetic_ops.c",
        "../tests/chapter_11/valid/long_expressions/assign.c",
        "../tests/chapter_11/valid/long_expressions/comparisons.c",
        "../tests/chapter_11/valid/long_expressions/large_constants.c",
        "../tests/chapter_11/valid/long_expressions/logical.c",
        "../tests/chapter_11/valid/long_expressions/long_and_int_locals.c",
        "../tests/chapter_11/valid/long_expressions/long_args.c",
        "../tests/chapter_11/valid/long_expressions/multi_op.c",
        "../tests/chapter_11/valid/long_expressions/return_long.c",
        "../tests/chapter_11/valid/long_expressions/rewrite_large_multiply_regression.c",
        "../tests/chapter_11/valid/long_expressions/simple.c",
        "../tests/chapter_11/valid/long_expressions/static_long.c",
        "../tests/chapter_11/valid/long_expressions/type_specifiers.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter11ValidCodeGenExtraCredit, "chapter_11", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_11/valid/extra_credit/bitshift.c",
        "../tests/chapter_11/valid/extra_credit/bitwise_long_op.c",
        "../tests/chapter_11/valid/extra_credit/compound_assign_to_int.c",
        "../tests/chapter_11/valid/extra_credit/compound_assign_to_long.c",
        "../tests/chapter_11/valid/extra_credit/compound_bitshift.c",
        "../tests/chapter_11/valid/extra_credit/compound_bitwise.c",
        "../tests/chapter_11/valid/extra_credit/increment_long.c",
        "../tests/chapter_11/valid/extra_credit/switch_int.c",
        "../tests/chapter_11/valid/extra_credit/switch_long.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

// Chapter 12
TEST_CASE(Chapter12ValidCodeGen, "chapter_12", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_12/valid/explicit_casts/chained_casts.c",
        "../tests/chapter_12/valid/explicit_casts/extension.c",
        "../tests/chapter_12/valid/explicit_casts/rewrite_movz_regression.c",
        "../tests/chapter_12/valid/explicit_casts/round_trip_casts.c",
        "../tests/chapter_12/valid/explicit_casts/same_size_conversion.c",
        "../tests/chapter_12/valid/explicit_casts/truncate.c",

        "../tests/chapter_12/valid/implicit_casts/common_type.c",
        "../tests/chapter_12/valid/implicit_casts/convert_by_assignment.c",
        "../tests/chapter_12/valid/implicit_casts/promote_constants.c",
        "../tests/chapter_12/valid/implicit_casts/static_initializers.c",

        "../tests/chapter_12/valid/libraries/unsigned_args.c",
        "../tests/chapter_12/valid/libraries/unsigned_args_client.c",
        "../tests/chapter_12/valid/libraries/unsigned_global_var.c",
        "../tests/chapter_12/valid/libraries/unsigned_global_var_client.c",

        "../tests/chapter_12/valid/type_specifiers/signed_type_specifiers.c",
        "../tests/chapter_12/valid/type_specifiers/unsigned_type_specifiers.c",

        "../tests/chapter_12/valid/unsigned_expressions/arithmetic_ops.c",
        "../tests/chapter_12/valid/unsigned_expressions/arithmetic_wraparound.c",
        "../tests/chapter_12/valid/unsigned_expressions/comparisons.c",
        "../tests/chapter_12/valid/unsigned_expressions/locals.c",
        "../tests/chapter_12/valid/unsigned_expressions/logical.c",
        "../tests/chapter_12/valid/unsigned_expressions/simple.c",
        "../tests/chapter_12/valid/unsigned_expressions/static_variables.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter12ValidCodeGenExtraCredit, "chapter_12", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_12/valid/extra_credit/bitwise_unsigned_ops.c",
        "../tests/chapter_12/valid/extra_credit/bitwise_unsigned_shift.c",
        "../tests/chapter_12/valid/extra_credit/compound_assign_uint.c",
        "../tests/chapter_12/valid/extra_credit/compound_bitshift.c",
        "../tests/chapter_12/valid/extra_credit/compound_bitwise.c",
        "../tests/chapter_12/valid/extra_credit/postfix_precedence.c",
        "../tests/chapter_12/valid/extra_credit/switch_uint.c",
        "../tests/chapter_12/valid/extra_credit/unsigned_incr_decr.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

// Chapter 13
TEST_CASE(Chapter13ValidCodeGen, "chapter_13", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_13/valid/constants/constant_doubles.c",
        "../tests/chapter_13/valid/constants/round_constants.c",
        "../tests/chapter_13/valid/explicit_casts/cvttsd2si_rewrite.c",
        "../tests/chapter_13/valid/explicit_casts/double_to_signed.c",
        "../tests/chapter_13/valid/explicit_casts/double_to_unsigned.c",
        "../tests/chapter_13/valid/explicit_casts/rewrite_cvttsd2si_regression.c",
        "../tests/chapter_13/valid/explicit_casts/signed_to_double.c",
        "../tests/chapter_13/valid/explicit_casts/unsigned_to_double.c",

        "../tests/chapter_13/valid/floating_expressions/arithmetic_ops.c",
        "../tests/chapter_13/valid/floating_expressions/comparisons.c",
        "../tests/chapter_13/valid/floating_expressions/logical.c",
        "../tests/chapter_13/valid/floating_expressions/loop_controlling_expression.c",
        "../tests/chapter_13/valid/floating_expressions/simple.c",
        "../tests/chapter_13/valid/floating_expressions/static_initialized_double.c",
        "../tests/chapter_13/valid/function_calls/double_and_int_parameters.c",
        "../tests/chapter_13/valid/function_calls/double_and_int_params_recursive.c",
        "../tests/chapter_13/valid/function_calls/double_parameters.c",
        "../tests/chapter_13/valid/function_calls/push_xmm.c",
        "../tests/chapter_13/valid/function_calls/return_double.c",
        "../tests/chapter_13/valid/function_calls/standard_library_call.c",
        "../tests/chapter_13/valid/function_calls/use_arg_after_fun_call.c",
        "../tests/chapter_13/valid/implicit_casts/common_type.c",
        "../tests/chapter_13/valid/implicit_casts/complex_arithmetic_common_type.c",
        "../tests/chapter_13/valid/implicit_casts/convert_for_assignment.c",
        "../tests/chapter_13/valid/implicit_casts/static_initializers.c",
        "../tests/chapter_13/valid/libraries/double_and_int_params_recursive.c",
        "../tests/chapter_13/valid/libraries/double_and_int_params_recursive_client.c",
        "../tests/chapter_13/valid/libraries/double_parameters.c",
        "../tests/chapter_13/valid/libraries/double_parameters_client.c",
        "../tests/chapter_13/valid/libraries/double_params_and_result.c",
        "../tests/chapter_13/valid/libraries/double_params_and_result_client.c",
        "../tests/chapter_13/valid/libraries/extern_double.c",
        "../tests/chapter_13/valid/libraries/extern_double_client.c",
        "../tests/chapter_13/valid/libraries/use_arg_after_fun_call.c",
        "../tests/chapter_13/valid/libraries/use_arg_after_fun_call_client.c",
        "../tests/chapter_13/valid/special_values/infinity.c",
        "../tests/chapter_13/valid/special_values/negative_zero.c",
        "../tests/chapter_13/valid/special_values/subnormal_not_zero.c",

        //"../tests/chapter_13/helper_libs/nan.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter13ValidCodeGenExtraCredit, "chapter_13", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_13/valid/extra_credit/compound_assign.c",
        "../tests/chapter_13/valid/extra_credit/compound_assign_implicit_cast.c",
        "../tests/chapter_13/valid/extra_credit/incr_and_decr.c",
        "../tests/chapter_13/valid/extra_credit/nan.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

// Chapter 14
TEST_CASE(Chapter14ValidCodeGen, "chapter_14", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_14/valid/casts/cast_between_pointer_types.c",
        "../tests/chapter_14/valid/casts/null_pointer_conversion.c",
        "../tests/chapter_14/valid/casts/pointer_int_casts.c",
        "../tests/chapter_14/valid/comparisons/compare_pointers.c",
        "../tests/chapter_14/valid/comparisons/compare_to_null.c",
        "../tests/chapter_14/valid/comparisons/pointers_as_conditions.c",
        "../tests/chapter_14/valid/declarators/abstract_declarators.c",
        "../tests/chapter_14/valid/declarators/declarators.c",
        "../tests/chapter_14/valid/declarators/declare_pointer_in_for_loop.c",
        "../tests/chapter_14/valid/dereference/address_of_dereference.c",
        "../tests/chapter_14/valid/dereference/dereference_expression_result.c",
        "../tests/chapter_14/valid/dereference/multilevel_indirection.c",
        "../tests/chapter_14/valid/dereference/read_through_pointers.c",
        "../tests/chapter_14/valid/dereference/simple.c",
        "../tests/chapter_14/valid/dereference/static_var_indirection.c",
        "../tests/chapter_14/valid/dereference/update_through_pointers.c",

        "../tests/chapter_14/valid/function_calls/address_of_argument.c",
        "../tests/chapter_14/valid/function_calls/return_pointer.c",
        "../tests/chapter_14/valid/function_calls/update_value_through_pointer_parameter.c",
        "../tests/chapter_14/valid/libraries/global_pointer.c",
        "../tests/chapter_14/valid/libraries/global_pointer_client.c",
        "../tests/chapter_14/valid/libraries/static_pointer.c",
        "../tests/chapter_14/valid/libraries/static_pointer_client.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter14ValidCodeGenExtraCredit, "chapter_14", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_14/valid/extra_credit/bitshift_dereferenced_ptrs.c",
        "../tests/chapter_14/valid/extra_credit/bitwise_ops_with_dereferenced_ptrs.c",
        "../tests/chapter_14/valid/extra_credit/compound_assign_conversion.c",
        "../tests/chapter_14/valid/extra_credit/compound_assign_through_pointer.c",
        "../tests/chapter_14/valid/extra_credit/compound_bitwise_dereferenced_ptrs.c",
        "../tests/chapter_14/valid/extra_credit/eval_compound_lhs_once.c",
        "../tests/chapter_14/valid/extra_credit/incr_and_decr_through_pointer.c",
        "../tests/chapter_14/valid/extra_credit/switch_dereferenced_pointer.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

// Chapter 15
TEST_CASE(Chapter15ValidCodeGen, "chapter_15", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_15/valid/allocation/test_alignment.c",
        "../tests/chapter_15/valid/casts/cast_array_of_pointers.c",
        "../tests/chapter_15/valid/casts/implicit_and_explicit_conversions.c",
        "../tests/chapter_15/valid/casts/multi_dim_casts.c",
        "../tests/chapter_15/valid/declarators/array_as_argument.c",
        "../tests/chapter_15/valid/declarators/big_array.c",
        "../tests/chapter_15/valid/declarators/equivalent_declarators.c",
        "../tests/chapter_15/valid/declarators/for_loop_array.c",
        "../tests/chapter_15/valid/declarators/return_nested_array.c",

        "../tests/chapter_15/valid/initialization/automatic.c",
        "../tests/chapter_15/valid/initialization/automatic_nested.c",
        "../tests/chapter_15/valid/initialization/static.c",
        "../tests/chapter_15/valid/initialization/static_nested.c",
        "../tests/chapter_15/valid/initialization/trailing_comma_initializer.c",
        "../tests/chapter_15/valid/libraries/global_array.c",
        "../tests/chapter_15/valid/libraries/global_array_client.c",
        "../tests/chapter_15/valid/libraries/return_pointer_to_array.c",
        "../tests/chapter_15/valid/libraries/return_pointer_to_array_client.c",
        "../tests/chapter_15/valid/libraries/set_array_val.c",
        "../tests/chapter_15/valid/libraries/set_array_val_client.c",
        "../tests/chapter_15/valid/pointer_arithmetic/add_dereference_and_assign.c",
        "../tests/chapter_15/valid/pointer_arithmetic/compare.c",
        "../tests/chapter_15/valid/pointer_arithmetic/pointer_add.c",
        "../tests/chapter_15/valid/pointer_arithmetic/pointer_diff.c",
        "../tests/chapter_15/valid/subscripting/addition_subscript_equivalence.c",
        "../tests/chapter_15/valid/subscripting/array_of_pointers_to_arrays.c",
        "../tests/chapter_15/valid/subscripting/complex_operands.c",
        "../tests/chapter_15/valid/subscripting/simple.c",
        "../tests/chapter_15/valid/subscripting/simple_subscripts.c",
        "../tests/chapter_15/valid/subscripting/subscript_nested.c",
        "../tests/chapter_15/valid/subscripting/subscript_pointer.c",
        "../tests/chapter_15/valid/subscripting/subscript_precedence.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter15ValidCodeGenExtraCredit, "chapter_15", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_15/valid/extra_credit/bitwise_subscript.c",
        "../tests/chapter_15/valid/extra_credit/compound_assign_and_increment.c",
        "../tests/chapter_15/valid/extra_credit/compound_assign_array_of_pointers.c",
        "../tests/chapter_15/valid/extra_credit/compound_assign_to_nested_subscript.c",
        "../tests/chapter_15/valid/extra_credit/compound_assign_to_subscripted_val.c",
        "../tests/chapter_15/valid/extra_credit/compound_bitwise_subscript.c",
        "../tests/chapter_15/valid/extra_credit/compound_lval_evaluated_once.c",
        "../tests/chapter_15/valid/extra_credit/compound_nested_pointer_assignment.c",
        "../tests/chapter_15/valid/extra_credit/compound_pointer_assignment.c",
        "../tests/chapter_15/valid/extra_credit/incr_and_decr_nested_pointers.c",
        "../tests/chapter_15/valid/extra_credit/incr_and_decr_pointers.c",
        "../tests/chapter_15/valid/extra_credit/incr_decr_subscripted_vals.c",
        "../tests/chapter_15/valid/extra_credit/postfix_prefix_precedence.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

// Chapter 16
TEST_CASE(Chapter16ValidCodeGen, "chapter_16", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_16/valid/chars/access_through_char_pointer.c",
        "../tests/chapter_16/valid/chars/chained_casts.c",
        "../tests/chapter_16/valid/chars/char_arguments.c",
        "../tests/chapter_16/valid/chars/char_expressions.c",
        "../tests/chapter_16/valid/chars/common_type.c",
        "../tests/chapter_16/valid/chars/convert_by_assignment.c",
        // "../tests/chapter_16/valid/chars/data_on_page_boundary_linux.s",
        // "../tests/chapter_16/valid/chars/data_on_page_boundary_osx.s",
        "../tests/chapter_16/valid/chars/explicit_casts.c",
        "../tests/chapter_16/valid/chars/integer_promotion.c",
        "../tests/chapter_16/valid/chars/partial_initialization.c",
        "../tests/chapter_16/valid/chars/push_arg_on_page_boundary.c",
        "../tests/chapter_16/valid/chars/return_char.c",
        "../tests/chapter_16/valid/chars/rewrite_movz_regression.c",
        "../tests/chapter_16/valid/chars/static_initializers.c",
        "../tests/chapter_16/valid/chars/type_specifiers.c",
        "../tests/chapter_16/valid/char_constants/char_constant_operations.c",
        "../tests/chapter_16/valid/char_constants/control_characters.c",
        "../tests/chapter_16/valid/char_constants/escape_sequences.c",
        "../tests/chapter_16/valid/char_constants/return_char_constant.c",

        "../tests/chapter_16/valid/libraries/char_arguments.c",
        "../tests/chapter_16/valid/libraries/char_arguments_client.c",
        "../tests/chapter_16/valid/libraries/global_char.c",
        "../tests/chapter_16/valid/libraries/global_char_client.c",
        "../tests/chapter_16/valid/libraries/return_char.c",
        "../tests/chapter_16/valid/libraries/return_char_client.c",
        "../tests/chapter_16/valid/strings_as_initializers/adjacent_strings_in_initializer.c",
        "../tests/chapter_16/valid/strings_as_initializers/array_init_special_chars.c",
        "../tests/chapter_16/valid/strings_as_initializers/literals_and_compound_initializers.c",
        "../tests/chapter_16/valid/strings_as_initializers/partial_initialize_via_string.c",
        "../tests/chapter_16/valid/strings_as_initializers/simple.c",
        "../tests/chapter_16/valid/strings_as_initializers/terminating_null_bytes.c",
        "../tests/chapter_16/valid/strings_as_initializers/test_alignment.c",
        "../tests/chapter_16/valid/strings_as_initializers/transfer_by_eightbyte.c",
        "../tests/chapter_16/valid/strings_as_initializers/write_to_array.c",
        "../tests/chapter_16/valid/strings_as_lvalues/addr_of_string.c",
        "../tests/chapter_16/valid/strings_as_lvalues/adjacent_strings.c",
        "../tests/chapter_16/valid/strings_as_lvalues/array_of_strings.c",
        "../tests/chapter_16/valid/strings_as_lvalues/cast_string_pointer.c",
        "../tests/chapter_16/valid/strings_as_lvalues/empty_string.c",
        "../tests/chapter_16/valid/strings_as_lvalues/pointer_operations.c",
        "../tests/chapter_16/valid/strings_as_lvalues/simple.c",
        "../tests/chapter_16/valid/strings_as_lvalues/standard_library_calls.c",
        "../tests/chapter_16/valid/strings_as_lvalues/strings_in_function_calls.c",
        "../tests/chapter_16/valid/strings_as_lvalues/string_special_characters.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter16ValidCodeGenExtraCredit, "chapter_16", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_16/valid/extra_credit/bitshift_chars.c",
        "../tests/chapter_16/valid/extra_credit/bitwise_ops_character_constants.c",
        "../tests/chapter_16/valid/extra_credit/bitwise_ops_chars.c",
        "../tests/chapter_16/valid/extra_credit/char_consts_as_cases.c",
        "../tests/chapter_16/valid/extra_credit/compound_assign_chars.c",
        "../tests/chapter_16/valid/extra_credit/compound_bitwise_ops_chars.c",
        "../tests/chapter_16/valid/extra_credit/incr_decr_chars.c",
        "../tests/chapter_16/valid/extra_credit/incr_decr_unsigned_chars.c",
        "../tests/chapter_16/valid/extra_credit/promote_switch_cond.c",
        "../tests/chapter_16/valid/extra_credit/promote_switch_cond_2.c",
        "../tests/chapter_16/valid/extra_credit/switch_on_char_const.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

// Chapter 17
TEST_CASE(Chapter17ValidCodeGen, "chapter_17", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_17/valid/libraries/pass_alloced_memory.c",
        "../tests/chapter_17/valid/libraries/pass_alloced_memory_client.c",
        "../tests/chapter_17/valid/libraries/sizeof_extern.c",
        "../tests/chapter_17/valid/libraries/sizeof_extern_client.c",
        "../tests/chapter_17/valid/libraries/test_for_memory_leaks.c",
        "../tests/chapter_17/valid/libraries/test_for_memory_leaks_client.c",
        "../tests/chapter_17/valid/sizeof/simple.c",
        "../tests/chapter_17/valid/sizeof/sizeof_array.c",
        "../tests/chapter_17/valid/sizeof/sizeof_basic_types.c",
        "../tests/chapter_17/valid/sizeof/sizeof_consts.c",
        "../tests/chapter_17/valid/sizeof/sizeof_derived_types.c",
        "../tests/chapter_17/valid/sizeof/sizeof_expressions.c",
        "../tests/chapter_17/valid/sizeof/sizeof_not_evaluated.c",
        "../tests/chapter_17/valid/sizeof/sizeof_result_is_ulong.c",
        "../tests/chapter_17/valid/void/cast_to_void.c",
        "../tests/chapter_17/valid/void/ternary.c",
        "../tests/chapter_17/valid/void/void_for_loop.c",
        "../tests/chapter_17/valid/void/void_function.c",
        "../tests/chapter_17/valid/void_pointer/array_of_pointers_to_void.c",
        "../tests/chapter_17/valid/void_pointer/common_pointer_type.c",
        "../tests/chapter_17/valid/void_pointer/conversion_by_assignment.c",
        "../tests/chapter_17/valid/void_pointer/explicit_cast.c",
        "../tests/chapter_17/valid/void_pointer/memory_management_functions.c",
        "../tests/chapter_17/valid/void_pointer/simple.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter17ValidCodeGenExtraCredit, "chapter_17", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_17/valid/extra_credit/sizeof_bitwise.c",
        "../tests/chapter_17/valid/extra_credit/sizeof_compound.c",
        "../tests/chapter_17/valid/extra_credit/sizeof_compound_bitwise.c",
        "../tests/chapter_17/valid/extra_credit/sizeof_incr.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

// Chapter 18
TEST_CASE(Chapter18ValidCodeGen, "chapter_18", "--codegen")
{
    std::vector<std::string> srcFiles = {
        //"../tests/chapter_18/valid/no_structure_parameters/README.md",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/array_of_structs.c",
        // "../tests/chapter_18/valid/no_structure_parameters/libraries/array_of_structs.h",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/array_of_structs_client.c",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/global_struct.c",
        // "../tests/chapter_18/valid/no_structure_parameters/libraries/global_struct.h",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/global_struct_client.c",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/opaque_struct.c",
        // "../tests/chapter_18/valid/no_structure_parameters/libraries/opaque_struct_client.c",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/param_struct_pointer.c",
        // "../tests/chapter_18/valid/no_structure_parameters/libraries/param_struct_pointer.h",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/param_struct_pointer_client.c",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/return_struct_pointer.c",
        // "../tests/chapter_18/valid/no_structure_parameters/libraries/return_struct_pointer.h",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/return_struct_pointer_client.c",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/initializers/auto_struct_initializers.c",
        // "../tests/chapter_18/valid/no_structure_parameters/libraries/initializers/auto_struct_initializers.h",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/initializers/auto_struct_initializers_client.c",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/initializers/nested_auto_struct_initializers.c",
        // "../tests/chapter_18/valid/no_structure_parameters/libraries/initializers/nested_auto_struct_initializers.h",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/initializers/nested_auto_struct_initializers_client.c",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/initializers/nested_static_struct_initializers.c",
        // "../tests/chapter_18/valid/no_structure_parameters/libraries/initializers/nested_static_struct_initializers.h",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/initializers/nested_static_struct_initializers_client.c",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/initializers/static_struct_initializers.c",
        // "../tests/chapter_18/valid/no_structure_parameters/libraries/initializers/static_struct_initializers.h",
        "../tests/chapter_18/valid/no_structure_parameters/libraries/initializers/static_struct_initializers_client.c",
        "../tests/chapter_18/valid/no_structure_parameters/parse_and_lex/postfix_precedence.c",
        "../tests/chapter_18/valid/no_structure_parameters/parse_and_lex/space_around_struct_member.c",
        "../tests/chapter_18/valid/no_structure_parameters/parse_and_lex/struct_member_looks_like_const.c",
        "../tests/chapter_18/valid/no_structure_parameters/parse_and_lex/trailing_comma.c",
        "../tests/chapter_18/valid/no_structure_parameters/scalar_member_access/arrow.c",
        "../tests/chapter_18/valid/no_structure_parameters/scalar_member_access/dot.c",
        "../tests/chapter_18/valid/no_structure_parameters/scalar_member_access/linked_list.c",
        "../tests/chapter_18/valid/no_structure_parameters/scalar_member_access/nested_struct.c",
        "../tests/chapter_18/valid/no_structure_parameters/scalar_member_access/static_structs.c",
        "../tests/chapter_18/valid/no_structure_parameters/semantic_analysis/cast_struct_to_void.c",
        "../tests/chapter_18/valid/no_structure_parameters/semantic_analysis/incomplete_structs.c",
        "../tests/chapter_18/valid/no_structure_parameters/semantic_analysis/namespaces.c",
        "../tests/chapter_18/valid/no_structure_parameters/semantic_analysis/resolve_tags.c",
        "../tests/chapter_18/valid/no_structure_parameters/size_and_offset_calculations/member_comparisons.c",
        "../tests/chapter_18/valid/no_structure_parameters/size_and_offset_calculations/member_offsets.c",
        "../tests/chapter_18/valid/no_structure_parameters/size_and_offset_calculations/sizeof_exps.c",
        "../tests/chapter_18/valid/no_structure_parameters/size_and_offset_calculations/sizeof_type.c",
        // "../tests/chapter_18/valid/no_structure_parameters/size_and_offset_calculations/struct_sizes.h",
        "../tests/chapter_18/valid/no_structure_parameters/smoke_tests/simple.c",
        "../tests/chapter_18/valid/no_structure_parameters/smoke_tests/static_vs_auto.c",
        "../tests/chapter_18/valid/no_structure_parameters/struct_copy/copy_struct.c",
        "../tests/chapter_18/valid/no_structure_parameters/struct_copy/copy_struct_through_pointer.c",
        "../tests/chapter_18/valid/no_structure_parameters/struct_copy/copy_struct_with_arrow_operator.c",
        "../tests/chapter_18/valid/no_structure_parameters/struct_copy/copy_struct_with_dot_operator.c",
        "../tests/chapter_18/valid/no_structure_parameters/struct_copy/stack_clobber.c",
        // "../tests/chapter_18/valid/no_structure_parameters/struct_copy/structs.h",
        //"../tests/chapter_18/valid/parameters/data_on_page_boundary_linux.s",
        //"../tests/chapter_18/valid/parameters/data_on_page_boundary_osx.s",
        "../tests/chapter_18/valid/parameters/incomplete_param_type.c",
        "../tests/chapter_18/valid/parameters/pass_args_on_page_boundary.c",
        "../tests/chapter_18/valid/parameters/simple.c",
        "../tests/chapter_18/valid/parameters/stack_clobber.c",
        "../tests/chapter_18/valid/parameters/libraries/classify_params.c",
        // "../tests/chapter_18/valid/parameters/libraries/classify_params.h",
        "../tests/chapter_18/valid/parameters/libraries/classify_params_client.c",
        "../tests/chapter_18/valid/parameters/libraries/modify_param.c",
        // "../tests/chapter_18/valid/parameters/libraries/modify_param.h",
        "../tests/chapter_18/valid/parameters/libraries/modify_param_client.c",
        "../tests/chapter_18/valid/parameters/libraries/param_calling_conventions.c",
        // "../tests/chapter_18/valid/parameters/libraries/param_calling_conventions.h",
        "../tests/chapter_18/valid/parameters/libraries/param_calling_conventions_client.c",
        "../tests/chapter_18/valid/parameters/libraries/pass_struct.c",
        // "../tests/chapter_18/valid/parameters/libraries/pass_struct.h",
        "../tests/chapter_18/valid/parameters/libraries/pass_struct_client.c",
        "../tests/chapter_18/valid/parameters/libraries/struct_sizes.c",
        // "../tests/chapter_18/valid/parameters/libraries/struct_sizes.h",
        "../tests/chapter_18/valid/parameters/libraries/struct_sizes_client.c",
        //"../tests/chapter_18/valid/params_and_returns/big_data_on_page_boundary_linux.s",
        //"../tests/chapter_18/valid/params_and_returns/big_data_on_page_boundary_osx.s",
        //"../tests/chapter_18/valid/params_and_returns/data_on_page_boundary_linux.s",
        //"../tests/chapter_18/valid/params_and_returns/data_on_page_boundary_osx.s",
        "../tests/chapter_18/valid/params_and_returns/ignore_retval.c",
        "../tests/chapter_18/valid/params_and_returns/return_big_struct_on_page_boundary.c",
        "../tests/chapter_18/valid/params_and_returns/return_incomplete_type.c",
        //"../tests/chapter_18/valid/params_and_returns/return_space_address_overlap_linux.s",
        //"../tests/chapter_18/valid/params_and_returns/return_space_address_overlap_osx.s",
        "../tests/chapter_18/valid/params_and_returns/return_space_overlap.c",
        "../tests/chapter_18/valid/params_and_returns/return_struct_on_page_boundary.c",
        "../tests/chapter_18/valid/params_and_returns/simple.c",
        "../tests/chapter_18/valid/params_and_returns/stack_clobber.c",
        "../tests/chapter_18/valid/params_and_returns/temporary_lifetime.c",
        "../tests/chapter_18/valid/params_and_returns/libraries/access_retval_members.c",
        // "../tests/chapter_18/valid/params_and_returns/libraries/access_retval_members.h",
        "../tests/chapter_18/valid/params_and_returns/libraries/access_retval_members_client.c",
        "../tests/chapter_18/valid/params_and_returns/libraries/missing_retval.c",
        // "../tests/chapter_18/valid/params_and_returns/libraries/missing_retval.h",
        "../tests/chapter_18/valid/params_and_returns/libraries/missing_retval_client.c",
        "../tests/chapter_18/valid/params_and_returns/libraries/return_calling_conventions.c",
        // "../tests/chapter_18/valid/params_and_returns/libraries/return_calling_conventions.h",
        "../tests/chapter_18/valid/params_and_returns/libraries/return_calling_conventions_client.c",
        "../tests/chapter_18/valid/params_and_returns/libraries/retval_struct_sizes.c",
        // "../tests/chapter_18/valid/params_and_returns/libraries/retval_struct_sizes.h",
        "../tests/chapter_18/valid/params_and_returns/libraries/retval_struct_sizes_client.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

TEST_CASE(Chapter18ValidCodeGenExtraCredit, "chapter_18", "--codegen")
{
    std::vector<std::string> srcFiles = {
        // "../tests/chapter_18/valid/extra_credit/README.md",
        // "../tests/chapter_18/valid/extra_credit/union_types.h",
        "../tests/chapter_18/valid/extra_credit/libraries/classify_unions.c",
        "../tests/chapter_18/valid/extra_credit/libraries/classify_unions_client.c",
        "../tests/chapter_18/valid/extra_credit/libraries/param_passing.c",
        "../tests/chapter_18/valid/extra_credit/libraries/param_passing_client.c",
        "../tests/chapter_18/valid/extra_credit/libraries/static_union_inits.c",
        // "../tests/chapter_18/valid/extra_credit/libraries/static_union_inits.h",
        "../tests/chapter_18/valid/extra_credit/libraries/static_union_inits_client.c",
        "../tests/chapter_18/valid/extra_credit/libraries/union_inits.c",
        // "../tests/chapter_18/valid/extra_credit/libraries/union_inits.h",
        "../tests/chapter_18/valid/extra_credit/libraries/union_inits_client.c",
        // "../tests/chapter_18/valid/extra_credit/libraries/union_lib.h",
        "../tests/chapter_18/valid/extra_credit/libraries/union_retvals.c",
        "../tests/chapter_18/valid/extra_credit/libraries/union_retvals_client.c",
        "../tests/chapter_18/valid/extra_credit/member_access/nested_union_access.c",
        "../tests/chapter_18/valid/extra_credit/member_access/static_union_access.c",
        "../tests/chapter_18/valid/extra_credit/member_access/union_init_and_member_access.c",
        "../tests/chapter_18/valid/extra_credit/member_access/union_temp_lifetime.c",
        "../tests/chapter_18/valid/extra_credit/other_features/bitwise_ops_struct_members.c",
        "../tests/chapter_18/valid/extra_credit/other_features/compound_assign_struct_members.c",
        "../tests/chapter_18/valid/extra_credit/other_features/decr_arrow_lexing.c",
        "../tests/chapter_18/valid/extra_credit/other_features/incr_struct_members.c",
        "../tests/chapter_18/valid/extra_credit/other_features/label_tag_member_namespace.c",
        "../tests/chapter_18/valid/extra_credit/other_features/struct_decl_in_switch_statement.c",
        "../tests/chapter_18/valid/extra_credit/semantic_analysis/cast_union_to_void.c",
        "../tests/chapter_18/valid/extra_credit/semantic_analysis/decl_shadows_decl.c",
        "../tests/chapter_18/valid/extra_credit/semantic_analysis/incomplete_union_types.c",
        "../tests/chapter_18/valid/extra_credit/semantic_analysis/redeclare_union.c",
        "../tests/chapter_18/valid/extra_credit/semantic_analysis/struct_shadows_union.c",
        "../tests/chapter_18/valid/extra_credit/semantic_analysis/union_members_same_type.c",
        "../tests/chapter_18/valid/extra_credit/semantic_analysis/union_namespace.c",
        "../tests/chapter_18/valid/extra_credit/semantic_analysis/union_self_pointer.c",
        "../tests/chapter_18/valid/extra_credit/semantic_analysis/union_shadows_struct.c",
        "../tests/chapter_18/valid/extra_credit/size_and_offset/compare_union_pointers.c",
        "../tests/chapter_18/valid/extra_credit/size_and_offset/union_sizes.c",
        "../tests/chapter_18/valid/extra_credit/union_copy/assign_to_union.c",
        "../tests/chapter_18/valid/extra_credit/union_copy/copy_non_scalar_members.c",
        "../tests/chapter_18/valid/extra_credit/union_copy/copy_thru_pointer.c",
        "../tests/chapter_18/valid/extra_credit/union_copy/unions_in_conditionals.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile);
    }
}

// Chapter 20
TEST_CASE(RegAllocWithoutCoalescing, "chapter_20", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_20/all_types/no_coalescing/dbl_fun_call.c",
        "../tests/chapter_20/all_types/no_coalescing/dbl_trivially_colorable.c",
        "../tests/chapter_20/all_types/no_coalescing/div_interference.c",
        "../tests/chapter_20/all_types/no_coalescing/force_spill_doubles.c",
        "../tests/chapter_20/all_types/no_coalescing/force_spill_mixed_ints.c",
        "../tests/chapter_20/all_types/no_coalescing/fourteen_pseudos_interfere.c",
        "../tests/chapter_20/all_types/no_coalescing/mixed_type_stack_alignment.c",
        "../tests/chapter_20/all_types/no_coalescing/store_pointer_in_register.c",
        "../tests/chapter_20/all_types/no_coalescing/track_dbl_arg_registers.c",
        "../tests/chapter_20/int_only/no_coalescing/many_pseudos_fewer_conflicts.c",
        "../tests/chapter_20/int_only/no_coalescing/bin_uses_operands.c",
        "../tests/chapter_20/int_only/no_coalescing/callee_saved_stack_alignment.c",
        "../tests/chapter_20/int_only/no_coalescing/cdq_interference.c",
        "../tests/chapter_20/int_only/no_coalescing/cmp_generates_operands.c",
        "../tests/chapter_20/int_only/no_coalescing/cmp_no_updates.c",
        "../tests/chapter_20/int_only/no_coalescing/copy_no_interference.c",
        "../tests/chapter_20/int_only/no_coalescing/division_uses_ax.c",
        "../tests/chapter_20/int_only/no_coalescing/eax_live_at_exit.c",
        "../tests/chapter_20/int_only/no_coalescing/force_spill.c",
        "../tests/chapter_20/int_only/no_coalescing/funcall_generates_args.c",
        "../tests/chapter_20/int_only/no_coalescing/idiv_interference.c",
        "../tests/chapter_20/int_only/no_coalescing/loop.c",
        "../tests/chapter_20/int_only/no_coalescing/optimistic_coloring.c",
        "../tests/chapter_20/int_only/no_coalescing/preserve_across_fun_call.c",
        "../tests/chapter_20/int_only/no_coalescing/rewrite_regression_test.c",
        "../tests/chapter_20/int_only/no_coalescing/same_instr_interference.c",
        "../tests/chapter_20/int_only/no_coalescing/same_instr_no_interference.c",
        "../tests/chapter_20/int_only/no_coalescing/test_spill_metric.c",
        "../tests/chapter_20/int_only/no_coalescing/test_spill_metric_2.c",
        "../tests/chapter_20/int_only/no_coalescing/track_arg_registers.c",
        "../tests/chapter_20/int_only/no_coalescing/trivially_colorable.c",
        "../tests/chapter_20/int_only/no_coalescing/unary_interference.c",
        "../tests/chapter_20/int_only/no_coalescing/unary_uses_operand.c",
        "../tests/chapter_20/int_only/no_coalescing/use_all_hardregs.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile, true);
    }
}

TEST_CASE(RegAllocWithCoalescing, "chapter_20", "--codegen")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_20/int_only/with_coalescing/briggs_coalesce.c",
        "../tests/chapter_20/int_only/with_coalescing/briggs_coalesce_hardreg.c",
        "../tests/chapter_20/int_only/with_coalescing/briggs_dont_coalesce.c",
        "../tests/chapter_20/int_only/with_coalescing/coalesce_prevents_spill.c",
        "../tests/chapter_20/int_only/with_coalescing/george_coalesce.c",
        "../tests/chapter_20/int_only/with_coalescing/george_dont_coalesce.c",
        "../tests/chapter_20/int_only/with_coalescing/george_dont_coalesce_2.c",
        "../tests/chapter_20/int_only/with_coalescing/george_off_by_one.c",
        "../tests/chapter_20/int_only/with_coalescing/no_george_test_for_pseudos.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Codegen", "valid", srcFile);
        codegen_run_compiler_on(srcFile, true);
    }
}
