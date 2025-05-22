#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>

#include "TACKY.h"
#include "Assembly.h"
#include "Symbols.h"
#include "AssemblySymbols.h"

class CodeGen
{
public:
    CodeGen(Symbols::SymbolTable &symbolTable) : _symbolTable(symbolTable) {}

    std::vector<std::shared_ptr<Assembly::Instruction>> convertDblComparison(TACKY::BinaryOp op, const std::shared_ptr<Assembly::AsmType> &dstType, std::shared_ptr<Assembly::Operand> &asmSrc1, std::shared_ptr<Assembly::Operand> &asmSrc2, const std::shared_ptr<Assembly::Operand> &asmDst);
    std::shared_ptr<Assembly::StaticConstant> convertConstant(double key, const std::pair<std::string, size_t> &constant);
    std::tuple<
        std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>>,
        std::vector<std::shared_ptr<Assembly::Operand>>,
        std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>>>
    classifyParameters(const std::vector<std::shared_ptr<TACKY::Val>> &tackyVals);
    std::string addConstant(double dbl, size_t alignment = 8);
    std::shared_ptr<Types::DataType> tackyType(const std::shared_ptr<TACKY::Val> &operand);
    std::shared_ptr<Assembly::AsmType> convertType(const Types::DataType &type);
    std::shared_ptr<Assembly::AsmType> getAsmType(const std::shared_ptr<TACKY::Val> &operand);
    std::vector<std::shared_ptr<Assembly::Instruction>> passParams(const std::vector<std::shared_ptr<TACKY::Val>> &params);

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
    std::unordered_map<double, std::pair<std::string, size_t>> _constants;

    const std::vector<Assembly::RegName> INT_PARAM_PASSING_REGS{
        Assembly::RegName::DI,
        Assembly::RegName::SI,
        Assembly::RegName::DX,
        Assembly::RegName::CX,
        Assembly::RegName::R8,
        Assembly::RegName::R9,
    };

    const std::vector<Assembly::RegName> DBL_PARAM_PASSING_REGS{
        Assembly::RegName::XMM0,
        Assembly::RegName::XMM1,
        Assembly::RegName::XMM2,
        Assembly::RegName::XMM3,
        Assembly::RegName::XMM4,
        Assembly::RegName::XMM5,
        Assembly::RegName::XMM6,
        Assembly::RegName::XMM7,
    };
};

#endif