#pragma once

#include <vector>
#include <string>
#include <memory>
#include "../Tacky.h"

// Performs constant folding on a list of TACKY instructions.
std::vector<std::shared_ptr<TACKY::Instruction>>
constantFold(const std::string &debugLabel,
             const std::vector<std::shared_ptr<TACKY::Instruction>> &instructions,
             const Symbols::SymbolTable &symbolTable,
             bool debug);