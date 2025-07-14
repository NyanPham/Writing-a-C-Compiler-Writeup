#include <algorithm>
#include <cassert>
#include <set>

#include "CFG.h"
#include "../utils/TackyPrettyPrint.h"

#include "optimizations/CopyPropa.h"
#include "optimizations/UnreachableCodeElim.h"
#include "optimizations/DeadStoreElim.h"

namespace cfg
{

    // Partition instructions into basic blocks
    std::vector<std::vector<std::shared_ptr<TACKY::Instruction>>>
    partitionIntoBasicBlocks(const std::vector<std::shared_ptr<TACKY::Instruction>> &instructions)
    {
        std::vector<std::vector<std::shared_ptr<TACKY::Instruction>>> blocks;
        std::vector<std::shared_ptr<TACKY::Instruction>> current;
        for (const auto &instr : instructions)
        {
            if (instr->getType() == TACKY::NodeType::Label)
            {
                if (!current.empty())
                {
                    blocks.push_back(current);
                    current.clear();
                }
                current.push_back(instr);
            }
            else if (
                instr->getType() == TACKY::NodeType::Jump ||
                instr->getType() == TACKY::NodeType::JumpIfZero ||
                instr->getType() == TACKY::NodeType::JumpIfNotZero ||
                instr->getType() == TACKY::NodeType::Return)
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
    Graph<std::monostate> instructionsToCFG(const std::string &debugLabel, const std::vector<std::shared_ptr<TACKY::Instruction>> &instructions)
    {
        Graph<std::monostate> g;
        auto blocks = partitionIntoBasicBlocks(instructions);

        // Build basic blocks
        for (size_t i = 0; i < blocks.size(); ++i)
        {
            BasicBlock<std::monostate> blk;
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
            if (!blk.instructions.empty() && blk.instructions.front().second->getType() == TACKY::NodeType::Label)
            {
                auto labelInstr = std::static_pointer_cast<TACKY::Label>(blk.instructions.front().second);
                labelMap[labelInstr->getName()] = blk.id;
            }
        }

        // Add edges
        auto add_edge = [&](const NodeID &pred, const NodeID &succ)
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
            add_edge(NodeID::Entry(), NodeID::Block(0));

        // Add outgoing edges for each block
        for (const auto &[idx, blk] : g.basicBlocks)
        {
            if (blk.instructions.empty())
                continue;
            auto lastInstr = blk.instructions.back().second;
            NodeID nextBlock = (idx + 1 < static_cast<int>(g.basicBlocks.size())) ? NodeID::Block(idx + 1) : NodeID::Exit();

            switch (lastInstr->getType())
            {
            case TACKY::NodeType::Return:
                add_edge(blk.id, NodeID::Exit());
                break;
            case TACKY::NodeType::Jump:
            {
                auto jump = std::static_pointer_cast<TACKY::Jump>(lastInstr);
                auto it = labelMap.find(jump->getTarget());
                if (it != labelMap.end())
                    add_edge(blk.id, it->second);
                break;
            }
            case TACKY::NodeType::JumpIfZero:
            {
                auto jz = std::static_pointer_cast<TACKY::JumpIfZero>(lastInstr);
                auto it = labelMap.find(jz->getTarget());
                if (it != labelMap.end())
                    add_edge(blk.id, it->second);
                add_edge(blk.id, nextBlock);
                break;
            }
            case TACKY::NodeType::JumpIfNotZero:
            {
                auto jnz = std::static_pointer_cast<TACKY::JumpIfNotZero>(lastInstr);
                auto it = labelMap.find(jnz->getTarget());
                if (it != labelMap.end())
                    add_edge(blk.id, it->second);
                add_edge(blk.id, nextBlock);
                break;
            }
            default:
                add_edge(blk.id, nextBlock);
                break;
            }
        }

        return g;
    }

    // Convert CFG back to instructions
    std::vector<std::shared_ptr<TACKY::Instruction>> cfgToInstructions(const Graph<std::monostate> &g)
    {
        std::vector<std::shared_ptr<TACKY::Instruction>> result;
        for (const auto &[idx, blk] : g.basicBlocks)
        {
            for (const auto &instr_pair : blk.instructions)
            {
                result.push_back(instr_pair.second);
            }
        }
        return result;
    }

    // Get successors for a node
    template <typename V>
    std::vector<NodeID> getSuccs(const NodeID &id, const Graph<V> &g)
    {
        if (id.kind == NodeID::Kind::Entry)
            return g.entrySuccs;
        if (id.kind == NodeID::Kind::Block)
            return g.basicBlocks.at(id.index).succs;
        return {};
    }

    // Get block annotation value
    template <typename V>
    V getBlockValue(int blocknum, const Graph<V> &g)
    {
        return g.basicBlocks.at(blocknum).value;
    }

    // Update a basic block in the CFG
    template <typename V>
    Graph<V> updateBasicBlock(int blockIdx, const BasicBlock<V> &newBlock, const Graph<V> &g)
    {
        Graph<V> g2 = g;
        g2.basicBlocks[blockIdx] = newBlock;
        return g2;
    }

    // Initialize annotation for all blocks
    template <typename VA, typename VB>
    Graph<VB> initializeAnnotation(const Graph<VA> &cfg, const VB &dummyVal)
    {
        Graph<VB> out;
        out.debugLabel = cfg.debugLabel;
        out.entrySuccs = cfg.entrySuccs;
        out.exitPreds = cfg.exitPreds;
        for (const auto &[idx, blk] : cfg.basicBlocks)
        {
            BasicBlock<VB> b2;
            b2.id = blk.id;
            b2.preds = blk.preds;
            b2.succs = blk.succs;
            b2.value = dummyVal;
            for (const auto &instr_pair : blk.instructions)
            {
                b2.instructions.emplace_back(std::make_pair(dummyVal, instr_pair.second));
            }
            out.basicBlocks[idx] = b2;
        }
        return out;
    }

    // Strip all annotations (set to std::monostate)
    template <typename VA>
    Graph<std::monostate> stripAnnotations(const Graph<VA> &cfg)
    {
        return initializeAnnotation<VA, std::monostate>(cfg, std::monostate{});
    }

    // For debugging: print the CFG (simple version)
    template <typename V>
    void printCFG(const Graph<V> &cfg, std::function<void(const V &)> printVal)
    {
        std::cout << "CFG: " << cfg.debugLabel << "\n";
        for (const auto &[idx, blk] : cfg.basicBlocks)
        {
            std::cout << "Block " << idx << ":\n";
            std::cout << "  preds: ";
            for (const auto &p : blk.preds)
            {
                if (p.kind == NodeID::Kind::Block)
                    std::cout << p.index << " ";
                else if (p.kind == NodeID::Kind::Entry)
                    std::cout << "Entry ";
                else
                    std::cout << "Exit ";
            }
            std::cout << "\n  succs: ";
            for (const auto &s : blk.succs)
            {
                if (s.kind == NodeID::Kind::Block)
                    std::cout << s.index << " ";
                else if (s.kind == NodeID::Kind::Entry)
                    std::cout << "Entry ";
                else
                    std::cout << "Exit ";
            }
            std::cout << "\n  annotation: ";
            printVal(blk.value);
            std::cout << "\n  instructions:\n";
            for (const auto &instr_pair : blk.instructions)
            {
                std::cout << "    ";
                // You may want to pretty print the instruction here
                std::cout << "[instr]\n";
            }
        }
    }

    // For debugging: print the CFG (pretty version)
    template <typename V>
    void printCFGPretty(const Graph<V> &cfg, std::function<void(const V &)> printVal)
    {
        std::cout << "==== CFG: " << cfg.debugLabel << " ====" << std::endl;
        for (const auto &[idx, blk] : cfg.basicBlocks)
        {
            std::cout << "Block " << idx << ":\n";
            std::cout << "  Annotation: ";
            printVal(blk.value);
            std::cout << "\n";
            std::cout << "  Instructions:\n";
            for (const auto &instr_pair : blk.instructions)
            {
                std::cout << "    ";
                TackyPrettyPrint printer;
                printer.visit(*instr_pair.second, false);
                std::cout << "\n";
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

    void printCFGPretty(const Graph<std::monostate> &cfg)
    {
        std::cout << "==== CFG: " << cfg.debugLabel << " ====" << std::endl;
        for (const auto &[idx, blk] : cfg.basicBlocks)
        {
            std::cout << "Block " << idx << ":\n";
            std::cout << "  Instructions:\n";
            for (const auto &instr_pair : blk.instructions)
            {
                std::cout << "    ";
                TackyPrettyPrint printer;
                printer.visit(*instr_pair.second, false);
                std::cout << "\n";
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

    // Explicit template instantiations for std::monostate
    template std::vector<NodeID> getSuccs<std::monostate>(const NodeID &, const Graph<std::monostate> &);
    template void printCFG<std::monostate>(const Graph<std::monostate> &, std::function<void(const std::monostate &)>);
    template void printCFGPretty<std::monostate>(const Graph<std::monostate> &, std::function<void(const std::monostate &)>);

    // Explicit template instantiations for optimization types
    template Graph<std::set<CopyPropa::Copy>> initializeAnnotation<std::monostate, std::set<CopyPropa::Copy>>(const Graph<std::monostate> &, const std::set<CopyPropa::Copy> &);
    template Graph<std::set<std::string>> initializeAnnotation<std::monostate, std::set<std::string>>(const Graph<std::monostate> &, const std::set<std::string> &);

    template Graph<std::monostate> stripAnnotations<std::set<CopyPropa::Copy>>(const Graph<std::set<CopyPropa::Copy>> &);
    template Graph<std::monostate> stripAnnotations<std::set<std::string>>(const Graph<std::set<std::string>> &);
}