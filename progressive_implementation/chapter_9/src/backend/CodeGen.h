#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <memory>
#include <vector>

#include "TACKY.h"
#include "Assembly.h"
#include "Symbols.h"

class CodeGen
{
public:
    CodeGen(Symbols::SymbolTable &_symbolTable) : symbolTable(_symbolTable) {}

    std::vector<std::shared_ptr<Assembly::Instruction>> passParams(const std::vector<std::string> &params);

    std::shared_ptr<Assembly::Operand> convertVal(const std::shared_ptr<TACKY::Val> &val);
    Assembly::UnaryOp convertUnop(const TACKY::UnaryOp op);
    Assembly::BinaryOp convertBinop(const TACKY::BinaryOp op);
    Assembly::CondCode convertCondCode(const TACKY::BinaryOp op);
    std::vector<std::shared_ptr<Assembly::Instruction>> convertFunCall(const std::shared_ptr<TACKY::FunCall> &fnCall);
    std::vector<std::shared_ptr<Assembly::Instruction>> convertInstruction(const std::shared_ptr<TACKY::Instruction> &inst);
    std::shared_ptr<Assembly::Function> convertFunction(const std::shared_ptr<TACKY::Function> &fun);
    std::shared_ptr<Assembly::Program> gen(std::shared_ptr<TACKY::Program> prog);

private:
    Symbols::SymbolTable &symbolTable;

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