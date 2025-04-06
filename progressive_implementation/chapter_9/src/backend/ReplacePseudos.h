#ifndef REPLACE_PSEUDO_H
#define REPLACE_PSEUDO_H

#include <utility>
#include <map>
#include <string>
#include <memory>

#include "Assembly.h"
#include "Symbols.h"

struct ReplacementState
{
    int currOffset;
    std::map<std::string, int> offsetMap;
};

using ReplaceOperandPair = std::pair<std::shared_ptr<Assembly::Operand>, ReplacementState>;
using ReplaceInstPair = std::pair<std::shared_ptr<Assembly::Instruction>, ReplacementState>;

class ReplacePseudos
{
public:
    ReplacePseudos(Symbols::SymbolTable &symbolTable) : _symbolTable{symbolTable} {};

    ReplacementState createInitState();
    ReplaceOperandPair replaceOperand(const std::shared_ptr<Assembly::Operand> &operand, ReplacementState &state);
    ReplaceInstPair replacePseudosInInstruction(const std::shared_ptr<Assembly::Instruction> &inst, ReplacementState &state);
    std::shared_ptr<Assembly::Function> replacePseudosInFunction(const std::shared_ptr<Assembly::Function> &func);
    std::shared_ptr<Assembly::Program> replacePseudos(const std::shared_ptr<Assembly::Program> &prog);

private:
    Symbols::SymbolTable &_symbolTable;
};

#endif