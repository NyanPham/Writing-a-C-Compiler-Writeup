#pragma once

#include <memory>
#include <optional>
#include <string>
#include "../Tacky.h"
#include "../Symbols.h"

namespace OptimizeUtils
{

    // Returns the destination value of a TACKY instruction, or nullopt if none.
    std::optional<std::shared_ptr<TACKY::Val>> getDst(const std::shared_ptr<TACKY::Instruction> &instr);

    // Returns true if the variable name is static (using the symbol table).
    bool isStatic(const std::string &varName, const Symbols::SymbolTable &symbolTable);

}