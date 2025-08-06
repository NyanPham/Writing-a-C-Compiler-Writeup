#ifndef INSTRUCTION_FIXUP_H
#define INSTRUCTION_FIXUP_H

#include <string>
#include <memory>
#include <vector>
#include <set>

#include "Assembly.h"
#include "AssemblySymbols.h"

class InstructionFixup
{
public:
    InstructionFixup(AssemblySymbols::AsmSymbolTable &asmSymbolTable) : _asmSymbolTable{asmSymbolTable} {};

    std::vector<std::shared_ptr<Assembly::Instruction>> fixupInstruction(
        const std::shared_ptr<Assembly::Instruction> &inst,
        const std::vector<Assembly::RegName> &calleeSavedRegs);

    std::shared_ptr<Assembly::TopLevel> fixupTopLevel(const std::shared_ptr<Assembly::TopLevel> &topLevel);
    std::shared_ptr<Assembly::Program> fixupProgram(const std::shared_ptr<Assembly::Program> &prog);

private:
    AssemblySymbols::AsmSymbolTable &_asmSymbolTable;
};

#endif