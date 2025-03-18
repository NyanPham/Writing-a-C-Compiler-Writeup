#ifndef REPLACE_PSEUDO_H
#define REPLACE_PSEUDO_H

#include <utility>
#include <map>
#include <string>
#include <memory>

#include "Assembly.h"

struct ReplacementState
{
    int currOffset;
    std::map<std::string, int> offsetMap;
};

using ReplaceOperandPair = std::pair<std::shared_ptr<Assembly::Operand>, ReplacementState>;
using ReplaceInstPair = std::pair<std::shared_ptr<Assembly::Instruction>, ReplacementState>;
using ReplaceFunctionPair = std::pair<std::shared_ptr<Assembly::Function>, int>;
using ReplaceProgramPair = std::pair<std::shared_ptr<Assembly::Program>, int>;

class ReplacePseudos
{
public:
    ReplacePseudos() = default;

    ReplacementState createInitState();
    ReplaceOperandPair replaceOperand(const std::shared_ptr<Assembly::Operand> &operand, ReplacementState &state);
    ReplaceInstPair replacePseudosInInstruction(const std::shared_ptr<Assembly::Instruction> &inst, ReplacementState &state);
    ReplaceFunctionPair replacePseudosInFunction(const std::shared_ptr<Assembly::Function> &func);
    ReplaceProgramPair replacePseudos(const std::shared_ptr<Assembly::Program> &prog);
};

#endif