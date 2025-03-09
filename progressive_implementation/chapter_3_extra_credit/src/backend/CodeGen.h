#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <memory>
#include <vector>

#include "TACKY.h"
#include "Assembly.h"

class CodeGen
{
public:
    CodeGen() {}

    std::shared_ptr<Assembly::Operand> convertVal(const std::shared_ptr<TACKY::Val> &val);
    Assembly::UnaryOp convertUnop(const TACKY::UnaryOp op);
    Assembly::BinaryOp convertBinop(const TACKY::BinaryOp op);
    std::vector<std::shared_ptr<Assembly::Instruction>> convertInstruction(const std::shared_ptr<TACKY::Instruction> &inst);
    std::shared_ptr<Assembly::Function> convertFunction(const std::shared_ptr<TACKY::Function> &fun);
    std::shared_ptr<Assembly::Program> gen(std::shared_ptr<TACKY::Program> prog);
};

#endif