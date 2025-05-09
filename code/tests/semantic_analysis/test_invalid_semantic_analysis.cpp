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