#include <iostream>
#include <algorithm>
#include <variant>

#include "CopyPropa.h"
#include "../optimizations/OptimizeUtils.h"
#include "../utils/TackyPrettyPrint.h"
#include "../CFG.h"
#include "../Symbols.h"

namespace CopyPropa
{

    // Comparison for Copy
    bool Copy::operator<(const Copy &other) const
    {
        if (*dst < *other.dst)
            return true;
        if (*other.dst < *dst)
            return false;
        return *src < *other.src;
    }
    bool Copy::operator==(const Copy &other) const
    {
        return (*src == *other.src) && (*dst == *other.dst);
    }

    void printCopy(const Copy &copy, std::ostream &os)
    {
        TackyPrettyPrint printer;
        printer.visit(*copy.dst, false);
        os << " = ";
        printer.visit(*copy.src, false);
    }

    // Helper: equality for TACKY::Val
    static bool eqVal(const std::shared_ptr<TACKY::Val> &a, const std::shared_ptr<TACKY::Val> &b)
    {
        return *a == *b;
    }

    // Helper: same type or same signedness
    static bool sameType(const std::shared_ptr<TACKY::Val> &v1, const std::shared_ptr<TACKY::Val> &v2, const Symbols::SymbolTable &symbolTable)
    {
        auto t1 = TACKY::typeOfVal(v1, symbolTable);
        auto t2 = TACKY::typeOfVal(v2, symbolTable);
        return t1 == t2 || Types::isSigned(t1) == Types::isSigned(t2);
    }

    // Helper: is variable aliased
    static bool varIsAliased(const std::set<std::string> &aliasedVars, const std::shared_ptr<TACKY::Val> &v, const Symbols::SymbolTable &symbolTable)
    {
        if (v->getType() == TACKY::NodeType::Constant)
            return false;
        if (v->getType() == TACKY::NodeType::Var)
        {
            auto name = std::static_pointer_cast<TACKY::Var>(v)->getName();
            return aliasedVars.count(name) > 0 || OptimizeUtils::isStatic(name, symbolTable);
        }
        return false;
    }

    // Remove all copies that involve the updated value
    static ReachingCopies filterUpdated(const ReachingCopies &copies, const std::shared_ptr<TACKY::Val> &updated)
    {
        ReachingCopies filtered;
        for (const auto &cp : copies)
        {
            if (!eqVal(cp.src, updated) && !eqVal(cp.dst, updated))
                filtered.insert(cp);
        }
        return filtered;
    }

    // Transfer function: annotate each instruction with reaching copies before it
    static std::pair<ReachingCopies, std::vector<std::pair<ReachingCopies, std::shared_ptr<TACKY::Instruction>>>>
    transfer(
        const std::set<std::string> &aliasedVars,
        const cfg::BasicBlock<ReachingCopies, TACKY::Instruction> &block,
        const ReachingCopies &initialReachingCopies,
        const Symbols::SymbolTable &symbolTable)
    {
        ReachingCopies current = initialReachingCopies;
        std::vector<std::pair<ReachingCopies, std::shared_ptr<TACKY::Instruction>>> annotated_instructions;

        for (const auto &instr_pair : block.instructions)
        {
            // Annotate instruction with current reaching copies
            annotated_instructions.emplace_back(current, instr_pair.second);

            auto instr = instr_pair.second;
            switch (instr->getType())
            {
            case TACKY::NodeType::Copy:
            {
                auto copy = std::static_pointer_cast<TACKY::Copy>(instr);
                auto src = copy->getSrc();
                auto dst = copy->getDst();
                Copy cp{src, dst};
                Copy reverse_cp{dst, src};
                if (current.count(reverse_cp))
                {
                    // dst and src already have the same value, so there's no effect
                }
                else if (sameType(src, dst, symbolTable))
                {
                    current = filterUpdated(current, dst);
                    current.insert(cp);
                }
                else
                {
                    current = filterUpdated(current, dst);
                }
                break;
            }
            case TACKY::NodeType::FunCall:
            {
                auto fun = std::static_pointer_cast<TACKY::FunCall>(instr);
                ReachingCopies copies = current;
                if (fun->getOptDst())
                {
                    copies = filterUpdated(copies, fun->getOptDst().value());
                }
                // Remove copies that are aliased
                ReachingCopies filtered;
                for (const auto &cp : copies)
                {
                    if (!varIsAliased(aliasedVars, cp.src, symbolTable) && !varIsAliased(aliasedVars, cp.dst, symbolTable))
                        filtered.insert(cp);
                }
                current = filtered;
                break;
            }
            case TACKY::NodeType::Store:
            {
                ReachingCopies filtered;
                for (const auto &cp : current)
                {
                    if (!varIsAliased(aliasedVars, cp.src, symbolTable) && !varIsAliased(aliasedVars, cp.dst, symbolTable))
                        filtered.insert(cp);
                }
                current = filtered;
                break;
            }
            default:
            {
                auto dst = OptimizeUtils::getDst(instr);
                if (dst)
                    current = filterUpdated(current, *dst);
                break;
            }
            }
        }
        return {current, annotated_instructions};
    }

    // Meet function: intersection of incoming copies
    static ReachingCopies meet(const ReachingCopies &ident, const cfg::Graph<ReachingCopies, TACKY::Instruction> &cfg, const cfg::BasicBlock<ReachingCopies, TACKY::Instruction> &block)
    {
        ReachingCopies incoming = ident;
        for (const auto &pred : block.preds)
        {
            if (pred.kind == cfg::NodeID::Kind::Entry)
            {
                incoming = ReachingCopies{};
            }
            else if (pred.kind == cfg::NodeID::Kind::Block)
            {
                auto v = cfg.basicBlocks.at(pred.index).value;
                ReachingCopies tmp;
                std::set_intersection(incoming.begin(), incoming.end(), v.begin(), v.end(), std::inserter(tmp, tmp.begin()));
                incoming = tmp;
            }
        }
        return incoming;
    }

    // Collect all possible copies in the CFG
    static ReachingCopies collectAllCopies(const cfg::Graph<std::monostate, TACKY::Instruction> &cfg, const Symbols::SymbolTable &symbolTable)
    {
        ReachingCopies result;
        auto instrs = cfg::cfgToInstructions(cfg);
        for (const auto &instr : instrs)
        {
            if (instr->getType() == TACKY::NodeType::Copy)
            {
                auto copy = std::static_pointer_cast<TACKY::Copy>(instr);
                if (sameType(copy->getSrc(), copy->getDst(), symbolTable))
                {
                    result.insert(Copy{copy->getSrc(), copy->getDst()});
                }
            }
        }
        return result;
    }

    // Find reaching copies for all blocks
    static cfg::Graph<ReachingCopies, TACKY::Instruction> findReachingCopies(
        const std::set<std::string> &aliasedVars,
        const cfg::Graph<std::monostate, TACKY::Instruction> &cfg,
        const Symbols::SymbolTable &symbolTable,
        bool debug)
    {
        ReachingCopies ident = collectAllCopies(cfg, symbolTable);
        auto annotated_cfg = cfg::initializeAnnotation(cfg, ident);

        // Worklist algorithm
        std::vector<int> worklist;
        for (const auto &[idx, _] : annotated_cfg.basicBlocks)
            worklist.push_back(idx);

        while (!worklist.empty())
        {
            if (debug)
            {
                printGraph(annotated_cfg, "FindingReachingCopies_in_progress");
            }

            int block_idx = worklist.back();
            worklist.pop_back();
            auto &blk = annotated_cfg.basicBlocks.at(block_idx);
            ReachingCopies old_annotation = blk.value;
            ReachingCopies incoming = meet(ident, annotated_cfg, blk);
            auto [new_copies, annotated_instructions] = transfer(aliasedVars, blk, incoming, symbolTable);
            if (new_copies != old_annotation || blk.instructions != annotated_instructions)
            {
                blk.value = new_copies;
                blk.instructions = std::move(annotated_instructions);
                // Add successors to worklist
                for (const auto &succ : blk.succs)
                {
                    if (succ.kind == cfg::NodeID::Kind::Block)
                        worklist.push_back(succ.index);
                }
            }
        }
        return annotated_cfg;
    }

    // Print the annotated CFG for debugging
    void printGraph(const cfg::Graph<ReachingCopies, TACKY::Instruction> &cfg, const std::string &extraTag)
    {
        std::cout << "==== CopyProp CFG with Reaching Copies ====" << std::endl;
        std::cout << "Debug label: " << cfg.debugLabel << extraTag << std::endl;
        for (const auto &[idx, block] : cfg.basicBlocks)
        {
            std::cout << "Block " << idx << ":\n";
            std::cout << "  Reaching copies in: { ";
            bool first = true;
            for (const auto &cp : block.value)
            {
                if (!first)
                    std::cout << ", ";
                printCopy(cp, std::cout);
                first = false;
            }
            std::cout << " }\n";
            std::cout << "  Instructions:\n";
            for (const auto &instr_pair : block.instructions)
            {
                // Print reaching copies for this instruction
                std::cout << "    [Reaching copies]: { ";
                bool first = true;
                for (const auto &cp : instr_pair.first)
                {
                    if (!first)
                        std::cout << ", ";
                    printCopy(cp, std::cout);
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

    // Rewriting instructions with copy propagation
    static std::optional<std::pair<ReachingCopies, std::shared_ptr<TACKY::Instruction>>>
    rewriteInstruction(const std::pair<ReachingCopies, std::shared_ptr<TACKY::Instruction>> &instr_pair)
    {
        const auto &reaching_copies = instr_pair.first;
        const auto &instr = instr_pair.second;

        // Helper to replace a Val with its reaching copy source if available
        auto replace = [&](const std::shared_ptr<TACKY::Val> &op) -> std::shared_ptr<TACKY::Val>
        {
            if (op->getType() == TACKY::NodeType::Constant)
                return op;
            if (op->getType() == TACKY::NodeType::Var)
            {
                for (const auto &cp : reaching_copies)
                {
                    if (*cp.dst == *op)
                        return cp.src;
                }
            }
            return op;
        };

        // Remove useless copies
        if (instr->getType() == TACKY::NodeType::Copy)
        {
            auto copy = std::static_pointer_cast<TACKY::Copy>(instr);
            Copy cp{copy->getSrc(), copy->getDst()};
            Copy reverse_cp{copy->getDst(), copy->getSrc()};
            if (reaching_copies.count(cp) || reaching_copies.count(reverse_cp))
            {
                return std::nullopt;
            }
            // Replace src if possible
            auto new_src = replace(copy->getSrc());
            if (new_src != copy->getSrc())
            {
                auto new_copy = std::make_shared<TACKY::Copy>(new_src, copy->getDst());
                return std::make_pair(reaching_copies, new_copy);
            }
        }

        // Replace operands for other instructions
        switch (instr->getType())
        {
        case TACKY::NodeType::Unary:
        {
            auto unary = std::static_pointer_cast<TACKY::Unary>(instr);
            auto new_src = replace(unary->getSrc());
            if (new_src != unary->getSrc())
            {
                auto new_unary = std::make_shared<TACKY::Unary>(unary->getOp(), new_src, unary->getDst());
                return std::make_pair(reaching_copies, new_unary);
            }
            break;
        }
        case TACKY::NodeType::Binary:
        {
            auto binary = std::static_pointer_cast<TACKY::Binary>(instr);
            auto new_src1 = replace(binary->getSrc1());
            auto new_src2 = replace(binary->getSrc2());
            if (new_src1 != binary->getSrc1() || new_src2 != binary->getSrc2())
            {
                auto new_binary = std::make_shared<TACKY::Binary>(binary->getOp(), new_src1, new_src2, binary->getDst());
                return std::make_pair(reaching_copies, new_binary);
            }
            break;
        }
        case TACKY::NodeType::Return:
        {
            auto ret = std::static_pointer_cast<TACKY::Return>(instr);
            if (ret->getOptValue())
            {
                auto new_val = replace(ret->getOptValue().value());
                if (new_val != ret->getOptValue().value())
                {
                    auto new_ret = std::make_shared<TACKY::Return>(new_val);
                    return std::make_pair(reaching_copies, new_ret);
                }
            }
            break;
        }
        case TACKY::NodeType::JumpIfZero:
        {
            auto jz = std::static_pointer_cast<TACKY::JumpIfZero>(instr);
            auto new_cond = replace(jz->getCond());
            if (new_cond != jz->getCond())
            {
                auto new_jz = std::make_shared<TACKY::JumpIfZero>(new_cond, jz->getTarget());
                return std::make_pair(reaching_copies, new_jz);
            }
            break;
        }
        case TACKY::NodeType::JumpIfNotZero:
        {
            auto jnz = std::static_pointer_cast<TACKY::JumpIfNotZero>(instr);
            auto new_cond = replace(jnz->getCond());
            if (new_cond != jnz->getCond())
            {
                auto new_jnz = std::make_shared<TACKY::JumpIfNotZero>(new_cond, jnz->getTarget());
                return std::make_pair(reaching_copies, new_jnz);
            }
            break;
        }
        case TACKY::NodeType::FunCall:
        {
            auto fun = std::static_pointer_cast<TACKY::FunCall>(instr);
            std::vector<std::shared_ptr<TACKY::Val>> new_args;
            bool changed = false;
            for (const auto &arg : fun->getArgs())
            {
                auto new_arg = replace(arg);
                if (new_arg != arg)
                    changed = true;
                new_args.push_back(new_arg);
            }
            if (changed)
            {
                auto new_fun = std::make_shared<TACKY::FunCall>(fun->getFnName(), new_args, fun->getOptDst());
                return std::make_pair(reaching_copies, new_fun);
            }
            break;
        }
        case TACKY::NodeType::SignExtend:
        {
            auto sx = std::static_pointer_cast<TACKY::SignExtend>(instr);
            auto new_src = replace(sx->getSrc());
            if (new_src != sx->getSrc())
            {
                auto new_sx = std::make_shared<TACKY::SignExtend>(new_src, sx->getDst());
                return std::make_pair(reaching_copies, new_sx);
            }
            break;
        }
        case TACKY::NodeType::ZeroExtend:
        {
            auto zx = std::static_pointer_cast<TACKY::ZeroExtend>(instr);
            auto new_src = replace(zx->getSrc());
            if (new_src != zx->getSrc())
            {
                auto new_zx = std::make_shared<TACKY::ZeroExtend>(new_src, zx->getDst());
                return std::make_pair(reaching_copies, new_zx);
            }
            break;
        }
        case TACKY::NodeType::DoubleToInt:
        {
            auto d2i = std::static_pointer_cast<TACKY::DoubleToInt>(instr);
            auto new_src = replace(d2i->getSrc());
            if (new_src != d2i->getSrc())
            {
                auto new_d2i = std::make_shared<TACKY::DoubleToInt>(new_src, d2i->getDst());
                return std::make_pair(reaching_copies, new_d2i);
            }
            break;
        }
        case TACKY::NodeType::IntToDouble:
        {
            auto i2d = std::static_pointer_cast<TACKY::IntToDouble>(instr);
            auto new_src = replace(i2d->getSrc());
            if (new_src != i2d->getSrc())
            {
                auto new_i2d = std::make_shared<TACKY::IntToDouble>(new_src, i2d->getDst());
                return std::make_pair(reaching_copies, new_i2d);
            }
            break;
        }
        case TACKY::NodeType::DoubleToUInt:
        {
            auto d2u = std::static_pointer_cast<TACKY::DoubleToUInt>(instr);
            auto new_src = replace(d2u->getSrc());
            if (new_src != d2u->getSrc())
            {
                auto new_d2u = std::make_shared<TACKY::DoubleToUInt>(new_src, d2u->getDst());
                return std::make_pair(reaching_copies, new_d2u);
            }
            break;
        }
        case TACKY::NodeType::UIntToDouble:
        {
            auto u2d = std::static_pointer_cast<TACKY::UIntToDouble>(instr);
            auto new_src = replace(u2d->getSrc());
            if (new_src != u2d->getSrc())
            {
                auto new_u2d = std::make_shared<TACKY::UIntToDouble>(new_src, u2d->getDst());
                return std::make_pair(reaching_copies, new_u2d);
            }
            break;
        }
        case TACKY::NodeType::Truncate:
        {
            auto t = std::static_pointer_cast<TACKY::Truncate>(instr);
            auto new_src = replace(t->getSrc());
            if (new_src != t->getSrc())
            {
                auto new_t = std::make_shared<TACKY::Truncate>(new_src, t->getDst());
                return std::make_pair(reaching_copies, new_t);
            }
            break;
        }
        case TACKY::NodeType::Load:
        {
            auto load = std::static_pointer_cast<TACKY::Load>(instr);
            auto new_src_ptr = replace(load->getSrcPtr());
            if (new_src_ptr != load->getSrcPtr())
            {
                auto new_load = std::make_shared<TACKY::Load>(new_src_ptr, load->getDst());
                return std::make_pair(reaching_copies, new_load);
            }
            break;
        }
        case TACKY::NodeType::Store:
        {
            auto store = std::static_pointer_cast<TACKY::Store>(instr);
            auto new_src = replace(store->getSrc());
            if (new_src != store->getSrc())
            {
                auto new_store = std::make_shared<TACKY::Store>(new_src, store->getDstPtr());
                return std::make_pair(reaching_copies, new_store);
            }
            break;
        }
        case TACKY::NodeType::AddPtr:
        {
            auto addptr = std::static_pointer_cast<TACKY::AddPtr>(instr);
            auto new_ptr = replace(addptr->getPtr());
            auto new_index = replace(addptr->getIndex());
            if (new_ptr != addptr->getPtr() || new_index != addptr->getIndex())
            {
                auto new_addptr = std::make_shared<TACKY::AddPtr>(new_ptr, new_index, addptr->getScale(), addptr->getDst());
                return std::make_pair(reaching_copies, new_addptr);
            }
            break;
        }
        case TACKY::NodeType::CopyToOffset:
        {
            auto c2o = std::static_pointer_cast<TACKY::CopyToOffset>(instr);
            auto new_src = replace(c2o->getSrc());
            if (new_src != c2o->getSrc())
            {
                auto new_c2o = std::make_shared<TACKY::CopyToOffset>(new_src, c2o->getDst(), c2o->getOffset());
                return std::make_pair(reaching_copies, new_c2o);
            }
            break;
        }
        case TACKY::NodeType::CopyFromOffset:
        {
            auto cfo = std::static_pointer_cast<TACKY::CopyFromOffset>(instr);
            auto src_var = std::make_shared<TACKY::Var>(cfo->getSrc());
            auto replaced = replace(src_var);
            if (replaced != src_var)
            {
                if (replaced->getType() == TACKY::NodeType::Var)
                {
                    auto new_cfo = std::make_shared<TACKY::CopyFromOffset>(
                        static_cast<TACKY::Var *>(replaced.get())->getName(),
                        cfo->getOffset(),
                        cfo->getDst());
                    return std::make_pair(reaching_copies, new_cfo);
                }
                else
                {
                    throw std::runtime_error("internal error: CopyFromOffset src replaced with constant");
                }
            }
            break;
        }
        default:
            break;
        }

        return instr_pair;
    }

    // Main entry: optimize copy propagation
    cfg::Graph<std::monostate, TACKY::Instruction> optimize(
        const std::set<std::string> &aliasedVars,
        const cfg::Graph<std::monostate, TACKY::Instruction> &cfg,
        const Symbols::SymbolTable &symbolTable,
        bool debug)
    {
        auto annotated_cfg = findReachingCopies(aliasedVars, cfg, symbolTable, debug);
        if (debug)
        {
            printGraph(annotated_cfg, "");
        }

        // Rewrite instructions in each block
        cfg::Graph<ReachingCopies, TACKY::Instruction> transformed_cfg = annotated_cfg;
        for (auto &[idx, block] : transformed_cfg.basicBlocks)
        {
            std::vector<std::pair<ReachingCopies, std::shared_ptr<TACKY::Instruction>>> new_instrs;
            for (const auto &instr_pair : block.instructions)
            {
                auto rewritten = rewriteInstruction(instr_pair);
                if (rewritten)
                {
                    new_instrs.push_back(*rewritten);
                }
            }
            block.instructions = std::move(new_instrs);
        }

        // Remove annotations since we no longer need them
        return cfg::stripAnnotations(transformed_cfg);
    }

}