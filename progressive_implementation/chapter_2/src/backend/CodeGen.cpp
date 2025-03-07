#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

#include "CodeGen.h"
#include "TACKY.h"
#include "Assembly.h"

std::shared_ptr<Assembly::Operand> CodeGen::convertVal(const std::shared_ptr<TACKY::Val> &val)
{
    switch (val->getType())
    {
    case TACKY::NodeType::Constant:
    {
        return std::make_shared<Assembly::Imm>(std::dynamic_pointer_cast<TACKY::Constant>(val)->getValue());
    }
    case TACKY::NodeType::Var:
    {
        return std::make_shared<Assembly::Pseudo>(std::dynamic_pointer_cast<TACKY::Var>(val)->getName());
    }
    default:
    {
        throw std::runtime_error("Internal error: Invalid value to convert to assembly");
    }
    }
}

Assembly::UnaryOp CodeGen::convertOp(const TACKY::UnaryOp op)
{
    switch (op)
    {
    case TACKY::UnaryOp::Complement:
    {
        return Assembly::UnaryOp::Not;
    }
    case TACKY::UnaryOp::Negate:
    {
        return Assembly::UnaryOp::Neg;
    }
    default:
    {
        throw std::runtime_error("Invalid unary operator");
    }
    }
}

std::vector<std::shared_ptr<Assembly::Instruction>> CodeGen::convertInstruction(const std::shared_ptr<TACKY::Instruction> &inst)
{
    switch (inst->getType())
    {
    case TACKY::NodeType::Return:
    {
        auto asmVal = convertVal(std::dynamic_pointer_cast<TACKY::Return>(inst)->getValue());

        return {
            std::make_shared<Assembly::Mov>(asmVal, std::make_shared<Assembly::Reg>(Assembly::RegName::AX)),
            std::make_shared<Assembly::Ret>(),
        };
    }
    case TACKY::NodeType::Unary:
    {
        auto unaryInst = std::dynamic_pointer_cast<TACKY::Unary>(inst);

        auto asmOp = convertOp(unaryInst->getOp());
        auto asmSrc = convertVal(unaryInst->getSrc());
        auto asmDst = convertVal(unaryInst->getDst());

        return {
            std::make_shared<Assembly::Mov>(asmSrc, asmDst),
            std::make_shared<Assembly::Unary>(asmOp, asmDst),
        };
    }
    default:
        throw std::runtime_error("Internal Error: Invalid TACKY instruction");
    }
}

std::shared_ptr<Assembly::Function> CodeGen::convertFunction(const std::shared_ptr<TACKY::Function> &fun)
{
    std::vector<std::shared_ptr<Assembly::Instruction>> instructions{};

    for (auto &inst : fun->getInstructions())
    {
        auto asmInstructions = convertInstruction(inst);
        instructions.insert(instructions.end(), asmInstructions.begin(), asmInstructions.end());
    }

    return std::make_shared<Assembly::Function>(fun->getName(), instructions);
}

std::shared_ptr<Assembly::Program> CodeGen::gen(std::shared_ptr<TACKY::Program> prog)
{
    auto asmFunction = convertFunction(prog->getFunction());
    return std::make_shared<Assembly::Program>(asmFunction);
}
