#include "DeadStoreElim.h"
#include "../optimizations/OptimizeUtils.h"
#include "../utils/TackyPrettyPrint.h"
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <memory>
#include <variant>
#include <iostream>

namespace
{

    using StringSet = std::set<std::string>;

    // Helper: add variable to set if it's a Var
    void addVar(const std::shared_ptr<TACKY::Val> &v, StringSet &set)
    {
        if (v->getType() == TACKY::NodeType::Var)
        {
            set.insert(std::static_pointer_cast<TACKY::Var>(v)->getName());
        }
    }

    // Helper: remove variable from set if it's a Var
    void removeVar(const std::shared_ptr<TACKY::Val> &v, StringSet &set)
    {
        if (v->getType() == TACKY::NodeType::Var)
        {
            set.erase(std::static_pointer_cast<TACKY::Var>(v)->getName());
        }
    }

    // Transfer function for a block (reverse order, with per-instruction annotation)
    std::pair<StringSet, std::vector<std::pair<StringSet, std::shared_ptr<TACKY::Instruction>>>>
    transfer(
        const StringSet &staticAndAliasedVars,
        const cfg::BasicBlock<StringSet> &block,
        const StringSet &endLiveVars)
    {
        StringSet live = endLiveVars;
        std::vector<std::pair<StringSet, std::shared_ptr<TACKY::Instruction>>> annotated_instructions;

        for (auto it = block.instructions.rbegin(); it != block.instructions.rend(); ++it)
        {
            // Annotate with live-before
            annotated_instructions.emplace_back(live, it->second);

            const auto &instr = it->second;
            switch (instr->getType())
            {
            case TACKY::NodeType::Binary:
            {
                auto b = std::static_pointer_cast<TACKY::Binary>(instr);
                removeVar(b->getDst(), live);
                addVar(b->getSrc1(), live);
                addVar(b->getSrc2(), live);
                break;
            }
            case TACKY::NodeType::Unary:
            {
                auto u = std::static_pointer_cast<TACKY::Unary>(instr);
                removeVar(u->getDst(), live);
                addVar(u->getSrc(), live);
                break;
            }
            case TACKY::NodeType::JumpIfZero:
            {
                auto jz = std::static_pointer_cast<TACKY::JumpIfZero>(instr);
                addVar(jz->getCond(), live);
                break;
            }
            case TACKY::NodeType::JumpIfNotZero:
            {
                auto jnz = std::static_pointer_cast<TACKY::JumpIfNotZero>(instr);
                addVar(jnz->getCond(), live);
                break;
            }
            case TACKY::NodeType::Copy:
            {
                auto c = std::static_pointer_cast<TACKY::Copy>(instr);
                removeVar(c->getDst(), live);
                addVar(c->getSrc(), live);
                break;
            }
            case TACKY::NodeType::Return:
            {
                auto r = std::static_pointer_cast<TACKY::Return>(instr);
                if (r->getOptValue())
                    addVar(r->getOptValue().value(), live);
                break;
            }
            case TACKY::NodeType::FunCall:
            {
                auto f = std::static_pointer_cast<TACKY::FunCall>(instr);
                if (f->getOptDst())
                    removeVar(f->getOptDst().value(), live);
                for (const auto &arg : f->getArgs())
                    addVar(arg, live);
                live.insert(staticAndAliasedVars.begin(), staticAndAliasedVars.end());
                break;
            }
            case TACKY::NodeType::SignExtend:
            {
                auto sx = std::static_pointer_cast<TACKY::SignExtend>(instr);
                removeVar(sx->getDst(), live);
                addVar(sx->getSrc(), live);
                break;
            }
            case TACKY::NodeType::ZeroExtend:
            {
                auto zx = std::static_pointer_cast<TACKY::ZeroExtend>(instr);
                removeVar(zx->getDst(), live);
                addVar(zx->getSrc(), live);
                break;
            }
            case TACKY::NodeType::DoubleToInt:
            {
                auto d2i = std::static_pointer_cast<TACKY::DoubleToInt>(instr);
                removeVar(d2i->getDst(), live);
                addVar(d2i->getSrc(), live);
                break;
            }
            case TACKY::NodeType::IntToDouble:
            {
                auto i2d = std::static_pointer_cast<TACKY::IntToDouble>(instr);
                removeVar(i2d->getDst(), live);
                addVar(i2d->getSrc(), live);
                break;
            }
            case TACKY::NodeType::DoubleToUInt:
            {
                auto d2u = std::static_pointer_cast<TACKY::DoubleToUInt>(instr);
                removeVar(d2u->getDst(), live);
                addVar(d2u->getSrc(), live);
                break;
            }
            case TACKY::NodeType::UIntToDouble:
            {
                auto u2d = std::static_pointer_cast<TACKY::UIntToDouble>(instr);
                removeVar(u2d->getDst(), live);
                addVar(u2d->getSrc(), live);
                break;
            }
            case TACKY::NodeType::Truncate:
            {
                auto t = std::static_pointer_cast<TACKY::Truncate>(instr);
                removeVar(t->getDst(), live);
                addVar(t->getSrc(), live);
                break;
            }
            case TACKY::NodeType::AddPtr:
            {
                auto ap = std::static_pointer_cast<TACKY::AddPtr>(instr);
                removeVar(ap->getDst(), live);
                addVar(ap->getPtr(), live);
                addVar(ap->getIndex(), live);
                break;
            }
            case TACKY::NodeType::GetAddress:
            {
                auto ga = std::static_pointer_cast<TACKY::GetAddress>(instr);
                removeVar(ga->getDst(), live);
                break;
            }
            case TACKY::NodeType::Load:
            {
                auto l = std::static_pointer_cast<TACKY::Load>(instr);
                removeVar(l->getDst(), live);
                addVar(l->getSrcPtr(), live);
                live.insert(staticAndAliasedVars.begin(), staticAndAliasedVars.end());
                break;
            }
            case TACKY::NodeType::Store:
            {
                auto s = std::static_pointer_cast<TACKY::Store>(instr);
                addVar(s->getSrc(), live);
                addVar(s->getDstPtr(), live);
                break;
            }
            case TACKY::NodeType::CopyToOffset:
            {
                auto c2o = std::static_pointer_cast<TACKY::CopyToOffset>(instr);
                addVar(c2o->getSrc(), live);
                break;
            }
            case TACKY::NodeType::CopyFromOffset:
            {
                auto cfo = std::static_pointer_cast<TACKY::CopyFromOffset>(instr);
                removeVar(cfo->getDst(), live);
                addVar(std::make_shared<TACKY::Var>(cfo->getSrc()), live);
                break;
            }
            default:
                break; // Jump, Label, Return None, etc.
            }
        }
        std::reverse(annotated_instructions.begin(), annotated_instructions.end());
        return {live, annotated_instructions};
    }

    // Meet function: union of live variables from all successors
    StringSet meet(
        const StringSet &staticVars,
        const cfg::Graph<StringSet> &cfg,
        const cfg::BasicBlock<StringSet> &block)
    {
        StringSet live;
        for (const auto &succ : block.succs)
        {
            if (succ.kind == cfg::NodeID::Kind::Exit)
            {
                live.insert(staticVars.begin(), staticVars.end());
            }
            else if (succ.kind == cfg::NodeID::Kind::Block)
            {
                const auto &succ_blk = cfg.basicBlocks.at(succ.index);
                live.insert(succ_blk.value.begin(), succ_blk.value.end());
            }
        }
        return live;
    }

    // Print the annotated CFG for debugging
    void printLiveVarCFG(const cfg::Graph<StringSet> &cfg, const std::string &extraTag = "")
    {
        std::cout << "==== DeadStoreElim CFG with Live Variables ====" << std::endl;
        std::cout << "Debug label: " << cfg.debugLabel << "_dse" << extraTag << std::endl;
        for (const auto &[idx, block] : cfg.basicBlocks)
        {
            std::cout << "Block " << idx << ":\n";
            std::cout << "  Live variables: { ";
            bool first = true;
            for (const auto &var : block.value)
            {
                if (!first)
                    std::cout << ", ";
                std::cout << var;
                first = false;
            }
            std::cout << " }\n";
            std::cout << "  Instructions:\n";
            for (const auto &instr_pair : block.instructions)
            {
                // Print live variables before this instruction
                std::cout << "    [Live before]: { ";
                bool first = true;
                for (const auto &var : instr_pair.first)
                {
                    if (!first)
                        std::cout << ", ";
                    std::cout << var;
                    first = false;
                }
                std::cout << " }\n";

                // Print the instruction itself
                std::cout << "    ";
                TackyPrettyPrint printer;
                printer.visit(*instr_pair.second, false);
                std::cout << "\n";
            }
            std::cout << std::endl;
        }
    }

    // Find live variables for all blocks (backward dataflow)
    cfg::Graph<StringSet> findLiveVariables(
        const StringSet &staticVars,
        const StringSet &aliasedVars,
        const cfg::Graph<std::monostate> &cfg,
        bool debug)
    {
        auto annotated_cfg = cfg::initializeAnnotation(cfg, StringSet{});
        StringSet staticAndAliasedVars = staticVars;
        staticAndAliasedVars.insert(aliasedVars.begin(), aliasedVars.end());

        // Worklist algorithm (reverse postorder)
        std::vector<int> worklist;
        for (const auto &[idx, _] : annotated_cfg.basicBlocks)
            worklist.push_back(idx);

        while (!worklist.empty())
        {
            if (debug)
            {
                printLiveVarCFG(annotated_cfg, "FindingLiveVariables_in_progress");
            }

            int block_idx = worklist.back();
            worklist.pop_back();
            auto &blk = annotated_cfg.basicBlocks.at(block_idx);
            StringSet old_live = blk.value;
            StringSet live_at_exit = meet(staticVars, annotated_cfg, blk);
            auto [new_live, annotated_instructions] = transfer(staticAndAliasedVars, blk, live_at_exit);
            if (new_live != old_live || blk.instructions != annotated_instructions)
            {
                blk.value = new_live;
                blk.instructions = std::move(annotated_instructions);
                // Add predecessors to worklist
                for (const auto &pred : blk.preds)
                {
                    if (pred.kind == cfg::NodeID::Kind::Block)
                        worklist.push_back(pred.index);
                }
            }
        }
        return annotated_cfg;
    }

    // Is this a dead store?
    bool isDeadStore(const std::pair<StringSet, std::shared_ptr<TACKY::Instruction>> &instr_pair)
    {
        const auto &live_vars = instr_pair.first;
        const auto &instr = instr_pair.second;
        if (instr->getType() == TACKY::NodeType::FunCall || instr->getType() == TACKY::NodeType::Store)
            return false;
        auto dst = OptimizeUtils::getDst(instr);
        if (dst && (*dst)->getType() == TACKY::NodeType::Var)
        {
            auto v = std::static_pointer_cast<TACKY::Var>(*dst)->getName();
            return live_vars.count(v) == 0;
        }
        return false;
    }

    // Get all static variables from the symbol table
    StringSet getStaticVars(const Symbols::SymbolTable &symbolTable)
    {
        StringSet result;
        for (const auto &[name, sym] : symbolTable.getAllSymbols())
        {
            if (std::holds_alternative<Symbols::StaticAttr>(sym.attrs))
                result.insert(name);
        }
        return result;
    }

}

cfg::Graph<std::monostate> eliminateDeadStores(
    const std::set<std::string> &aliasedVars,
    const cfg::Graph<std::monostate> &cfg,
    const Symbols::SymbolTable &symbolTable,
    bool debug)
{
    StringSet staticVars = getStaticVars(symbolTable);
    auto annotated_cfg = findLiveVariables(staticVars, aliasedVars, cfg, debug);

    // Debug print: print annotated CFG before rewriting
    if (debug)
    {
        printLiveVarCFG(annotated_cfg);
    }

    // Remove dead stores
    auto transformed_cfg = annotated_cfg;
    for (auto &[idx, block] : transformed_cfg.basicBlocks)
    {
        std::vector<std::pair<StringSet, std::shared_ptr<TACKY::Instruction>>> new_instrs;
        for (const auto &instr_pair : block.instructions)
        {
            if (!isDeadStore(instr_pair))
                new_instrs.push_back(instr_pair);
        }
        block.instructions = std::move(new_instrs);
    }

    // Remove annotations
    return cfg::stripAnnotations(transformed_cfg);
}