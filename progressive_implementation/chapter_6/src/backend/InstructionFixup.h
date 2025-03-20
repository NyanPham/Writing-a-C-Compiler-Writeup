#ifndef INSTRUCTION_FIXUP_H
#define INSTRUCTION_FIXUP_H

#include <string>
#include <memory>
#include <vector>

#include "Assembly.h"

class InstructionFixup
{
public:
    InstructionFixup() = default;

    std::vector<std::shared_ptr<Assembly::Instruction>> fixupInstruction(const std::shared_ptr<Assembly::Instruction> &inst);
    std::shared_ptr<Assembly::Function> fixupFunction(const std::shared_ptr<Assembly::Function> &fun, int lastStackSlot);
    std::shared_ptr<Assembly::Program> fixupProgram(const std::shared_ptr<Assembly::Program> &prog, int lastStackSlot);
};

#endif