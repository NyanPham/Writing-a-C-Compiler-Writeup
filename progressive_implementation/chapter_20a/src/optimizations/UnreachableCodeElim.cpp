#include "UnreachableCodeElim.h"
#include <set>
#include <algorithm>
#include <variant>
#include <iostream>
#include "../utils/TackyPrettyPrint.h"

namespace
{
    // DFS to find reachable nodes
    void dfs(const cfg::Graph<std::monostate, TACKY::Instruction> &g, const cfg::NodeID &node, std::set<cfg::NodeID> &visited)
    {
        if (visited.count(node))
            return;
        visited.insert(node);
        for (const auto &succ : cfg::getSuccs<std::monostate, TACKY::Instruction>(node, g))
        {
            dfs(g, succ, visited);
        }
    }

    // Remove unreachable blocks
    cfg::Graph<std::monostate, TACKY::Instruction> eliminateUnreachableBlocks(cfg::Graph<std::monostate, TACKY::Instruction> cfg)
    {
        std::set<cfg::NodeID> reachable;
        dfs(cfg, cfg::NodeID::Entry(), reachable);

        // Remove unreachable blocks and update edges
        std::vector<int> to_remove;
        for (const auto &[idx, blk] : cfg.basicBlocks)
        {
            // Skip Entry and Exit blocks
            if (blk.id.kind == cfg::NodeID::Kind::Entry || blk.id.kind == cfg::NodeID::Kind::Exit)
                continue;

            if (!reachable.count(blk.id))
            {
                // Remove edges from preds and succs
                for (const auto &p : blk.preds)
                {
                    auto &pred_blk = cfg.basicBlocks[p.index];
                    pred_blk.succs.erase(std::remove(pred_blk.succs.begin(), pred_blk.succs.end(), blk.id), pred_blk.succs.end());
                }
                for (const auto &s : blk.succs)
                {
                    auto &succ_blk = cfg.basicBlocks[s.index];
                    succ_blk.preds.erase(std::remove(succ_blk.preds.begin(), succ_blk.preds.end(), blk.id), succ_blk.preds.end());
                }
                to_remove.push_back(idx);
            }
        }
        for (int idx : to_remove)
        {
            cfg.basicBlocks.erase(idx);
        }
        return cfg;
    }

    // Remove useless jumps at the end of a block if all successors are the next block
    cfg::Graph<std::monostate, TACKY::Instruction> eliminateUselessJumps(cfg::Graph<std::monostate, TACKY::Instruction> cfg)
    {
        std::vector<int> block_indices;
        for (const auto &[idx, _] : cfg.basicBlocks)
            block_indices.push_back(idx);
        std::sort(block_indices.begin(), block_indices.end());

        for (size_t i = 0; i + 1 < block_indices.size(); ++i)
        {
            auto &blk = cfg.basicBlocks[block_indices[i]];
            if (blk.instructions.empty())
                continue;
            auto &last_instr = blk.instructions.back().second;
            auto next_block_id = cfg::NodeID::Block(block_indices[i + 1]);
            bool all_succs_are_next = !blk.succs.empty() && std::all_of(blk.succs.begin(), blk.succs.end(),
                                                                        [&](const cfg::NodeID &n)
                                                                        { return n == next_block_id; });

            auto t = last_instr->getType();
            if ((t == TACKY::NodeType::Jump || t == TACKY::NodeType::JumpIfZero || t == TACKY::NodeType::JumpIfNotZero) && all_succs_are_next)
            {
                blk.instructions.pop_back();
            }
        }
        return cfg;
    }

    // Remove useless labels at the start of a block if all preds are the default pred
    cfg::Graph<std::monostate, TACKY::Instruction> eliminateUselessLabels(cfg::Graph<std::monostate, TACKY::Instruction> cfg)
    {
        std::vector<int> block_indices;
        for (const auto &[idx, _] : cfg.basicBlocks)
            block_indices.push_back(idx);
        std::sort(block_indices.begin(), block_indices.end());

        for (size_t i = 0; i < block_indices.size(); ++i)
        {
            auto &blk = cfg.basicBlocks[block_indices[i]];
            if (blk.instructions.empty())
                continue;
            auto &first_instr = blk.instructions.front().second;
            if (first_instr->getType() == TACKY::NodeType::Label)
            {
                cfg::NodeID default_pred = (i == 0) ? cfg::NodeID::Entry() : cfg::NodeID::Block(block_indices[i - 1]);
                bool all_preds_are_default = !blk.preds.empty() && std::all_of(blk.preds.begin(), blk.preds.end(),
                                                                               [&](const cfg::NodeID &n)
                                                                               { return n == default_pred; });
                if (all_preds_are_default)
                {
                    blk.instructions.erase(blk.instructions.begin());
                }
            }
        }
        return cfg;
    }

    // Remove empty blocks (with exactly one pred and one succ)
    cfg::Graph<std::monostate, TACKY::Instruction> removeEmptyBlocks(cfg::Graph<std::monostate, TACKY::Instruction> cfg)
    {
        std::vector<int> to_remove;
        for (const auto &[idx, blk] : cfg.basicBlocks)
        {
            // Skip Entry and Exit blocks
            if (blk.id.kind == cfg::NodeID::Kind::Entry || blk.id.kind == cfg::NodeID::Kind::Exit)
                continue;

            if (blk.instructions.empty())
            {
                if (blk.preds.size() == 1 && blk.succs.size() == 1)
                {
                    auto pred = blk.preds[0];
                    auto succ = blk.succs[0];
                    // Remove edges
                    auto &pred_blk = cfg.basicBlocks[pred.index];
                    pred_blk.succs.erase(std::remove(pred_blk.succs.begin(), pred_blk.succs.end(), blk.id), pred_blk.succs.end());
                    auto &succ_blk = cfg.basicBlocks[succ.index];
                    succ_blk.preds.erase(std::remove(succ_blk.preds.begin(), succ_blk.preds.end(), blk.id), succ_blk.preds.end());
                    // Add edge from pred to succ
                    pred_blk.succs.push_back(succ);
                    succ_blk.preds.push_back(pred);
                    to_remove.push_back(idx);
                }
                else
                {
                    std::cerr << "Warning: Empty block " << idx << " does not have exactly one predecessor and one successor\n";
                    std::cerr << "  Block ID: " << blk.id << "\n";
                    std::cerr << "  Predecessors: ";
                    for (const auto &pred_id : blk.preds)
                    {
                        std::cerr << pred_id << " ";
                    }
                    std::cerr << "\n  Successors: ";
                    for (const auto &succ_id : blk.succs)
                    {
                        std::cerr << succ_id << " ";
                    }
                    std::cerr << "\n";
                    throw std::runtime_error("Empty block should have exactly one predecessor and one successor");
                }
            }
        }
        for (int idx : to_remove)
        {
            cfg.basicBlocks.erase(idx);
        }
        return cfg;
    }

    // Debug print for the CFG
    void printUnreachableCFG(const cfg::Graph<std::monostate, TACKY::Instruction> &cfg)
    {
        std::cout << "==== UnreachableCodeElim CFG ====" << std::endl;
        std::cout << "Debug label: " << cfg.debugLabel << "_unreachable" << std::endl;
        for (const auto &[idx, block] : cfg.basicBlocks)
        {
            std::cout << "Block " << idx << ":\n";
            std::cout << "  Instructions:\n";
            for (const auto &instr_pair : block.instructions)
            {
                std::cout << "    ";
                TackyPrettyPrint printer;
                printer.visit(*instr_pair.second, false);
                std::cout << "\n";
            }
            std::cout << "  Predecessors: ";
            for (const auto &pred : block.preds)
            {
                if (pred.kind == cfg::NodeID::Kind::Entry)
                    std::cout << "Entry ";
                else if (pred.kind == cfg::NodeID::Kind::Exit)
                    std::cout << "Exit ";
                else
                    std::cout << "Block" << pred.index << " ";
            }
            std::cout << "\n";
            std::cout << "  Successors: ";
            for (const auto &succ : block.succs)
            {
                if (succ.kind == cfg::NodeID::Kind::Entry)
                    std::cout << "Entry ";
                else if (succ.kind == cfg::NodeID::Kind::Exit)
                    std::cout << "Exit ";
                else
                    std::cout << "Block" << succ.index << " ";
            }
            std::cout << "\n\n";
        }
    }

}

cfg::Graph<std::monostate, TACKY::Instruction> eliminateUnreachableCode(const cfg::Graph<std::monostate, TACKY::Instruction> &input_cfg, bool debug)
{
    if (debug)
    {
        printUnreachableCFG(input_cfg);
    }

    cfg::Graph<std::monostate, TACKY::Instruction> cfg = input_cfg;
    cfg = eliminateUnreachableBlocks(cfg);
    cfg = eliminateUselessJumps(cfg);
    cfg = eliminateUselessLabels(cfg);
    cfg = removeEmptyBlocks(cfg);
    return cfg;
}