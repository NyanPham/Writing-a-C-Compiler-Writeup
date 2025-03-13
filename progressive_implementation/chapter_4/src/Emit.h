#ifndef EMIT_H
#define EMIT_H

#include <string>
#include <memory>
#include "Assembly.h"

enum class OperandSize
{
    Byte1,
    Byte4,
};

class Emit
{
public:
    Emit() {}

    std::string emitReg(std::shared_ptr<Assembly::Reg> reg, OperandSize size);
    std::string emitImm(std::shared_ptr<Assembly::Imm> imm);
    std::string emitStack(std::shared_ptr<Assembly::Stack> stack);
    std::string emitPseudo(std::shared_ptr<Assembly::Pseudo> pseudo);
    std::string showOperand(std::shared_ptr<Assembly::Operand> operand);
    std::string showByteOperand(std::shared_ptr<Assembly::Operand> operand);
    std::string showUnaryOp(Assembly::UnaryOp op);
    std::string showBinaryOp(Assembly::BinaryOp op);
    std::string showLabel(const std::string &name);
    std::string showLocalLabel(const std::string &name);
    std::string showCondCode(Assembly::CondCode condCode);
    std::string emitMov(std::shared_ptr<Assembly::Mov> mov);
    std::string emitUnary(std::shared_ptr<Assembly::Unary> unary);
    std::string emitBinary(std::shared_ptr<Assembly::Binary> binary);
    std::string emitCmp(std::shared_ptr<Assembly::Cmp> cmp);
    std::string emitIdiv(std::shared_ptr<Assembly::Idiv> idiv);
    std::string emitCdq(std::shared_ptr<Assembly::Cdq> cdq);
    std::string emitJmp(std::shared_ptr<Assembly::Jmp> jmp);
    std::string emitJmpCC(std::shared_ptr<Assembly::JmpCC> jmpCC);
    std::string emitSetCC(std::shared_ptr<Assembly::SetCC> setCC);
    std::string emitLabel(std::shared_ptr<Assembly::Label> label);
    std::string emitAllocateStack(std::shared_ptr<Assembly::AllocateStack> allocateStack);
    std::string emitRet(std::shared_ptr<Assembly::Ret> ret);
    std::string emitInst(std::shared_ptr<Assembly::Instruction> inst);
    std::string emitFunction(std::shared_ptr<Assembly::Function> fun);
    std::string emitProgram(std::shared_ptr<Assembly::Program> prog);
    std::string emitStackNote();
    void emit(std::shared_ptr<Assembly::Program> prog, const std::string &output);
};

#endif