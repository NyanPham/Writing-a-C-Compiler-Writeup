#ifndef CFG_H
#define CFG_H

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <optional>
#include <functional>
#include <iostream>
#include <variant>
#include "Tacky.h"

namespace cfg
{

    // NodeID for CFG
    struct NodeID
    {
        enum class Kind
        {
            Entry,
            Block,
            Exit
        } kind;
        int index; // Only valid if kind == Block

        static NodeID Entry() { return {Kind::Entry, -1}; }
        static NodeID Exit() { return {Kind::Exit, -1}; }
        static NodeID Block(int idx) { return {Kind::Block, idx}; }

        bool operator==(const NodeID &other) const
        {
            return kind == other.kind && index == other.index;
        }
        bool operator!=(const NodeID &other) const { return !(*this == other); }
        bool operator<(const NodeID &other) const
        {
            if (kind != other.kind)
                return kind < other.kind;
            return index < other.index;
        }
    };

    // Add this operator<< overload:
    inline std::ostream &operator<<(std::ostream &os, const NodeID &id)
    {
        switch (id.kind)
        {
        case NodeID::Kind::Entry:
            os << "Entry";
            break;
        case NodeID::Kind::Exit:
            os << "Exit";
            break;
        case NodeID::Kind::Block:
            os << "Block" << id.index;
            break;
        default:
            os << "Unknown";
            break;
        }
        return os;
    }

    // Basic block parameterized by annotation type V
    template <typename V>
    struct BasicBlock
    {
        NodeID id;
        std::vector<std::pair<V, std::shared_ptr<TACKY::Instruction>>> instructions;
        std::vector<NodeID> preds;
        std::vector<NodeID> succs;
        V value;
    };

    // CFG parameterized by annotation type V
    template <typename V>
    struct Graph
    {
        std::map<int, BasicBlock<V>> basicBlocks;
        std::vector<NodeID> entrySuccs;
        std::vector<NodeID> exitPreds;
        std::string debugLabel;
    };

    // Partition instructions into basic blocks
    std::vector<std::vector<std::shared_ptr<TACKY::Instruction>>>
    partitionIntoBasicBlocks(const std::vector<std::shared_ptr<TACKY::Instruction>> &instructions);

    // Build a CFG from a list of instructions
    Graph<std::monostate> instructionsToCFG(const std::string &debugLabel, const std::vector<std::shared_ptr<TACKY::Instruction>> &instructions);

    // Convert CFG back to instructions
    std::vector<std::shared_ptr<TACKY::Instruction>> cfgToInstructions(const Graph<std::monostate> &g);

    // Get successors for a node
    template <typename V>
    std::vector<NodeID> getSuccs(const NodeID &id, const Graph<V> &g);

    // Get block annotation value
    template <typename V>
    V getBlockValue(int blocknum, const Graph<V> &g);

    // Update a basic block in the CFG
    template <typename V>
    Graph<V> updateBasicBlock(int blockIdx, const BasicBlock<V> &newBlock, const Graph<V> &g);

    // Initialize annotation for all blocks
    template <typename VA, typename VB>
    Graph<VB> initializeAnnotation(const Graph<VA> &cfg, const VB &dummyVal);

    // Strip all annotations (set to std::monostate)
    template <typename VA>
    Graph<std::monostate> stripAnnotations(const Graph<VA> &cfg);

    // For debugging: print the CFG (simple version)
    template <typename V>
    void printCFG(const Graph<V> &cfg, std::function<void(const V &)> printVal);

    // For debugging: print the CFG with pretty-printed instructions and annotations
    template <typename V>
    void printCFGPretty(const Graph<V> &cfg, std::function<void(const V &)> printVal);

    void printCFGPretty(const Graph<std::monostate> &cfg);
}

#endif