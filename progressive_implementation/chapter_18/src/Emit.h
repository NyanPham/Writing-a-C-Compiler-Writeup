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
    std::string escape(const std::string &s);
    std::string emitInit(const Initializers::StaticInit &init);
    std::string alignDirective();
    std::string emitGlobalDirective(bool global, const std::string &label);
    std::string showLongReg(const std::shared_ptr<Assembly::Reg> &reg);
    std::string showOperand(const std::shared_ptr<Assembly::AsmType> &asmType, const std::shared_ptr<Assembly::Operand> &operand);
    std::string showByteReg(const std::shared_ptr<Assembly::Reg> &reg);
    std::string showByteOperand(const std::shared_ptr<Assembly::Operand> &operand);
    std::string showQuadwordReg(const std::shared_ptr<Assembly::Reg> &reg);
    std::string showQuadwordOperand(const std::shared_ptr<Assembly::Operand> &operand);
    std::string showDoubleReg(const std::shared_ptr<Assembly::Reg> &reg);
    std::string showFunName(const std::shared_ptr<Assembly::Call> &fnCall);
    std::string showUnaryOp(Assembly::UnaryOp op);
    std::string showBinaryOp(Assembly::BinaryOp op);
    std::string showLabel(const std::string &name);
    std::string showLocalLabel(const std::string &name);
    std::string showCondCode(Assembly::CondCode condCode);
    std::string emitInst(std::shared_ptr<Assembly::Instruction> inst);
    std::string emitConstant(const std::string &name, size_t alignement, const Initializers::StaticInit &init);
    std::string emitTopLevel(std::shared_ptr<Assembly::TopLevel> topLevel);
    std::string emitStackNote();
    void emit(std::shared_ptr<Assembly::Program> prog, const std::string &output);

private:
    AssemblySymbols::AsmSymbolTable &_asmSymbolTable;
};

#endif