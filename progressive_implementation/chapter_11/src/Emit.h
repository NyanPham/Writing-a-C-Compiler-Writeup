#ifndef EMIT_H
#define EMIT_H

#include <string>
#include <memory>
#include "Assembly.h"
#include "Initializers.h"
#include "./backend/AssemblySymbols.h"

enum class OperandSize
{
    Byte1,
    Byte4,
};

class Emit
{
public:
    Emit(AssemblySymbols::AsmSymbolTable &asmSymbolTable) : _asmSymbolTable{asmSymbolTable} {};

    std::string suffix(const std::shared_ptr<Assembly::AsmType> &asmType);
    std::string emitZeroInit(const Initializers::StaticInit &init);
    std::string emitInit(const Initializers::StaticInit &init);
    std::string alignDirective();
    std::string emitGlobalDirective(bool global, const std::string &label);
    std::string showLongReg(const std::shared_ptr<Assembly::Reg> &reg);
    std::string showOperand(const std::shared_ptr<Assembly::AsmType> &asmType, const std::shared_ptr<Assembly::Operand> &operand);
    std::string showByteReg(const std::shared_ptr<Assembly::Reg> &reg);
    std::string showByteOperand(const std::shared_ptr<Assembly::Operand> &operand);
    std::string showQuadwordReg(const std::shared_ptr<Assembly::Reg> &reg);
    std::string showQuadwordOperand(const std::shared_ptr<Assembly::Operand> &operand);
    std::string showFunName(const std::shared_ptr<Assembly::Call> &fnCall);
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
    std::string emitPush(std::shared_ptr<Assembly::Push> push);
    std::string emitCall(std::shared_ptr<Assembly::Call> call);
    std::string emitMovsx(std::shared_ptr<Assembly::Movsx> movsx);
    std::string emitRet(std::shared_ptr<Assembly::Ret> ret);
    std::string emitInst(std::shared_ptr<Assembly::Instruction> inst);
    std::string emitTopLevel(std::shared_ptr<Assembly::TopLevel> topLevel);
    std::string emitStackNote();
    void emit(std::shared_ptr<Assembly::Program> prog, const std::string &output);

private:
    AssemblySymbols::AsmSymbolTable &_asmSymbolTable;
};

#endif