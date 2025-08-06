#include "../TestFramework.h"
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>

// Use these globals from test_compiler.cpp
extern bool g_redirectOutput;
extern std::string g_redirectToFile;
extern bool g_printProcess;

// Helper to run compiler.exe on a source file with --validate and expect failure
inline void typecheck_run_compiler_on_invalid(const std::string &srcFile)
{
    std::string cmd = "..\\bin\\compiler.exe " + srcFile + " --validate";
    if (!g_redirectToFile.empty())
    {
        cmd += " >" + g_redirectToFile + " 2>&1";
    }
    else if (g_redirectOutput)
    {
        cmd += " >nul 2>&1";
    }
    int result = std::system(cmd.c_str());
    if (result == 0)
    {
        std::cerr << "Expected error compiling file " << srcFile << std::endl;
    }
    ASSERT_TRUE(result != 0);
}

inline void print_compile_process(const char *stage, const char *validity, const std::string &srcFile)
{
    if (g_printProcess)
    {
        std::cout << stage << " " << validity << " compiler on: " << srcFile << '\n';
    }
}

TEST_CASE(Chapter5InvalidSemantic, "chapter_5", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_5/invalid_semantics/declared_after_use.c",
        "../tests/chapter_5/invalid_semantics/invalid_lvalue.c",
        "../tests/chapter_5/invalid_semantics/invalid_lvalue_2.c",
        "../tests/chapter_5/invalid_semantics/mixed_precedence_assignment.c",
        "../tests/chapter_5/invalid_semantics/redefine.c",
        "../tests/chapter_5/invalid_semantics/undeclared_var.c",
        "../tests/chapter_5/invalid_semantics/undeclared_var_and.c",
        "../tests/chapter_5/invalid_semantics/undeclared_var_compare.c",
        "../tests/chapter_5/invalid_semantics/undeclared_var_unary.c",
        "../tests/chapter_5/invalid_semantics/use_then_redefine.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter5InvalidSemanticExtraCredit, "chapter_5", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_5/invalid_semantics/extra_credit/compound_invalid_lvalue.c",
        "../tests/chapter_5/invalid_semantics/extra_credit/compound_invalid_lvalue_2.c",
        "../tests/chapter_5/invalid_semantics/extra_credit/postfix_decr_non_lvalue.c",
        "../tests/chapter_5/invalid_semantics/extra_credit/postfix_incr_non_lvalue.c",
        "../tests/chapter_5/invalid_semantics/extra_credit/prefix_decr_non_lvalue.c",
        "../tests/chapter_5/invalid_semantics/extra_credit/prefix_incr_non_lvalue.c",
        "../tests/chapter_5/invalid_semantics/extra_credit/undeclared_bitwise_op.c",
        "../tests/chapter_5/invalid_semantics/extra_credit/undeclared_compound_assignment.c",
        "../tests/chapter_5/invalid_semantics/extra_credit/undeclared_compound_assignment_use.c",
        "../tests/chapter_5/invalid_semantics/extra_credit/undeclared_postfix_decr.c",
        "../tests/chapter_5/invalid_semantics/extra_credit/undeclared_prefix_incr.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter6InvalidSemantic, "chapter_6", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_6/invalid_semantics/invalid_var_in_if.c",
        "../tests/chapter_6/invalid_semantics/ternary_assign.c",
        "../tests/chapter_6/invalid_semantics/undeclared_var_in_ternary.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter6InvalidSemanticExtraCredit, "chapter_6", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_6/invalid_semantics/extra_credit/duplicate_labels.c",
        "../tests/chapter_6/invalid_semantics/extra_credit/goto_missing_label.c",
        "../tests/chapter_6/invalid_semantics/extra_credit/goto_variable.c",
        "../tests/chapter_6/invalid_semantics/extra_credit/undeclared_var_in_labeled_statement.c",
        "../tests/chapter_6/invalid_semantics/extra_credit/use_label_as_variable.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter7InvalidSemantic, "chapter_7", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_7/invalid_semantics/double_define.c",
        "../tests/chapter_7/invalid_semantics/double_define_after_scope.c",
        "../tests/chapter_7/invalid_semantics/out_of_scope.c",
        "../tests/chapter_7/invalid_semantics/use_before_declare.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter7InvalidSemanticExtraCredit, "chapter_7", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_7/invalid_semantics/extra_credit/different_labels_same_scope.c",
        "../tests/chapter_7/invalid_semantics/extra_credit/duplicate_labels_different_scopes.c",
        "../tests/chapter_7/invalid_semantics/extra_credit/goto_use_before_declare.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter8InvalidSemantic, "chapter_8", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_8/invalid_semantics/break_not_in_loop.c",
        "../tests/chapter_8/invalid_semantics/continue_not_in_loop.c",
        "../tests/chapter_8/invalid_semantics/out_of_scope_do_loop.c",
        "../tests/chapter_8/invalid_semantics/out_of_scope_loop_variable.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter8InvalidSemanticExtraCredit, "chapter_8", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_8/invalid_semantics/extra_credit/case_continue.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/case_outside_switch.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/default_continue.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/default_outside_switch.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/different_cases_same_scope.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/duplicate_case.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/duplicate_case_in_labeled_switch.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/duplicate_case_in_nested_statement.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/duplicate_default.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/duplicate_default_in_nested_statement.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/duplicate_label_in_default.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/duplicate_label_in_loop.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/duplicate_variable_in_switch.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/labeled_break_outside_loop.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/non_constant_case.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/switch_continue.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/undeclared_variable_in_case.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/undeclared_variable_in_default.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/undeclared_var_switch_expression.c",
        "../tests/chapter_8/invalid_semantics/extra_credit/undefined_label_in_case.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter9InvalidSemantic, "chapter_9", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_9/invalid_declarations/assign_to_fun_call.c",
        "../tests/chapter_9/invalid_declarations/decl_params_with_same_name.c",
        "../tests/chapter_9/invalid_declarations/nested_function_definition.c",
        "../tests/chapter_9/invalid_declarations/params_with_same_name.c",
        "../tests/chapter_9/invalid_declarations/redefine_fun_as_var.c",
        "../tests/chapter_9/invalid_declarations/redefine_parameter.c",
        "../tests/chapter_9/invalid_declarations/redefine_var_as_fun.c",
        "../tests/chapter_9/invalid_declarations/undeclared_fun.c",
        "../tests/chapter_9/invalid_declarations/wrong_parameter_names.c",

        "../tests/chapter_9/invalid_types/assign_fun_to_variable.c",
        "../tests/chapter_9/invalid_types/assign_value_to_function.c",
        "../tests/chapter_9/invalid_types/call_variable_as_function.c",
        "../tests/chapter_9/invalid_types/conflicting_function_declarations.c",
        "../tests/chapter_9/invalid_types/conflicting_local_function_declaration.c",
        "../tests/chapter_9/invalid_types/divide_by_function.c",
        "../tests/chapter_9/invalid_types/multiple_function_definitions.c",
        "../tests/chapter_9/invalid_types/multiple_function_definitions_2.c",
        "../tests/chapter_9/invalid_types/too_few_args.c",
        "../tests/chapter_9/invalid_types/too_many_args.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter9InvalidSemanticExtraCredit, "chapter_9", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_9/invalid_declarations/extra_credit/call_label_as_function.c",
        "../tests/chapter_9/invalid_declarations/extra_credit/compound_assign_to_fun_call.c",
        "../tests/chapter_9/invalid_declarations/extra_credit/decrement_fun_call.c",
        "../tests/chapter_9/invalid_declarations/extra_credit/increment_fun_call.c",

        "../tests/chapter_9/invalid_labels/extra_credit/goto_cross_function.c",
        "../tests/chapter_9/invalid_labels/extra_credit/goto_function.c",

        "../tests/chapter_9/invalid_types/extra_credit/bitwise_op_function.c",
        "../tests/chapter_9/invalid_types/extra_credit/compound_assign_function_lhs.c",
        "../tests/chapter_9/invalid_types/extra_credit/compound_assign_function_rhs.c",
        "../tests/chapter_9/invalid_types/extra_credit/postfix_incr_fun_name.c",
        "../tests/chapter_9/invalid_types/extra_credit/prefix_decr_fun_name.c",
        "../tests/chapter_9/invalid_types/extra_credit/switch_on_function.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter10InvalidSemantic, "chapter_10", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_10/invalid_declarations/conflicting_local_declarations.c",
        "../tests/chapter_10/invalid_declarations/extern_follows_local_var.c",
        "../tests/chapter_10/invalid_declarations/extern_follows_static_local_var.c",
        "../tests/chapter_10/invalid_declarations/local_var_follows_extern.c",
        "../tests/chapter_10/invalid_declarations/out_of_scope_extern_var.c",
        "../tests/chapter_10/invalid_declarations/redefine_param_as_identifier_with_linkage.c",
        "../tests/chapter_10/invalid_declarations/undeclared_global_variable.c",
        "../tests/chapter_10/invalid_types/conflicting_function_linkage.c",
        "../tests/chapter_10/invalid_types/conflicting_function_linkage_2.c",
        "../tests/chapter_10/invalid_types/conflicting_global_definitions.c",
        "../tests/chapter_10/invalid_types/conflicting_variable_linkage.c",
        "../tests/chapter_10/invalid_types/conflicting_variable_linkage_2.c",
        "../tests/chapter_10/invalid_types/extern_for_loop_counter.c",
        "../tests/chapter_10/invalid_types/extern_variable_initializer.c",
        "../tests/chapter_10/invalid_types/non_constant_static_initializer.c",
        "../tests/chapter_10/invalid_types/non_constant_static_local_initializer.c",
        "../tests/chapter_10/invalid_types/redeclare_file_scope_var_as_fun.c",
        "../tests/chapter_10/invalid_types/redeclare_fun_as_file_scope_var.c",
        "../tests/chapter_10/invalid_types/redeclare_fun_as_var.c",
        "../tests/chapter_10/invalid_types/static_block_scope_function_declaration.c",
        "../tests/chapter_10/invalid_types/static_for_loop_counter.c",
        "../tests/chapter_10/invalid_types/use_file_scope_variable_as_fun.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter10InvalidSemanticExtraCredit, "chapter_10", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_10/invalid_labels/extra_credit/goto_global_var.c",
        "../tests/chapter_10/invalid_types/extra_credit/static_var_case.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter11InvalidSemantic, "chapter_11", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_11/invalid_types/call_long_as_function.c",
        "../tests/chapter_11/invalid_types/cast_lvalue.c",
        "../tests/chapter_11/invalid_types/conflicting_function_types.c",
        "../tests/chapter_11/invalid_types/conflicting_global_types.c",
        "../tests/chapter_11/invalid_types/conflicting_variable_types.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter11InvalidSemanticExtraCredit, "chapter_11", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_11/invalid_labels/extra_credit/bitshift_duplicate_cases.c",
        "../tests/chapter_11/invalid_labels/extra_credit/switch_duplicate_cases.c",
        "../tests/chapter_11/invalid_labels/extra_credit/switch_duplicate_cases_2.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

// Chapter 12
TEST_CASE(Chapter12InvalidSemantic, "chapter_12", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_12/invalid_types/conflicting_signed_unsigned.c",
        "../tests/chapter_12/invalid_types/conflicting_uint_ulong.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter12InvalidSemanticExtraCredit, "chapter_12", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_12/invalid_labels/extra_credit/switch_duplicate_cases.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

// Chapter 13
TEST_CASE(Chapter13InvalidSemantic, "chapter_13", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_13/invalid_types/complement_double.c",
        "../tests/chapter_13/invalid_types/mod_double.c",
        "../tests/chapter_13/invalid_types/mod_double_2.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter13InvalidSemanticExtraCredit, "chapter_13", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_13/invalid_types/extra_credit/bitwise_and.c",
        "../tests/chapter_13/invalid_types/extra_credit/bitwise_or.c",
        "../tests/chapter_13/invalid_types/extra_credit/bitwise_shift_double.c",
        "../tests/chapter_13/invalid_types/extra_credit/bitwise_shift_double_2.c",
        "../tests/chapter_13/invalid_types/extra_credit/bitwise_xor.c",
        "../tests/chapter_13/invalid_types/extra_credit/compound_bitwise_and.c",
        "../tests/chapter_13/invalid_types/extra_credit/compound_bitwise_xor.c",
        "../tests/chapter_13/invalid_types/extra_credit/compound_left_bitshift.c",
        "../tests/chapter_13/invalid_types/extra_credit/compound_mod.c",
        "../tests/chapter_13/invalid_types/extra_credit/compound_mod_2.c",
        "../tests/chapter_13/invalid_types/extra_credit/compound_right_bitshift.c",
        "../tests/chapter_13/invalid_types/extra_credit/switch_double_case.c",
        "../tests/chapter_13/invalid_types/extra_credit/switch_on_double.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

// Chapter 14
TEST_CASE(Chapter14InvalidSemantic, "chapter_14", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_14/invalid_types/address_of_address.c",
        "../tests/chapter_14/invalid_types/address_of_assignment.c",
        "../tests/chapter_14/invalid_types/address_of_constant.c",
        "../tests/chapter_14/invalid_types/address_of_ternary.c",
        "../tests/chapter_14/invalid_types/assign_int_to_pointer.c",
        "../tests/chapter_14/invalid_types/assign_int_var_to_pointer.c",
        "../tests/chapter_14/invalid_types/assign_to_address.c",
        "../tests/chapter_14/invalid_types/assign_wrong_pointer_type.c",
        "../tests/chapter_14/invalid_types/bad_null_pointer_constant.c",
        "../tests/chapter_14/invalid_types/cast_double_to_pointer.c",
        "../tests/chapter_14/invalid_types/cast_pointer_to_double.c",
        "../tests/chapter_14/invalid_types/compare_mixed_pointer_types.c",
        "../tests/chapter_14/invalid_types/compare_pointer_to_ulong.c",
        "../tests/chapter_14/invalid_types/complement_pointer.c",
        "../tests/chapter_14/invalid_types/dereference_non_pointer.c",
        "../tests/chapter_14/invalid_types/divide_pointer.c",
        "../tests/chapter_14/invalid_types/invalid_pointer_initializer.c",
        "../tests/chapter_14/invalid_types/invalid_static_initializer.c",
        "../tests/chapter_14/invalid_types/multiply_pointers.c",
        "../tests/chapter_14/invalid_types/multiply_pointers_2.c",
        "../tests/chapter_14/invalid_types/negate_pointer.c",
        "../tests/chapter_14/invalid_types/pass_pointer_as_int.c",
        "../tests/chapter_14/invalid_types/return_wrong_pointer_type.c",
        "../tests/chapter_14/invalid_types/ternary_mixed_pointer_types.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter14InvalidSemanticExtraCredit, "chapter_14", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_14/invalid_types/extra_credit/bitwise_and_pointer.c",
        "../tests/chapter_14/invalid_types/extra_credit/bitwise_compound_assign_to_pointer.c",
        "../tests/chapter_14/invalid_types/extra_credit/bitwise_compound_assign_with_pointer.c",
        "../tests/chapter_14/invalid_types/extra_credit/bitwise_lshift_pointer.c",
        "../tests/chapter_14/invalid_types/extra_credit/bitwise_or_pointer.c",
        "../tests/chapter_14/invalid_types/extra_credit/bitwise_rshift_pointer.c",
        "../tests/chapter_14/invalid_types/extra_credit/bitwise_xor_pointer.c",
        "../tests/chapter_14/invalid_types/extra_credit/compound_assignment_not_lval.c",
        "../tests/chapter_14/invalid_types/extra_credit/compound_assign_thru_ptr_not_lval.c",
        "../tests/chapter_14/invalid_types/extra_credit/compound_divide_pointer.c",
        "../tests/chapter_14/invalid_types/extra_credit/compound_mod_pointer.c",
        "../tests/chapter_14/invalid_types/extra_credit/compound_multiply_pointer.c",
        "../tests/chapter_14/invalid_types/extra_credit/postfix_decr_not_lvalue.c",
        "../tests/chapter_14/invalid_types/extra_credit/prefix_incr_not_lvalue.c",
        "../tests/chapter_14/invalid_types/extra_credit/switch_on_pointer.c",

        "../tests/chapter_14/invalid_declarations/extra_credit/addr_of_label.c",
        "../tests/chapter_14/invalid_declarations/extra_credit/deref_label.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

// Chapter 15
TEST_CASE(Chapter15InvalidSemantic, "chapter_15", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_15/invalid_types/add_two_pointers.c",
        "../tests/chapter_15/invalid_types/assign_incompatible_pointer_types.c",
        "../tests/chapter_15/invalid_types/assign_to_array.c",
        "../tests/chapter_15/invalid_types/assign_to_array_2.c",
        "../tests/chapter_15/invalid_types/assign_to_array_3.c",
        "../tests/chapter_15/invalid_types/bad_arg_type.c",
        "../tests/chapter_15/invalid_types/cast_to_array_type.c",
        "../tests/chapter_15/invalid_types/cast_to_array_type_2.c",
        "../tests/chapter_15/invalid_types/cast_to_array_type_3.c",
        "../tests/chapter_15/invalid_types/compare_different_pointer_types.c",
        "../tests/chapter_15/invalid_types/compare_explicit_and_implict_addr.c",
        "../tests/chapter_15/invalid_types/compare_pointer_to_int.c",
        "../tests/chapter_15/invalid_types/compare_pointer_to_zero.c",
        "../tests/chapter_15/invalid_types/compound_initializer_for_scalar.c",
        "../tests/chapter_15/invalid_types/compound_initializer_for_static_scalar.c",
        "../tests/chapter_15/invalid_types/compound_initializer_too_long_static.c",
        "../tests/chapter_15/invalid_types/compound_inititializer_too_long.c",
        "../tests/chapter_15/invalid_types/conflicting_array_declarations.c",
        "../tests/chapter_15/invalid_types/conflicting_function_declarations.c",
        "../tests/chapter_15/invalid_types/double_subscript.c",
        "../tests/chapter_15/invalid_types/function_returns_array.c",
        "../tests/chapter_15/invalid_types/incompatible_elem_type_compound_init.c",
        "../tests/chapter_15/invalid_types/incompatible_elem_type_static_compound_init.c",
        "../tests/chapter_15/invalid_types/null_ptr_array_initializer.c",
        "../tests/chapter_15/invalid_types/null_ptr_static_array_initializer.c",
        "../tests/chapter_15/invalid_types/scalar_initializer_for_array.c",
        "../tests/chapter_15/invalid_types/scalar_initializer_for_static_array.c",
        "../tests/chapter_15/invalid_types/static_non_const_array.c",
        "../tests/chapter_15/invalid_types/subscript_both_pointers.c",
        "../tests/chapter_15/invalid_types/subscript_non_ptr.c",
        "../tests/chapter_15/invalid_types/sub_different_pointer_types.c",
        "../tests/chapter_15/invalid_types/sub_double_from_ptr.c",
        "../tests/chapter_15/invalid_types/sub_ptr_from_int.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter15InvalidSemanticExtraCredit, "chapter_15", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_15/invalid_types/extra_credit/compound_add_double_to_pointer.c",
        "../tests/chapter_15/invalid_types/extra_credit/compound_add_two_pointers.c",
        "../tests/chapter_15/invalid_types/extra_credit/compound_assign_to_array.c",
        "../tests/chapter_15/invalid_types/extra_credit/compound_assign_to_nested_array.c",
        "../tests/chapter_15/invalid_types/extra_credit/compound_sub_pointer_from_int.c",
        "../tests/chapter_15/invalid_types/extra_credit/postfix_incr_array.c",
        "../tests/chapter_15/invalid_types/extra_credit/postfix_incr_nested_array.c",
        "../tests/chapter_15/invalid_types/extra_credit/prefix_decr_array.c",
        "../tests/chapter_15/invalid_types/extra_credit/prefix_decr_nested_array.c",
        "../tests/chapter_15/invalid_types/extra_credit/switch_on_array.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

// Chapter 16
TEST_CASE(Chapter16InvalidSemantic, "chapter_16", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_16/invalid_types/assign_to_string_literal.c",
        "../tests/chapter_16/invalid_types/char_and_schar_conflict.c",
        "../tests/chapter_16/invalid_types/char_and_uchar_conflict.c",
        "../tests/chapter_16/invalid_types/compound_initializer_for_pointer.c",
        "../tests/chapter_16/invalid_types/implicit_conversion_between_char_pointers.c",
        "../tests/chapter_16/invalid_types/implicit_conversion_pointers_to_different_size_arrays.c",
        "../tests/chapter_16/invalid_types/negate_char_pointer.c",
        "../tests/chapter_16/invalid_types/string_initializer_for_multidim_array.c",
        "../tests/chapter_16/invalid_types/string_initializer_too_long.c",
        "../tests/chapter_16/invalid_types/string_initializer_too_long_nested.c",
        "../tests/chapter_16/invalid_types/string_initializer_too_long_nested_static.c",
        "../tests/chapter_16/invalid_types/string_initializer_too_long_static.c",
        "../tests/chapter_16/invalid_types/string_initializer_wrong_type.c",
        "../tests/chapter_16/invalid_types/string_initializer_wrong_type_nested.c",
        "../tests/chapter_16/invalid_types/string_initializer_wrong_type_nested_static.c",
        "../tests/chapter_16/invalid_types/string_literal_is_plain_char_pointer.c",
        "../tests/chapter_16/invalid_types/string_literal_is_plain_char_pointer_static.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter16InvalidSemanticExtraCredit, "chapter_16", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_16/invalid_types/extra_credit/bitwise_operation_on_string.c",
        "../tests/chapter_16/invalid_types/extra_credit/bit_shift_string.c",
        "../tests/chapter_16/invalid_types/extra_credit/case_statement_string.c",
        "../tests/chapter_16/invalid_types/extra_credit/compound_assign_from_string.c",
        "../tests/chapter_16/invalid_types/extra_credit/compound_assign_to_string.c",
        "../tests/chapter_16/invalid_types/extra_credit/postfix_incr_string.c",
        "../tests/chapter_16/invalid_types/extra_credit/prefix_incr_string.c",
        "../tests/chapter_16/invalid_types/extra_credit/switch_on_string.c",
        "../tests/chapter_16/invalid_labels/extra_credit/duplicate_case_char_const.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

// Chapter 17
TEST_CASE(Chapter17InvalidSemantic, "chapter_17", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_17/invalid_types/incomplete_types/add_void_pointer.c",
        "../tests/chapter_17/invalid_types/incomplete_types/sizeof_function.c",
        "../tests/chapter_17/invalid_types/incomplete_types/sizeof_void.c",
        "../tests/chapter_17/invalid_types/incomplete_types/sizeof_void_array.c",
        "../tests/chapter_17/invalid_types/incomplete_types/sizeof_void_expression.c",
        "../tests/chapter_17/invalid_types/incomplete_types/subscript_void.c",
        "../tests/chapter_17/invalid_types/incomplete_types/subscript_void_pointer_conditional.c",
        "../tests/chapter_17/invalid_types/incomplete_types/sub_void_pointer.c",
        "../tests/chapter_17/invalid_types/incomplete_types/void_array.c",
        "../tests/chapter_17/invalid_types/incomplete_types/void_array_in_cast.c",
        "../tests/chapter_17/invalid_types/incomplete_types/void_array_in_param_type.c",
        "../tests/chapter_17/invalid_types/incomplete_types/void_array_nested_in_declaration.c",
        "../tests/chapter_17/invalid_types/incomplete_types/void_array_pointer_in_declaration.c",
        "../tests/chapter_17/invalid_types/incomplete_types/void_array_pointer_in_param_type.c",
        "../tests/chapter_17/invalid_types/pointer_conversions/compare_void_ptr_to_int.c",
        "../tests/chapter_17/invalid_types/pointer_conversions/compare_void_to_other_pointer.c",
        "../tests/chapter_17/invalid_types/pointer_conversions/convert_ulong_to_void_ptr.c",
        "../tests/chapter_17/invalid_types/pointer_conversions/convert_void_ptr_to_int.c",
        "../tests/chapter_17/invalid_types/pointer_conversions/usual_arithmetic_conversions_ptr.c",
        "../tests/chapter_17/invalid_types/scalar_expressions/and_void.c",
        "../tests/chapter_17/invalid_types/scalar_expressions/cast_void.c",
        "../tests/chapter_17/invalid_types/scalar_expressions/not_void.c",
        "../tests/chapter_17/invalid_types/scalar_expressions/or_void.c",
        "../tests/chapter_17/invalid_types/scalar_expressions/void_condition_do_loop.c",
        "../tests/chapter_17/invalid_types/scalar_expressions/void_condition_for_loop.c",
        "../tests/chapter_17/invalid_types/scalar_expressions/void_condition_while_loop.c",
        "../tests/chapter_17/invalid_types/scalar_expressions/void_if_condition.c",
        "../tests/chapter_17/invalid_types/scalar_expressions/void_ternary_condition.c",
        "../tests/chapter_17/invalid_types/void/assign_to_void_lvalue.c",
        "../tests/chapter_17/invalid_types/void/assign_to_void_var.c",
        "../tests/chapter_17/invalid_types/void/assign_void_rval.c",
        "../tests/chapter_17/invalid_types/void/define_void.c",
        "../tests/chapter_17/invalid_types/void/initialized_void.c",
        "../tests/chapter_17/invalid_types/void/mismatched_conditional.c",
        "../tests/chapter_17/invalid_types/void/negate_void.c",
        "../tests/chapter_17/invalid_types/void/non_void_return.c",
        "../tests/chapter_17/invalid_types/void/no_return_value.c",
        "../tests/chapter_17/invalid_types/void/return_void_as_pointer.c",
        "../tests/chapter_17/invalid_types/void/subscript_void.c",
        "../tests/chapter_17/invalid_types/void/void_compare.c",
        "../tests/chapter_17/invalid_types/void/void_equality.c",
        "../tests/chapter_17/invalid_types/void/void_fun_params.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter17InvalidSemanticExtraCredit, "chapter_17", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_17/invalid_types/extra_credit/bitshift_void.c",
        "../tests/chapter_17/invalid_types/extra_credit/bitwise_void.c",
        "../tests/chapter_17/invalid_types/extra_credit/compound_add_void_pointer.c",
        "../tests/chapter_17/invalid_types/extra_credit/compound_sub_void_pointer.c",
        "../tests/chapter_17/invalid_types/extra_credit/compound_void_rval.c",
        "../tests/chapter_17/invalid_types/extra_credit/compound_void_rval_add.c",
        "../tests/chapter_17/invalid_types/extra_credit/compound_void_rval_bitshift.c",
        "../tests/chapter_17/invalid_types/extra_credit/postfix_decr_void.c",
        "../tests/chapter_17/invalid_types/extra_credit/postfix_decr_void_pointer.c",
        "../tests/chapter_17/invalid_types/extra_credit/postfix_incr_void_pointer.c",
        "../tests/chapter_17/invalid_types/extra_credit/prefix_decr_void_pointer.c",
        "../tests/chapter_17/invalid_types/extra_credit/prefix_incr_void.c",
        "../tests/chapter_17/invalid_types/extra_credit/prefix_incr_void_pointer.c",
        "../tests/chapter_17/invalid_types/extra_credit/switch_void.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

// Chapter 18
TEST_CASE(Chapter18InvalidSemantic, "chapter_18", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_18/invalid_struct_tags/array_of_undeclared.c",
        "../tests/chapter_18/invalid_struct_tags/cast_undeclared.c",
        "../tests/chapter_18/invalid_struct_tags/deref_undeclared.c",
        "../tests/chapter_18/invalid_struct_tags/file_scope_var_type_undeclared.c",
        "../tests/chapter_18/invalid_struct_tags/for_loop_scope.c",
        "../tests/chapter_18/invalid_struct_tags/for_loop_scope_2.c",
        "../tests/chapter_18/invalid_struct_tags/member_type_undeclared.c",
        "../tests/chapter_18/invalid_struct_tags/param_undeclared.c",
        "../tests/chapter_18/invalid_struct_tags/return_type_undeclared.c",
        "../tests/chapter_18/invalid_struct_tags/sizeof_undeclared.c",
        "../tests/chapter_18/invalid_struct_tags/var_type_undeclared.c",

        "../tests/chapter_18/invalid_types/incompatible_types/assign_different_pointer_type.c",
        "../tests/chapter_18/invalid_types/incompatible_types/assign_different_struct_type.c",
        "../tests/chapter_18/invalid_types/incompatible_types/branch_mismatch.c",
        "../tests/chapter_18/invalid_types/incompatible_types/branch_mismatch_2.c",
        "../tests/chapter_18/invalid_types/incompatible_types/compare_different_struct_pointers.c",
        "../tests/chapter_18/invalid_types/incompatible_types/return_wrong_struct_type.c",
        "../tests/chapter_18/invalid_types/incompatible_types/struct_param_mismatch.c",
        "../tests/chapter_18/invalid_types/incompatible_types/struct_pointer_param_mismatch.c",
        "../tests/chapter_18/invalid_types/initializers/compound_initializer_too_long.c",
        "../tests/chapter_18/invalid_types/initializers/initialize_nested_static_struct_member_wrong_type.c",
        "../tests/chapter_18/invalid_types/initializers/initialize_static_struct_with_zero.c",
        "../tests/chapter_18/invalid_types/initializers/initialize_struct_member_wrong_type.c",
        "../tests/chapter_18/invalid_types/initializers/initialize_struct_with_scalar.c",
        "../tests/chapter_18/invalid_types/initializers/initialize_struct_wrong_type.c",
        "../tests/chapter_18/invalid_types/initializers/init_struct_with_string.c",
        "../tests/chapter_18/invalid_types/initializers/nested_compound_initializer_too_long.c",
        "../tests/chapter_18/invalid_types/initializers/nested_static_compound_initializer_too_long.c",
        "../tests/chapter_18/invalid_types/initializers/nested_struct_initializer_wrong_type.c",
        "../tests/chapter_18/invalid_types/initializers/non_constant_static_elem_init.c",
        "../tests/chapter_18/invalid_types/initializers/non_constant_static_init.c",
        "../tests/chapter_18/invalid_types/initializers/static_initializer_too_long.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/assign_to_incomplete_var.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/cast_incomplete_struct.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/deref_incomplete_struct_pointer.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_arg_funcall.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_array_element.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_local_var.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_param.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_ptr_addition.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_ptr_subtraction.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_return_type_funcall.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_return_type_fun_def.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_struct_conditional.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_struct_full_expr.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_struct_member.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_subscript.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/incomplete_tentative_def.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/initialize_incomplete.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/sizeof_incomplete.c",
        "../tests/chapter_18/invalid_types/invalid_incomplete_structs/sizeof_incomplete_expr.c",
        "../tests/chapter_18/invalid_types/invalid_lvalues/address_of_non_lvalue.c",
        "../tests/chapter_18/invalid_types/invalid_lvalues/assign_nested_non_lvalue.c",
        "../tests/chapter_18/invalid_types/invalid_lvalues/assign_to_array.c",
        "../tests/chapter_18/invalid_types/invalid_lvalues/assign_to_non_lvalue.c",
        "../tests/chapter_18/invalid_types/invalid_member_operators/arrow_pointer_to_non_struct.c",
        "../tests/chapter_18/invalid_types/invalid_member_operators/bad_member.c",
        "../tests/chapter_18/invalid_types/invalid_member_operators/bad_pointer_member.c",
        "../tests/chapter_18/invalid_types/invalid_member_operators/member_of_non_struct.c",
        "../tests/chapter_18/invalid_types/invalid_member_operators/member_pointer_non_struct_pointer.c",
        "../tests/chapter_18/invalid_types/invalid_member_operators/nested_arrow_pointer_to_non_struct.c",
        "../tests/chapter_18/invalid_types/invalid_member_operators/postfix_precedence.c",
        "../tests/chapter_18/invalid_types/invalid_struct_declaration/duplicate_member_name.c",
        "../tests/chapter_18/invalid_types/invalid_struct_declaration/duplicate_struct_declaration.c",
        "../tests/chapter_18/invalid_types/invalid_struct_declaration/incomplete_member.c",
        "../tests/chapter_18/invalid_types/invalid_struct_declaration/invalid_array_member.c",
        "../tests/chapter_18/invalid_types/invalid_struct_declaration/invalid_self_reference.c",
        "../tests/chapter_18/invalid_types/invalid_struct_declaration/void_member.c",
        "../tests/chapter_18/invalid_types/scalar_required/and_struct.c",
        "../tests/chapter_18/invalid_types/scalar_required/assign_null_ptr_to_struct.c",
        "../tests/chapter_18/invalid_types/scalar_required/assign_scalar_to_struct.c",
        "../tests/chapter_18/invalid_types/scalar_required/cast_struct_to_scalar.c",
        "../tests/chapter_18/invalid_types/scalar_required/cast_to_struct.c",
        "../tests/chapter_18/invalid_types/scalar_required/compare_structs.c",
        "../tests/chapter_18/invalid_types/scalar_required/not_struct.c",
        "../tests/chapter_18/invalid_types/scalar_required/pass_struct_as_scalar_param.c",
        "../tests/chapter_18/invalid_types/scalar_required/struct_as_int.c",
        "../tests/chapter_18/invalid_types/scalar_required/struct_controlling_expression.c",
        "../tests/chapter_18/invalid_types/scalar_required/subscript_struct.c",
        "../tests/chapter_18/invalid_types/tag_resolution/address_of_wrong_type.c",
        "../tests/chapter_18/invalid_types/tag_resolution/conflicting_fun_param_types.c",
        "../tests/chapter_18/invalid_types/tag_resolution/conflicting_fun_ret_types.c",
        "../tests/chapter_18/invalid_types/tag_resolution/distinct_struct_types.c",
        "../tests/chapter_18/invalid_types/tag_resolution/incomplete_shadows_complete.c",
        "../tests/chapter_18/invalid_types/tag_resolution/incomplete_shadows_complete_cast.c",
        "../tests/chapter_18/invalid_types/tag_resolution/invalid_shadow_self_reference.c",
        "../tests/chapter_18/invalid_types/tag_resolution/member_name_wrong_scope.c",
        "../tests/chapter_18/invalid_types/tag_resolution/member_name_wrong_scope_nested.c",
        "../tests/chapter_18/invalid_types/tag_resolution/mismatched_return_type.c",
        "../tests/chapter_18/invalid_types/tag_resolution/shadowed_tag_branch_mismatch.c",
        "../tests/chapter_18/invalid_types/tag_resolution/shadow_struct.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

TEST_CASE(Chapter18InvalidSemanticExtraCredit, "chapter_18", "--validate")
{
    std::vector<std::string> srcFiles = {
        "../tests/chapter_18/invalid_struct_tags/extra_credit/sizeof_undeclared_union.c",
        "../tests/chapter_18/invalid_struct_tags/extra_credit/var_undeclared_union_type.c",

        //"../tests/chapter_18/invalid_types/extra_credit/README.md",
        "../tests/chapter_18/invalid_types/extra_credit/bad_union_member_access/nested_non_member.c",
        "../tests/chapter_18/invalid_types/extra_credit/bad_union_member_access/union_bad_member.c",
        "../tests/chapter_18/invalid_types/extra_credit/bad_union_member_access/union_bad_pointer_member.c",
        "../tests/chapter_18/invalid_types/extra_credit/incompatible_union_types/assign_different_union_type.c",
        "../tests/chapter_18/invalid_types/extra_credit/incompatible_union_types/assign_scalar_to_union.c",
        "../tests/chapter_18/invalid_types/extra_credit/incompatible_union_types/return_type_mismatch.c",
        "../tests/chapter_18/invalid_types/extra_credit/incompatible_union_types/union_branch_mismatch.c",
        "../tests/chapter_18/invalid_types/extra_credit/incompatible_union_types/union_pointer_branch_mismatch.c",
        "../tests/chapter_18/invalid_types/extra_credit/incomplete_unions/define_incomplete_union.c",
        "../tests/chapter_18/invalid_types/extra_credit/incomplete_unions/sizeof_incomplete_union_type.c",
        "../tests/chapter_18/invalid_types/extra_credit/invalid_union_lvalues/address_of_non_lvalue_union_member.c",
        "../tests/chapter_18/invalid_types/extra_credit/invalid_union_lvalues/assign_non_lvalue_union_member.c",
        "../tests/chapter_18/invalid_types/extra_credit/other_features/bitwise_op_structure.c",
        "../tests/chapter_18/invalid_types/extra_credit/other_features/compound_assign_struct_rval.c",
        "../tests/chapter_18/invalid_types/extra_credit/other_features/compound_assign_to_nested_struct.c",
        "../tests/chapter_18/invalid_types/extra_credit/other_features/compound_assign_to_struct.c",
        "../tests/chapter_18/invalid_types/extra_credit/other_features/duplicate_struct_types_after_label.c",
        "../tests/chapter_18/invalid_types/extra_credit/other_features/postfix_decr_struct_arrow.c",
        "../tests/chapter_18/invalid_types/extra_credit/other_features/postfix_incr_struct.c",
        "../tests/chapter_18/invalid_types/extra_credit/other_features/prefix_decr_struct.c",
        "../tests/chapter_18/invalid_types/extra_credit/other_features/prefix_incr_nested_struct.c",
        "../tests/chapter_18/invalid_types/extra_credit/other_features/switch_on_struct.c",
        "../tests/chapter_18/invalid_types/extra_credit/scalar_required/cast_between_unions.c",
        "../tests/chapter_18/invalid_types/extra_credit/scalar_required/cast_union_to_int.c",
        "../tests/chapter_18/invalid_types/extra_credit/scalar_required/compare_unions.c",
        "../tests/chapter_18/invalid_types/extra_credit/scalar_required/switch_on_union.c",
        "../tests/chapter_18/invalid_types/extra_credit/scalar_required/union_as_controlling_expression.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_initializers/initializer_too_long.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_initializers/nested_init_wrong_type.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_initializers/nested_union_init_too_long.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_initializers/scalar_union_initializer.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_initializers/static_aggregate_init_wrong_type.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_initializers/static_nested_init_not_const.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_initializers/static_nested_init_too_long.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_initializers/static_scalar_union_initializer.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_initializers/static_too_long.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_initializers/static_union_init_not_constant.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_initializers/static_union_init_wrong_type.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_initializers/union_init_wrong_type.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_struct_conflicts/conflicting_tag_declarations.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_struct_conflicts/conflicting_tag_decl_and_use.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_struct_conflicts/conflicting_tag_decl_and_use_self_reference.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_struct_conflicts/struct_shadowed_by_union.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_struct_conflicts/tag_decl_conflicts_with_def.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_struct_conflicts/tag_def_conflicts_with_decl.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_struct_conflicts/union_shadowed_by_incomplete_struct.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_tag_resolution/address_of_wrong_union_type.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_tag_resolution/compare_struct_and_union_ptrs.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_tag_resolution/conflicting_param_union_types.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_tag_resolution/distinct_union_types.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_tag_resolution/union_type_shadows_struct.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_tag_resolution/union_wrong_member.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_type_declarations/array_of_incomplete_union_type.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_type_declarations/duplicate_union_def.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_type_declarations/incomplete_union_member.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_type_declarations/member_name_conflicts.c",
        "../tests/chapter_18/invalid_types/extra_credit/union_type_declarations/union_self_reference.c",
    };
    for (const auto &srcFile : srcFiles)
    {
        print_compile_process("Validate", "invalid", srcFile);
        typecheck_run_compiler_on_invalid(srcFile);
    }
}

// // Chapter 19
// TEST_CASE(Chapter19InvalidSemantic, "chapter_19", "--validate")
// {
//     std::vector<std::string> srcFiles = {
//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler(settings);
//         try
//         {
//             int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
//             ASSERT_TRUE(status != 0);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
//             throw;
//         }
//     }
// }

// TEST_CASE(Chapter19InvalidSemanticExtraCredit, "chapter_19", "--validate")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler(settings);
//         try
//         {
//             int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
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
// TEST_CASE(Chapter20InvalidSemantic, "chapter_20", "--validate")
// {
//     std::vector<std::string> srcFiles = {
//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)