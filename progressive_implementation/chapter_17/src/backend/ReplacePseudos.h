#ifndef REPLACE_PSEUDO_H
#define REPLACE_PSEUDO_H

#include <utility>
#include <map>
#include <string>
#include <memory>

#include "Assembly.h"
#include "AssemblySymbols.h"

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
    ReplacePseudos(AssemblySymbols::AsmSymbolTable &asmSymbolTable) : _asmSymbolTable{asmSymbolTable} {};

    std::pair<int, ReplacementState> calculateOffset(const std::string &name, ReplacementState &state);
    ReplacementState createInitState();
    ReplaceOperandPair replaceOperand(const std::shared_ptr<Assembly::Operand> &operand, ReplacementState &state);
    ReplaceInstPair replacePseudosInInstruction(const std::shared_ptr<Assembly::Instruction> &inst, ReplacementState &state);
    std::shared_ptr<Assembly::TopLevel> replacePseudosInTopLevel(const std::shared_ptr<Assembly::TopLevel> &topLevel);
    std::shared_ptr<Assembly::Program> replacePseudos(const std::shared_ptr<Assembly::Program> &prog);

private:
    AssemblySymbols::AsmSymbolTable &_asmSymbolTable;
};

#endif