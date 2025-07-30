#pragma once

#include <set>
#include <string>
#include <variant>
#include "../CFG.h"
#include "../Symbols.h"

// Remove dead stores from a TACKY CFG.
cfg::Graph<std::monostate, TACKY::Instruction> eliminateDeadStores(
    const std::set<std::string> &aliasedVars,
    const cfg::Graph<std::monostate, TACKY::Instruction> &cfg,
    const Symbols::SymbolTable &symbolTable,
    bool debug);