#pragma once

#include <set>
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <variant>
#include "../CFG.h"
#include "../Symbols.h"
#include "../TACKY.h"

namespace CopyPropa
{

    struct Copy
    {
        std::shared_ptr<TACKY::Val> src;
        std::shared_ptr<TACKY::Val> dst;

        bool operator<(const Copy &other) const;
        bool operator==(const Copy &other) const;
    };

    void printCopy(const Copy &copy, std::ostream &os);

    // Set of reaching copies
    using ReachingCopies = std::set<Copy>;

    // Print the annotated CFG for debugging
    void printGraph(const cfg::Graph<ReachingCopies, TACKY::Instruction> &cfg, const std::string &extraTag = "");

    // Main entry: optimize copy propagation
    cfg::Graph<std::monostate, TACKY::Instruction> optimize(
        const std::set<std::string> &aliasedVars,
        const cfg::Graph<std::monostate, TACKY::Instruction> &cfg,
        const Symbols::SymbolTable &symbolTable,
        bool debug);

}