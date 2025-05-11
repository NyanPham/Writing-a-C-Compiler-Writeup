#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <memory>
#include <vector>

#include "TACKY.h"
#include "Assembly.h"
#include "Symbols.h"
#include "AssemblySymbols.h"

class CodeGen
{
public:
    CodeGen(Symbols::SymbolTable &symbolTable) : _symbolTable(symbolTable) {}

    std::shared_ptr<Types::DataType> tackyType(const std::shared_ptr<TACKY::Val> &operand);
    std::shared_ptr<Assembly::AsmType> convertType(const Types::DataType &type);
    std::shared_ptr<Assembly::AsmType> getAsmType(const std::shared_ptr<TACKY::Val> &operand);
    std::vector<std::shared_ptr<Assembly::Instruction>> passParams(const std::vector<std::string> &params);

    std::shared_ptr<Assembly::Operand> convertVal(const std::shared_ptr<TACKY::Val> &val);
    Assembly::UnaryOp convertUnop(const TACKY::UnaryOp op);
    Assembly::BinaryOp convertBinop(const TACKY::BinaryOp op);
    Assembly::BinaryOp convertShiftOp(const TACKY::BinaryOp op, bool isSigned);
    Assembly::CondCode convertCondCode(const TACKY::BinaryOp op, bool isSigned);
    std::vector<std::shared_ptr<Assembly::Instruction>> convertFunCall(const std::shared_ptr<TACKY::FunCall> &fnCall);
    std::vector<std::shared_ptr<Assembly::Instruction>> convertInstruction(const std::shared_ptr<TACKY::Instruction> &inst);
    std::shared_ptr<Assembly::TopLevel> convertTopLevel(const std::shared_ptr<TACKY::TopLevel> &topLevel);
    void convertSymbol(const std::string &name, const Symbols::Symbol &symbol);
    std::shared_ptr<Assembly::Program> gen(std::shared_ptr<TACKY::Program> prog);

    AssemblySymbols::AsmSymbolTable &getAsmSymbolTable() { return _asmSymbolTable; }

private:
    Symbols::SymbolTable &_symbolTable;
    AssemblySymbols::AsmSymbolTable _asmSymbolTable;

    const std::vector<Assembly::RegName> PARAM_PASSING_REGS{
        Assembly::RegName::DI,
        Assembly::RegName::SI,
        Assembly::RegName::DX,
        Assembly::RegName::CX,
        Assembly::RegName::R8,
        Assembly::RegName::R9,
    };
};

#endif