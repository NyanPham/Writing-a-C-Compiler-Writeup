#ifndef CFG_H
#define CFG_H

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <functional>
#include <iostream>
#include <variant>

namespace TACKY
{
    class Instruction;
}
namespace Assembly
{
    class Instruction;
}

namespace cfg
{
    struct NodeID
    {
        enum class Kind
        {
            Entry,
            Block,
            Exit
        } kind;
        int index;

        static NodeID Entry() { return {Kind::Entry, -1}; }
        static NodeID Exit() { return {Kind::Exit, -1}; }
        static NodeID Block(int idx) { return {Kind::Block, idx}; }

        bool operator==(const NodeID &other) const { return kind == other.kind && index == other.index; }
        bool operator!=(const NodeID &other) const { return !(*this == other); }
        bool operator<(const NodeID &other) const { return kind != other.kind ? kind < other.kind : index < other.index; }
    };

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

    template <typename V, typename I>
    struct BasicBlock
    {
        NodeID id;
        std::vector<std::pair<V, std::shared_ptr<I>>> instructions;
        std::vector<NodeID> preds;
        std::vector<NodeID> succs;
        V value;
    };

    template <typename V, typename I>
    struct Graph
    {
        std::map<int, BasicBlock<V, I>> basicBlocks;
        std::vector<NodeID> entrySuccs;
        std::vector<NodeID> exitPreds;
        std::string debugLabel;
    };

    // Partition instructions into basic blocks
    template <typename I>
    std::vector<std::vector<std::shared_ptr<I>>> partitionIntoBasicBlocks(const std::vector<std::shared_ptr<I>> &instructions);

    // Build a CFG from a list of instructions
    template <typename I>
    Graph<std::monostate, I> instructionsToCFG(const std::string &debugLabel, const std::vector<std::shared_ptr<I>> &instructions);

    // Convert CFG back to instructions
    template <typename I>
    std::vector<std::shared_ptr<I>> cfgToInstructions(const Graph<std::monostate, I> &g);

    // Get successors for a node
    template <typename V, typename I>
    std::vector<NodeID> getSuccs(const NodeID &id, const Graph<V, I> &g);

    // Get block annotation value
    template <typename V, typename I>
    V getBlockValue(int blocknum, const Graph<V, I> &g);

    // Update a basic block in the CFG
    template <typename V, typename I>
    Graph<V, I> updateBasicBlock(int blockIdx, const BasicBlock<V, I> &newBlock, const Graph<V, I> &g);

    // Initialize annotation for all blocks
    template <typename VA, typename VB, typename I>
    Graph<VB, I> initializeAnnotation(const Graph<VA, I> &cfg, const VB &dummyVal);

    // Strip all annotations (set to std::monostate)
    template <typename VA, typename I>
    Graph<std::monostate, I> stripAnnotations(const Graph<VA, I> &cfg);

    // For debugging: print the CFG (simple version)
    template <typename V, typename I>
    void printCFG(const Graph<V, I> &cfg, std::function<void(const V &)> printVal);

    // For debugging: print the CFG with pretty-printed instructions and annotations
    template <typename V, typename I>
    void printCFGPretty(const Graph<V, I> &cfg, std::function<void(const V &)> printVal);

    // For debugging: print the CFG with pretty-printed instructions (no annotation)
    template <typename I>
    void printCFGPretty(const Graph<std::monostate, I> &cfg);
}

#endif