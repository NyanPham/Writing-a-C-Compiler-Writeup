#ifndef EMIT_H
#define EMIT_H

#include <string>
#include <memory>
#include "Assembly.h"

class Emit
{
public:
    Emit() {}

    std::string emitImm(std::shared_ptr<Assembly::Imm> imm);
    std::string emitReg(std::shared_ptr<Assembly::Register> reg);
    std::string showOperand(std::shared_ptr<Assembly::Operand> operand);
    std::string showLabel(const std::string &name);
    std::string emitRet(std::shared_ptr<Assembly::Ret> ret);
    std::string emitMov(std::shared_ptr<Assembly::Mov> mov);
    std::string emitInst(std::shared_ptr<Assembly::Instruction> inst);
    std::string emitFunction(std::shared_ptr<Assembly::Function> fun);
    std::string emitProgram(std::shared_ptr<Assembly::Program> prog);
    std::string emitStackNote();
    void emit(std::shared_ptr<Assembly::Program> prog, const std::string &output);
};

#endif