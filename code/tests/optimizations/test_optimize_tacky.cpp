#include "../TestFramework.h"
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>

// Use these globals from test_compiler.cpp
extern bool g_redirectOutput;
extern std::string g_redirectToFile;

// Helper to run compiler.exe on a source file with the given optimization flag
inline void run_tacky_optimization(const std::string& srcFile, const std::string& optFlag) {
    std::string cmd = "..\\bin\\compiler.exe " + srcFile + " --tacky --" + optFlag;
    if (!g_redirectToFile.empty()) {
        cmd += " >" + g_redirectToFile + " 2>&1";
    } else if (g_redirectOutput) {
        cmd += " >nul 2>&1";
    }
    int result = std::system(cmd.c_str());
    if (result != 0) {
        std::cerr << "Error optimizing file " << srcFile << " with --" << optFlag << std::endl;
    }
    ASSERT_TRUE(result == 0);
}

// ===================== CONSTANT FOLDING =====================

TEST_CASE(ConstantFolding_AllTypes, "chapter_19", "constant_folding")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/constant_folding/all_types/fold_cast_from_double.c",
        "../tests/chapter_19/constant_folding/all_types/fold_cast_to_double.c",
        "../tests/chapter_19/constant_folding/all_types/fold_conditional_jump.c",
        "../tests/chapter_19/constant_folding/all_types/fold_double.c",
        "../tests/chapter_19/constant_folding/all_types/fold_double_cast_exception.c",
        "../tests/chapter_19/constant_folding/all_types/fold_extensions_and_copies.c",
        "../tests/chapter_19/constant_folding/all_types/fold_long.c",
        "../tests/chapter_19/constant_folding/all_types/fold_truncate.c",
        "../tests/chapter_19/constant_folding/all_types/fold_uint.c",
        "../tests/chapter_19/constant_folding/all_types/fold_ulong.c",
        "../tests/chapter_19/constant_folding/all_types/negative_zero.c"
    };
    for (const auto& file : files) {
        run_tacky_optimization(file, "constant_folding");
    }
}

TEST_CASE(ConstantFolding_AllTypes_ExtraCredit, "chapter_19", "constant_folding_extra_credit")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/constant_folding/all_types/extra_credit/cast_nan_not_executed.c",
        "../tests/chapter_19/constant_folding/all_types/extra_credit/fold_bitwise_long.c",
        "../tests/chapter_19/constant_folding/all_types/extra_credit/fold_bitwise_unsigned.c",
        "../tests/chapter_19/constant_folding/all_types/extra_credit/fold_nan.c",
        "../tests/chapter_19/constant_folding/all_types/extra_credit/return_nan.c"
    };
    for (const auto& file : files) {
        run_tacky_optimization(file, "constant_folding");
    }
}

TEST_CASE(ConstantFolding_IntOnly, "chapter_19", "constant_folding_int_only")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/constant_folding/int_only/fold_binary.c",
        "../tests/chapter_19/constant_folding/int_only/fold_conditional_jump.c",
        "../tests/chapter_19/constant_folding/int_only/fold_control_flow.c",
        "../tests/chapter_19/constant_folding/int_only/fold_exception.c",
        "../tests/chapter_19/constant_folding/int_only/fold_unary.c"
    };
    for (const auto& file : files) {
        run_tacky_optimization(file, "constant_folding");
    }
}

TEST_CASE(ConstantFolding_IntOnly_ExtraCredit, "chapter_19", "constant_folding_int_only_extra_credit")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/constant_folding/int_only/extra_credit/fold_bitwise.c"
    };
    for (const auto& file : files) {
        run_tacky_optimization(file, "constant_folding");
    }
}

// ===================== COPY PROPAGATION =====================

TEST_CASE(CopyPropagation_AllTypes, "chapter_19", "copy_propagation")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/copy_propagation/all_types/alias_analysis.c",
        "../tests/chapter_19/copy_propagation/all_types/char_type_conversion.c",
        "../tests/chapter_19/copy_propagation/all_types/copy_struct.c",
        "../tests/chapter_19/copy_propagation/all_types/funcall_kills_aliased.c",
        "../tests/chapter_19/copy_propagation/all_types/pointer_arithmetic.c",
        "../tests/chapter_19/copy_propagation/all_types/propagate_all_types.c",
        "../tests/chapter_19/copy_propagation/all_types/propagate_into_type_conversions.c",
        "../tests/chapter_19/copy_propagation/all_types/propagate_null_pointer.c",
        "../tests/chapter_19/copy_propagation/all_types/redundant_double_copies.c",
        "../tests/chapter_19/copy_propagation/all_types/redundant_struct_copies.c",
        "../tests/chapter_19/copy_propagation/all_types/store_doesnt_kill.c",
        "../tests/chapter_19/copy_propagation/all_types/dont_propagate/copy_to_offset.c",
        "../tests/chapter_19/copy_propagation/all_types/dont_propagate/dont_propagate_addr_of.c",
        "../tests/chapter_19/copy_propagation/all_types/dont_propagate/static_are_aliased.c",
        "../tests/chapter_19/copy_propagation/all_types/dont_propagate/store_kills_aliased.c",
        "../tests/chapter_19/copy_propagation/all_types/dont_propagate/type_conversion.c",
        "../tests/chapter_19/copy_propagation/all_types/dont_propagate/zero_neg_zero_different.c"
    };
    for (const auto& file : files) {
        run_tacky_optimization(file, "copy_propagation");
    }
}

TEST_CASE(CopyPropagation_AllTypes_ExtraCredit, "chapter_19", "copy_propagation_extra_credit")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/copy_propagation/all_types/extra_credit/copy_union.c",
        "../tests/chapter_19/copy_propagation/all_types/extra_credit/pointer_compound_assignment.c",
        "../tests/chapter_19/copy_propagation/all_types/extra_credit/pointer_incr.c",
        "../tests/chapter_19/copy_propagation/all_types/extra_credit/redundant_nan_copy.c",
        "../tests/chapter_19/copy_propagation/all_types/extra_credit/redundant_union_copy.c",
        "../tests/chapter_19/copy_propagation/all_types/extra_credit/dont_propagate/update_union_member.c",
        "../tests/chapter_19/copy_propagation/all_types/extra_credit/dont_propagate/update_union_member_2.c"
    };
    for (const auto& file : files) {
        run_tacky_optimization(file, "copy_propagation");
    }
}

TEST_CASE(CopyPropagation_IntOnly, "chapter_19", "copy_propagation_int_only")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/copy_propagation/int_only/constant_propagation.c",
        "../tests/chapter_19/copy_propagation/int_only/different_paths_same_copy.c",
        "../tests/chapter_19/copy_propagation/int_only/different_source_values_same_copy.c",
        "../tests/chapter_19/copy_propagation/int_only/fig_19_8.c",
        "../tests/chapter_19/copy_propagation/int_only/init_all_copies.c",
        "../tests/chapter_19/copy_propagation/int_only/killed_then_redefined.c",
        "../tests/chapter_19/copy_propagation/int_only/kill_and_add_copies.c",
        "../tests/chapter_19/copy_propagation/int_only/multi_path_no_kill.c",
        "../tests/chapter_19/copy_propagation/int_only/nested_loops.c",
        "../tests/chapter_19/copy_propagation/int_only/propagate_into_complex_expressions.c",
        "../tests/chapter_19/copy_propagation/int_only/propagate_params.c",
        "../tests/chapter_19/copy_propagation/int_only/propagate_static.c",
        "../tests/chapter_19/copy_propagation/int_only/propagate_static_var.c",
        "../tests/chapter_19/copy_propagation/int_only/propagate_var.c",
        "../tests/chapter_19/copy_propagation/int_only/redundant_copies.c",
        "../tests/chapter_19/copy_propagation/int_only/dont_propagate/add_all_blocks_to_worklist.c",
        "../tests/chapter_19/copy_propagation/int_only/dont_propagate/dest_killed.c",
        "../tests/chapter_19/copy_propagation/int_only/dont_propagate/listing_19_14.c",
        "../tests/chapter_19/copy_propagation/int_only/dont_propagate/multi_values.c",
        "../tests/chapter_19/copy_propagation/int_only/dont_propagate/no_copies_reach_entry.c",
        "../tests/chapter_19/copy_propagation/int_only/dont_propagate/one_reaching_copy.c",
        "../tests/chapter_19/copy_propagation/int_only/dont_propagate/source_killed.c",
        "../tests/chapter_19/copy_propagation/int_only/dont_propagate/source_killed_on_one_path.c",
        "../tests/chapter_19/copy_propagation/int_only/dont_propagate/static_dst_killed.c",
        "../tests/chapter_19/copy_propagation/int_only/dont_propagate/static_src_killed.c"
    };
    for (const auto& file : files) {
        run_tacky_optimization(file, "copy_propagation");
    }
}

TEST_CASE(CopyPropagation_IntOnly_ExtraCredit, "chapter_19", "copy_propagation_int_only_extra_credit")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/copy_propagation/int_only/extra_credit/goto_define.c",
        "../tests/chapter_19/copy_propagation/int_only/extra_credit/prefix_result.c",
        "../tests/chapter_19/copy_propagation/int_only/extra_credit/propagate_from_default.c",
        "../tests/chapter_19/copy_propagation/int_only/extra_credit/propagate_into_case.c",
        "../tests/chapter_19/copy_propagation/int_only/extra_credit/dont_propagate/decr_kills_dest.c",
        "../tests/chapter_19/copy_propagation/int_only/extra_credit/dont_propagate/switch_fallthrough.c"
    };
    for (const auto& file : files) {
        run_tacky_optimization(file, "copy_propagation");
    }
}

// ===================== DEAD STORE ELIMINATION =====================

TEST_CASE(DeadStoreElimination_AllTypes, "chapter_19", "dead_store_elimination")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/dead_store_elimination/all_types/aliased_dead_at_exit.c",
        "../tests/chapter_19/dead_store_elimination/all_types/copy_to_dead_struct.c",
        "../tests/chapter_19/dead_store_elimination/all_types/delete_dead_pt_ii_instructions.c",
        "../tests/chapter_19/dead_store_elimination/all_types/getaddr_doesnt_gen.c",
        "../tests/chapter_19/dead_store_elimination/all_types/dont_elim/copytooffset_doesnt_kill.c",
        "../tests/chapter_19/dead_store_elimination/all_types/dont_elim/funcall_generates_aliased.c",
        "../tests/chapter_19/dead_store_elimination/all_types/dont_elim/load_generates_aliased.c",
        "../tests/chapter_19/dead_store_elimination/all_types/dont_elim/never_kill_store.c",
        "../tests/chapter_19/dead_store_elimination/all_types/dont_elim/recognize_all_uses.c",
        "../tests/chapter_19/dead_store_elimination/all_types/dont_elim/use_and_update.c"
    };
    for (const auto& file : files) {
        run_tacky_optimization(file, "dead_store_elimination");
    }
}

TEST_CASE(DeadStoreElimination_AllTypes_ExtraCredit, "chapter_19", "dead_store_elimination_extra_credit")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/dead_store_elimination/all_types/extra_credit/compound_assign_to_dead_struct_member.c",
        "../tests/chapter_19/dead_store_elimination/all_types/extra_credit/copy_to_dead_union.c",
        "../tests/chapter_19/dead_store_elimination/all_types/extra_credit/decr_struct_member.c",
        "../tests/chapter_19/dead_store_elimination/all_types/extra_credit/dont_elim/copy_generates_union.c",
        "../tests/chapter_19/dead_store_elimination/all_types/extra_credit/dont_elim/incr_through_pointer.c",
        "../tests/chapter_19/dead_store_elimination/all_types/extra_credit/dont_elim/type_punning.c"
    };
    for (const auto& file : files) {
        run_tacky_optimization(file, "dead_store_elimination");
    }
}

TEST_CASE(DeadStoreElimination_IntOnly, "chapter_19", "dead_store_elimination_int_only")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/dead_store_elimination/int_only/dead_store_static_var.c",
        "../tests/chapter_19/dead_store_elimination/int_only/delete_arithmetic_ops.c",
        "../tests/chapter_19/dead_store_elimination/int_only/elim_second_copy.c",
        "../tests/chapter_19/dead_store_elimination/int_only/fig_19_11.c",
        "../tests/chapter_19/dead_store_elimination/int_only/initialize_blocks_with_empty_set.c",
        "../tests/chapter_19/dead_store_elimination/int_only/loop_dead_store.c",
        "../tests/chapter_19/dead_store_elimination/int_only/simple.c",
        "../tests/chapter_19/dead_store_elimination/int_only/static_not_always_live.c",
        "../tests/chapter_19/dead_store_elimination/int_only/dont_elim/add_all_to_worklist.c",
        "../tests/chapter_19/dead_store_elimination/int_only/dont_elim/dont_remove_funcall.c",
        "../tests/chapter_19/dead_store_elimination/int_only/dont_elim/loop.c",
        "../tests/chapter_19/dead_store_elimination/int_only/dont_elim/nested_loops.c",
        "../tests/chapter_19/dead_store_elimination/int_only/dont_elim/recognize_all_uses.c",
        "../tests/chapter_19/dead_store_elimination/int_only/dont_elim/self_copy.c",
        "../tests/chapter_19/dead_store_elimination/int_only/dont_elim/static_vars_at_exit.c",
        "../tests/chapter_19/dead_store_elimination/int_only/dont_elim/static_vars_fun.c",
        "../tests/chapter_19/dead_store_elimination/int_only/dont_elim/used_one_path.c"
    };
    for (const auto& file : files) {
        run_tacky_optimization(file, "dead_store_elimination");
    }
}

TEST_CASE(DeadStoreElimination_IntOnly_ExtraCredit, "chapter_19", "dead_store_elimination_int_only_extra_credit")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/dead_store_elimination/int_only/extra_credit/dead_compound_assignment.c",
        "../tests/chapter_19/dead_store_elimination/int_only/extra_credit/dead_incr_decr.c",
        "../tests/chapter_19/dead_store_elimination/int_only/extra_credit/dont_elim/incr_and_dead_store.c"
    };
    for (const auto& file : files) {
        run_tacky_optimization(file, "dead_store_elimination");
    }
}

// ===================== UNREACHABLE CODE ELIMINATION =====================

TEST_CASE(UnreachableCodeElimination, "chapter_19", "unreachable_code_elimination")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/unreachable_code_elimination/and_clause.c",
        "../tests/chapter_19/unreachable_code_elimination/constant_if_else.c",
        "../tests/chapter_19/unreachable_code_elimination/dead_after_if_else.c",
        "../tests/chapter_19/unreachable_code_elimination/dead_after_return.c",
        "../tests/chapter_19/unreachable_code_elimination/dead_blocks_with_predecessors.c",
        "../tests/chapter_19/unreachable_code_elimination/dead_branch_inside_loop.c",
        "../tests/chapter_19/unreachable_code_elimination/dead_for_loop.c",
        "../tests/chapter_19/unreachable_code_elimination/empty.c",
        "../tests/chapter_19/unreachable_code_elimination/empty_block.c",
        "../tests/chapter_19/unreachable_code_elimination/infinite_loop.c",
        "../tests/chapter_19/unreachable_code_elimination/keep_final_jump.c",
        "../tests/chapter_19/unreachable_code_elimination/or_clause.c",
        "../tests/chapter_19/unreachable_code_elimination/remove_conditional_jumps.c",
        "../tests/chapter_19/unreachable_code_elimination/remove_jump_keep_label.c",
        "../tests/chapter_19/unreachable_code_elimination/remove_useless_starting_label.c"
    };
    for (const auto& file : files) {
        run_tacky_optimization(file, "unreachable_code_elimination");
    }
}

TEST_CASE(UnreachableCodeElimination_ExtraCredit, "chapter_19", "unreachable_code_elimination_extra_credit")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/unreachable_code_elimination/extra_credit/dead_before_first_switch_case.c",
        "../tests/chapter_19/unreachable_code_elimination/extra_credit/dead_in_switch_body.c",
        "../tests/chapter_19/unreachable_code_elimination/extra_credit/goto_skips_over_code.c",
        "../tests/chapter_19/unreachable_code_elimination/extra_credit/remove_unused_label.c",
        "../tests/chapter_19/unreachable_code_elimination/extra_credit/unreachable_switch_body.c"
    };
    for (const auto& file : files) {
        run_tacky_optimization(file, "unreachable_code_elimination");
    }
}

// ===================== WHOLE PIPELINE =====================

TEST_CASE(WholePipeline_AllTypes, "chapter_19", "whole_pipeline")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/whole_pipeline/all_types/alias_analysis_change.c",
        "../tests/chapter_19/whole_pipeline/all_types/fold_cast_from_double.c",
        "../tests/chapter_19/whole_pipeline/all_types/fold_cast_to_double.c",
        "../tests/chapter_19/whole_pipeline/all_types/fold_char_condition.c",
        "../tests/chapter_19/whole_pipeline/all_types/fold_extension_and_truncation.c",
        "../tests/chapter_19/whole_pipeline/all_types/fold_infinity.c",
        "../tests/chapter_19/whole_pipeline/all_types/fold_negative_values.c",
        "../tests/chapter_19/whole_pipeline/all_types/fold_negative_zero.c",
        "../tests/chapter_19/whole_pipeline/all_types/integer_promotions.c",
        "../tests/chapter_19/whole_pipeline/all_types/listing_19_5_more_types.c",
        "../tests/chapter_19/whole_pipeline/all_types/propagate_into_copyfromoffset.c",
        "../tests/chapter_19/whole_pipeline/all_types/propagate_into_copytooffset.c",
        "../tests/chapter_19/whole_pipeline/all_types/propagate_into_load.c",
        "../tests/chapter_19/whole_pipeline/all_types/propagate_into_store.c",
        "../tests/chapter_19/whole_pipeline/all_types/signed_unsigned_conversion.c"
    };
    for (const auto& file : files) {
         run_tacky_optimization(file, "optimize");
    }
}

TEST_CASE(WholePipeline_AllTypes_ExtraCredit, "chapter_19", "whole_pipeline_extra_credit")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/whole_pipeline/all_types/extra_credit/eval_nan_condition.c",
        "../tests/chapter_19/whole_pipeline/all_types/extra_credit/fold_compound_assign_all_types.c",
        "../tests/chapter_19/whole_pipeline/all_types/extra_credit/fold_compound_bitwise_assign_all_types.c",
        "../tests/chapter_19/whole_pipeline/all_types/extra_credit/fold_incr_decr_chars.c",
        "../tests/chapter_19/whole_pipeline/all_types/extra_credit/fold_incr_decr_doubles.c",
        "../tests/chapter_19/whole_pipeline/all_types/extra_credit/fold_incr_decr_unsigned.c",
        "../tests/chapter_19/whole_pipeline/all_types/extra_credit/fold_negative_long_bitshift.c",
        "../tests/chapter_19/whole_pipeline/all_types/extra_credit/nan.c"
    };
    for (const auto& file : files) {
         run_tacky_optimization(file, "optimize");
    }
}

TEST_CASE(WholePipeline_IntOnly, "chapter_19", "whole_pipeline_int_only")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/whole_pipeline/int_only/dead_condition.c",
        "../tests/chapter_19/whole_pipeline/int_only/elim_and_copy_prop.c",
        "../tests/chapter_19/whole_pipeline/int_only/int_min.c",
        "../tests/chapter_19/whole_pipeline/int_only/listing_19_5.c",
        "../tests/chapter_19/whole_pipeline/int_only/remainder_test.c"
    };
    for (const auto& file : files) {
         run_tacky_optimization(file, "optimize");
    }
}

TEST_CASE(WholePipeline_IntOnly_ExtraCredit, "chapter_19", "whole_pipeline_int_only_extra_credit")
{
    std::vector<std::string> files = {
        "../tests/chapter_19/whole_pipeline/int_only/extra_credit/compound_assign_exceptions.c",
        "../tests/chapter_19/whole_pipeline/int_only/extra_credit/evaluate_switch.c",
        "../tests/chapter_19/whole_pipeline/int_only/extra_credit/fold_bitwise_compound_assignment.c",
        "../tests/chapter_19/whole_pipeline/int_only/extra_credit/fold_compound_assignment.c",
        "../tests/chapter_19/whole_pipeline/int_only/extra_credit/fold_incr_and_decr.c",
        "../tests/chapter_19/whole_pipeline/int_only/extra_credit/fold_negative_bitshift.c"
    };
    for (const auto& file : files) {
         run_tacky_optimization(file, "optimize");
    }
}