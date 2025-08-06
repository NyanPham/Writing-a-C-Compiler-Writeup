#pragma once

#include <set>
#include <functional>
#include <vector>
#include <utility>
#include "CFG.h"

// Backward dataflow analysis utilities.
//
// Dataflow analysis over a CFG, parameterized by variable type and instruction type.
// - Annotation: std::set<Var>
// - AnnotatedBlock: cfg::BasicBlock<std::set<Var>, Instr>
// - AnnotatedGraph: cfg::Graph<std::set<Var>, Instr>
namespace BackwardDataflow
{
    template <typename Var, typename Instr>
    using Annotation = std::set<Var>;

    template <typename Var, typename Instr>
    using AnnotatedBlock = cfg::BasicBlock<Annotation<Var, Instr>, Instr>;

    template <typename Var, typename Instr>
    using AnnotatedGraph = cfg::Graph<Annotation<Var, Instr>, Instr>;

    // Analyze backward dataflow.
    // debugPrinter: called with the current graph for debugging.
    // meetFn: computes the meet (merge) of successor annotations for a block.
    // transferFn: computes the annotation for a block given the meet.
    // Returns the fixed-point AnnotatedGraph.
    template <typename Var, typename Instr>
    AnnotatedGraph<Var, Instr> analyze(
        std::function<void(const AnnotatedGraph<Var, Instr> &)> debugPrinter,
        std::function<Annotation<Var, Instr>(const AnnotatedGraph<Var, Instr> &, const AnnotatedBlock<Var, Instr> &)> meetFn,
        std::function<AnnotatedBlock<Var, Instr>(const AnnotatedBlock<Var, Instr> &, const Annotation<Var, Instr> &)> transferFn,
        const cfg::Graph<std::monostate, Instr> &cfg)
    {
        // Initialize all block annotations to empty set
        auto startingCfg = cfg::initializeAnnotation<std::monostate, Annotation<Var, Instr>, Instr>(cfg, Annotation<Var, Instr>{});

        // Worklist: vector of (blkId, block)
        std::vector<std::pair<int, AnnotatedBlock<Var, Instr>>> worklist;
        for (const auto &kv : startingCfg.basicBlocks)
            worklist.emplace_back(kv.first, kv.second);

        std::reverse(worklist.begin(), worklist.end());

        auto currentCfg = startingCfg;

        while (!worklist.empty())
        {
            debugPrinter(currentCfg);

            auto [blkId, blk] = worklist.back();
            worklist.pop_back();

            auto oldAnnotation = blk.value;
            auto liveVarsAtExit = meetFn(currentCfg, blk);
            auto blkPrime = transferFn(blk, liveVarsAtExit);

            // Update block in graph
            currentCfg.basicBlocks[blkId] = blkPrime;

            // If annotation changed, add predecessors to worklist
            if (oldAnnotation != blkPrime.value)
            {
                for (const auto &pred : blkPrime.preds)
                {
                    if (pred.kind == cfg::NodeID::Kind::Entry)
                        continue;
                    if (pred.kind == cfg::NodeID::Kind::Exit)
                        throw std::runtime_error("Internal error: malformed CFG");
                    if (pred.kind == cfg::NodeID::Kind::Block)
                    {
                        int predIdx = pred.index;
                        // Only add if not already present
                        auto it = std::find_if(worklist.begin(), worklist.end(),
                                               [predIdx](const auto &p)
                                               { return p.first == predIdx; });
                        if (it == worklist.end())
                        {
                            worklist.emplace_back(predIdx, currentCfg.basicBlocks.at(predIdx));
                        }
                    }
                }
            }
        }
        return currentCfg;
    }
}