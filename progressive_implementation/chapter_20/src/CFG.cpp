#include "CFG.h"
#include "TACKY.h"
#include "Assembly.h"
#include "../optimizations/CopyPropa.h"
#include <algorithm>
#include <cassert>
#include <set>

namespace cfg
{
    // Helper functions for TACKY
    inline bool isLabel(const std::shared_ptr<TACKY::Instruction> &instr)
    {
        return instr->getType() == TACKY::NodeType::Label;
    }
    inline bool isJump(const std::shared_ptr<TACKY::Instruction> &instr)
    {
        return instr->getType() == TACKY::NodeType::Jump;
    }
    inline bool isConditionalJump(const std::shared_ptr<TACKY::Instruction> &instr)
    {
        return instr->getType() == TACKY::NodeType::JumpIfZero || instr->getType() == TACKY::NodeType::JumpIfNotZero;
    }
    inline bool isReturn(const std::shared_ptr<TACKY::Instruction> &instr)
    {
        return instr->getType() == TACKY::NodeType::Return;
    }
    inline std::string getLabelName(const std::shared_ptr<TACKY::Instruction> &instr)
    {
        return static_cast<const TACKY::Label *>(instr.get())->getName();
    }
    inline std::string getJumpTarget(const std::shared_ptr<TACKY::Instruction> &instr)
    {
        if (instr->getType() == TACKY::NodeType::Jump)
            return static_cast<const TACKY::Jump *>(instr.get())->getTarget();
        if (instr->getType() == TACKY::NodeType::JumpIfZero)
            return static_cast<const TACKY::JumpIfZero *>(instr.get())->getTarget();
        if (instr->getType() == TACKY::NodeType::JumpIfNotZero)
            return static_cast<const TACKY::JumpIfNotZero *>(instr.get())->getTarget();
        return "";
    }

    // Helper functions for Assembly
    inline bool isLabel(const std::shared_ptr<Assembly::Instruction> &instr)
    {
        return instr->getType() == Assembly::NodeType::Label;
    }
    inline bool isJump(const std::shared_ptr<Assembly::Instruction> &instr)
    {
        return instr->getType() == Assembly::NodeType::Jmp;
    }
    inline bool isConditionalJump(const std::shared_ptr<Assembly::Instruction> &instr)
    {
        return instr->getType() == Assembly::NodeType::JmpCC;
    }
    inline bool isReturn(const std::shared_ptr<Assembly::Instruction> &instr)
    {
        return instr->getType() == Assembly::NodeType::Ret;
    }
    inline std::string getLabelName(const std::shared_ptr<Assembly::Instruction> &instr)
    {
        return static_cast<const Assembly::Label *>(instr.get())->getName();
    }
    inline std::string getJumpTarget(const std::shared_ptr<Assembly::Instruction> &instr)
    {
        if (instr->getType() == Assembly::NodeType::Jmp)
            return static_cast<const Assembly::Jmp *>(instr.get())->getTarget();
        if (instr->getType() == Assembly::NodeType::JmpCC)
            return static_cast<const Assembly::JmpCC *>(instr.get())->getTarget();
        return "";
    }

    // Partition instructions into basic blocks
    template <typename I>
    std::vector<std::vector<std::shared_ptr<I>>> partitionIntoBasicBlocks(const std::vector<std::shared_ptr<I>> &instructions)
    {
        std::vector<std::vector<std::shared_ptr<I>>> blocks;
        std::vector<std::shared_ptr<I>> current;
        for (const auto &instr : instructions)
        {
            if (isLabel(instr))
            {
                if (!current.empty())
                {
                    blocks.push_back(current);
                    current.clear();
                }
                current.push_back(instr);
            }
            else if (isJump(instr) || isConditionalJump(instr) || isReturn(instr))
            {
                current.push_back(instr);
                blocks.push_back(current);
                current.clear();
            }
            else
            {
                current.push_back(instr);
            }
        }
        if (!current.empty())
        {
            blocks.push_back(current);
        }
        return blocks;
    }

    // Build a CFG from a list of instructions
    template <typename I>
    Graph<std::monostate, I> instructionsToCFG(const std::string &debugLabel, const std::vector<std::shared_ptr<I>> &instructions)
    {
        Graph<std::monostate, I> g;
        auto blocks = partitionIntoBasicBlocks<I>(instructions);

        // Build basic blocks
        for (size_t i = 0; i < blocks.size(); ++i)
        {
            BasicBlock<std::monostate, I> blk;
            blk.id = NodeID::Block(static_cast<int>(i));
            for (const auto &instr : blocks[i])
            {
                blk.instructions.emplace_back(std::make_pair(std::monostate{}, instr));
            }
            blk.value = std::monostate{};
            g.basicBlocks[static_cast<int>(i)] = blk;
        }
        g.debugLabel = debugLabel;

        // Build label map
        std::map<std::string, NodeID> labelMap;
        for (const auto &[idx, blk] : g.basicBlocks)
        {
            if (!blk.instructions.empty() && isLabel(blk.instructions.front().second))
            {
                labelMap[getLabelName(blk.instructions.front().second)] = blk.id;
            }
        }

        // Add edges
        auto addEdge = [&](const NodeID &pred, const NodeID &succ)
        {
            if (pred.kind == NodeID::Kind::Block)
            {
                auto &blk = g.basicBlocks.at(pred.index);
                if (std::find(blk.succs.begin(), blk.succs.end(), succ) == blk.succs.end())
                    blk.succs.push_back(succ);
            }
            else if (pred.kind == NodeID::Kind::Entry)
            {
                if (std::find(g.entrySuccs.begin(), g.entrySuccs.end(), succ) == g.entrySuccs.end())
                    g.entrySuccs.push_back(succ);
            }
            if (succ.kind == NodeID::Kind::Block)
            {
                auto &blk = g.basicBlocks.at(succ.index);
                if (std::find(blk.preds.begin(), blk.preds.end(), pred) == blk.preds.end())
                    blk.preds.push_back(pred);
            }
            else if (succ.kind == NodeID::Kind::Exit)
            {
                if (std::find(g.exitPreds.begin(), g.exitPreds.end(), pred) == g.exitPreds.end())
                    g.exitPreds.push_back(pred);
            }
        };

        // Add entry edge
        if (!g.basicBlocks.empty())
            addEdge(NodeID::Entry(), NodeID::Block(0));

        // Add outgoing edges for each block
        for (const auto &[idx, blk] : g.basicBlocks)
        {
            if (blk.instructions.empty())
                continue;
            auto lastInstr = blk.instructions.back().second;
            NodeID nextBlock = (idx + 1 < static_cast<int>(g.basicBlocks.size())) ? NodeID::Block(idx + 1) : NodeID::Exit();

            if (isReturn(lastInstr))
                addEdge(blk.id, NodeID::Exit());
            else if (isJump(lastInstr))
            {
                auto it = labelMap.find(getJumpTarget(lastInstr));
                if (it != labelMap.end())
                    addEdge(blk.id, it->second);
            }
            else if (isConditionalJump(lastInstr))
            {
                auto it = labelMap.find(getJumpTarget(lastInstr));
                if (it != labelMap.end())
                    addEdge(blk.id, it->second);
                addEdge(blk.id, nextBlock);
            }
            else
                addEdge(blk.id, nextBlock);
        }

        return g;
    }

    // Convert CFG back to instructions
    template <typename I>
    std::vector<std::shared_ptr<I>> cfgToInstructions(const Graph<std::monostate, I> &g)
    {
        std::vector<std::shared_ptr<I>> result;
        for (const auto &[idx, blk] : g.basicBlocks)
        {
            for (const auto &instrPair : blk.instructions)
            {
                result.push_back(instrPair.second);
            }
        }
        return result;
    }

    // Get successors for a node
    template <typename V, typename I>
    std::vector<NodeID> getSuccs(const NodeID &id, const Graph<V, I> &g)
    {
        if (id.kind == NodeID::Kind::Entry)
            return g.entrySuccs;
        if (id.kind == NodeID::Kind::Block)
            return g.basicBlocks.at(id.index).succs;
        return {};
    }

    // Get block annotation value
    template <typename V, typename I>
    V getBlockValue(int blocknum, const Graph<V, I> &g)
    {
        return g.basicBlocks.at(blocknum).value;
    }

    // Update a basic block in the CFG
    template <typename V, typename I>
    Graph<V, I> updateBasicBlock(int blockIdx, const BasicBlock<V, I> &newBlock, const Graph<V, I> &g)
    {
        Graph<V, I> g2 = g;
        g2.basicBlocks[blockIdx] = newBlock;
        return g2;
    }

    // Initialize annotation for all blocks
    template <typename VA, typename VB, typename I>
    Graph<VB, I> initializeAnnotation(const Graph<VA, I> &cfg, const VB &dummyVal)
    {
        Graph<VB, I> out;
        out.debugLabel = cfg.debugLabel;
        out.entrySuccs = cfg.entrySuccs;
        out.exitPreds = cfg.exitPreds;
        for (const auto &[idx, blk] : cfg.basicBlocks)
        {
            BasicBlock<VB, I> b2;
            b2.id = blk.id;
            b2.preds = blk.preds;
            b2.succs = blk.succs;
            b2.value = dummyVal;
            for (const auto &instrPair : blk.instructions)
            {
                b2.instructions.emplace_back(std::make_pair(dummyVal, instrPair.second));
            }
            out.basicBlocks[idx] = b2;
        }
        return out;
    }

    // Strip all annotations (set to std::monostate)
    template <typename VA, typename I>
    Graph<std::monostate, I> stripAnnotations(const Graph<VA, I> &cfg)
    {
        return initializeAnnotation<VA, std::monostate, I>(cfg, std::monostate{});
    }

    // For debugging: print the CFG (simple version)
    template <typename V, typename I>
    void printCFG(const Graph<V, I> &cfg, std::function<void(const V &)> printVal)
    {
        std::cout << "CFG: " << cfg.debugLabel << "\n";
        for (const auto &[idx, blk] : cfg.basicBlocks)
        {
            std::cout << "Block " << idx << ":\n";
            std::cout << "  preds: ";
            for (const auto &p : blk.preds)
                std::cout << p << " ";
            std::cout << "\n  succs: ";
            for (const auto &s : blk.succs)
                std::cout << s << " ";
            std::cout << "\n  annotation: ";
            printVal(blk.value);
            std::cout << "\n  instructions:\n";
            for (const auto &instrPair : blk.instructions)
            {
                std::cout << "    [instr]\n";
            }
        }
    }

    // For debugging: print the CFG with pretty-printed instructions and annotations
    template <typename V, typename I>
    void printCFGPretty(const Graph<V, I> &cfg, std::function<void(const V &)> printVal)
    {
        std::cout << "==== CFG: " << cfg.debugLabel << " ====" << std::endl;
        for (const auto &[idx, blk] : cfg.basicBlocks)
        {
            std::cout << "Block " << idx << ":\n";
            std::cout << "  Annotation: ";
            printVal(blk.value);
            std::cout << "\n";
            std::cout << "  Instructions:\n";
            for (const auto &instrPair : blk.instructions)
            {
                std::cout << "    [instr]\n";
            }
            std::cout << "  Predecessors: ";
            for (const auto &pred : blk.preds)
                std::cout << pred << " ";
            std::cout << "\n";
            std::cout << "  Successors: ";
            for (const auto &succ : blk.succs)
                std::cout << succ << " ";
            std::cout << "\n\n";
        }
    }

    // For debugging: print the CFG with pretty-printed instructions (no annotation)
    template <typename I>
    void printCFGPretty(const Graph<std::monostate, I> &cfg)
    {
        std::cout << "==== CFG: " << cfg.debugLabel << " ====" << std::endl;
        for (const auto &[idx, blk] : cfg.basicBlocks)
        {
            std::cout << "Block " << idx << ":\n";
            std::cout << "  Instructions:\n";
            for (const auto &instrPair : blk.instructions)
            {
                std::cout << "    [instr]\n";
            }
            std::cout << "  Predecessors: ";
            for (const auto &pred : blk.preds)
                std::cout << pred << " ";
            std::cout << "\n";
            std::cout << "  Successors: ";
            for (const auto &succ : blk.succs)
                std::cout << succ << " ";
            std::cout << "\n\n";
        }
    }
}

// Explicit template instantiations
template class cfg::Graph<std::set<Assembly::Operand>, Assembly::Instruction>;
template class cfg::Graph<std::set<std::string>, TACKY::Instruction>;
template class cfg::Graph<std::set<CopyPropa::Copy>, TACKY::Instruction>;
template class cfg::Graph<std::monostate, TACKY::Instruction>;
template class cfg::Graph<std::monostate, Assembly::Instruction>;

// Explicit instantiation for pointer-based liveness analysis
template cfg::Graph<std::set<std::shared_ptr<Assembly::Operand>>, Assembly::Instruction> cfg::initializeAnnotation<std::monostate, std::set<std::shared_ptr<Assembly::Operand>>, Assembly::Instruction>(
    const cfg::Graph<std::monostate, Assembly::Instruction> &,
    const std::set<std::shared_ptr<Assembly::Operand>> &);

template cfg::Graph<std::set<std::string>, TACKY::Instruction> cfg::initializeAnnotation<std::monostate, std::set<std::string>, TACKY::Instruction>(
    const cfg::Graph<std::monostate, TACKY::Instruction> &,
    const std::set<std::string> &);
template cfg::Graph<std::set<Assembly::Operand>, Assembly::Instruction> cfg::initializeAnnotation<std::monostate, std::set<Assembly::Operand>, Assembly::Instruction>(
    const cfg::Graph<std::monostate, Assembly::Instruction> &,
    const std::set<Assembly::Operand> &);
template cfg::Graph<std::set<CopyPropa::Copy>, TACKY::Instruction> cfg::initializeAnnotation<std::monostate, std::set<CopyPropa::Copy>, TACKY::Instruction>(
    const cfg::Graph<std::monostate, TACKY::Instruction> &,
    const std::set<CopyPropa::Copy> &);

template cfg::Graph<std::monostate, TACKY::Instruction> cfg::stripAnnotations<std::set<std::string>, TACKY::Instruction>(
    const cfg::Graph<std::set<std::string>, TACKY::Instruction> &);
template cfg::Graph<std::monostate, TACKY::Instruction> cfg::stripAnnotations<std::set<CopyPropa::Copy>, TACKY::Instruction>(
    const cfg::Graph<std::set<CopyPropa::Copy>, TACKY::Instruction> &);

template std::vector<cfg::NodeID> cfg::getSuccs<std::monostate, TACKY::Instruction>(
    const cfg::NodeID &,
    const cfg::Graph<std::monostate, TACKY::Instruction> &);

template cfg::Graph<std::monostate, TACKY::Instruction> cfg::instructionsToCFG<TACKY::Instruction>(
    const std::string &,
    const std::vector<std::shared_ptr<TACKY::Instruction>> &);
template std::vector<std::shared_ptr<TACKY::Instruction>> cfg::cfgToInstructions<TACKY::Instruction>(
    const cfg::Graph<std::monostate, TACKY::Instruction> &);
template cfg::Graph<std::monostate, Assembly::Instruction> cfg::instructionsToCFG<Assembly::Instruction>(
    const std::string &,
    const std::vector<std::shared_ptr<Assembly::Instruction>> &);