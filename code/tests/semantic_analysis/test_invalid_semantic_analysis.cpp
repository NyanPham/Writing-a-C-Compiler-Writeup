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
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
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
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter9InvalidSemantic, "chapter_9", "--validate")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter9InvalidSemanticExtraCredit, "chapter_9", "--validate")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter10InvalidSemantic, "chapter_10", "--validate")
{
    std::vector<std::string> srcFiles = {
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
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter10InvalidSemanticExtraCredit, "chapter_10", "--validate")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_10/invalid_labels/extra_credit/goto_global_var.c",
        "tests/chapter_10/invalid_types/extra_credit/static_var_case.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter11InvalidSemantic, "chapter_11", "--validate")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_11/invalid_types/call_long_as_function.c",
        "tests/chapter_11/invalid_types/cast_lvalue.c",
        "tests/chapter_11/invalid_types/conflicting_function_types.c",
        "tests/chapter_11/invalid_types/conflicting_global_types.c",
        "tests/chapter_11/invalid_types/conflicting_variable_types.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter11InvalidSemanticExtraCredit, "chapter_11", "--validate")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_11/invalid_labels/extra_credit/bitshift_duplicate_cases.c",
        "tests/chapter_11/invalid_labels/extra_credit/switch_duplicate_cases.c",
        "tests/chapter_11/invalid_labels/extra_credit/switch_duplicate_cases_2.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
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
TEST_CASE(Chapter12InvalidSemantic, "chapter_12", "--validate")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_12/invalid_types/conflicting_signed_unsigned.c",
        "tests/chapter_12/invalid_types/conflicting_uint_ulong.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter12InvalidSemanticExtraCredit, "chapter_12", "--validate")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_12/invalid_labels/extra_credit/switch_duplicate_cases.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
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
TEST_CASE(Chapter13InvalidSemantic, "chapter_13", "--validate")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_13/invalid_types/complement_double.c",
        "tests/chapter_13/invalid_types/mod_double.c",
        "tests/chapter_13/invalid_types/mod_double_2.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter13InvalidSemanticExtraCredit, "chapter_13", "--validate")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
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
TEST_CASE(Chapter14InvalidSemantic, "chapter_14", "--validate")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter14InvalidSemanticExtraCredit, "chapter_14", "--validate")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
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
TEST_CASE(Chapter15InvalidSemantic, "chapter_15", "--validate")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

TEST_CASE(Chapter15InvalidSemanticExtraCredit, "chapter_15", "--validate")
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
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Validate, std::vector<std::string>{srcFile});
            ASSERT_TRUE(status != 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// // Chapter 16
// TEST_CASE(Chapter16InvalidSemantic, "chapter_16", "--validate")
// {
//     std::vector<std::string> srcFiles = {
//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
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

// TEST_CASE(Chapter16InvalidSemanticExtraCredit, "chapter_16", "--validate")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
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

// // Chapter 17
// TEST_CASE(Chapter17InvalidSemantic, "chapter_17", "--validate")
// {
//     std::vector<std::string> srcFiles = {
//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
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

// TEST_CASE(Chapter17InvalidSemanticExtraCredit, "chapter_17", "--validate")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
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

// // Chapter 18
// TEST_CASE(Chapter18InvalidSemantic, "chapter_18", "--validate")
// {
//     std::vector<std::string> srcFiles = {
//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
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

// TEST_CASE(Chapter18InvalidSemanticExtraCredit, "chapter_18", "--validate")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
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

// // Chapter 19
// TEST_CASE(Chapter19InvalidSemantic, "chapter_19", "--validate")
// {
//     std::vector<std::string> srcFiles = {
//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
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
//         Compiler compiler;
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
//     {
//         Compiler compiler;
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

// TEST_CASE(Chapter20InvalidSemanticExtraCredit, "chapter_20", "--validate")
// {
//     std::vector<std::string> srcFiles = {

//     };
//     Settings settings;

//     for (const auto &srcFile : srcFiles)
//     {
//         Compiler compiler;
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