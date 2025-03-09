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

Assembly::UnaryOp CodeGen::convertUnop(const TACKY::UnaryOp op)
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

Assembly::BinaryOp CodeGen::convertBinop(const TACKY::BinaryOp op)
{
    switch (op)
    {
    case TACKY::BinaryOp::Add:
        return Assembly::BinaryOp::Add;
    case TACKY::BinaryOp::Subtract:
        return Assembly::BinaryOp::Sub;
    case TACKY::BinaryOp::Multiply:
        return Assembly::BinaryOp::Mult;
    case TACKY::BinaryOp::Divide:
    case TACKY::BinaryOp::Remainder:
        throw std::runtime_error("Internal Error: Shouldn't handle division like other binary operators!");
    default:
        throw std::runtime_error("Internal Error: Unknown Binary Operators!");
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

        auto asmOp = convertUnop(unaryInst->getOp());
        auto asmSrc = convertVal(unaryInst->getSrc());
        auto asmDst = convertVal(unaryInst->getDst());

        return {
            std::make_shared<Assembly::Mov>(asmSrc, asmDst),
            std::make_shared<Assembly::Unary>(asmOp, asmDst),
        };
    }
    case TACKY::NodeType::Binary:
    {
        auto binaryInst = std::dynamic_pointer_cast<TACKY::Binary>(inst);

        auto asmSrc1 = convertVal(binaryInst->getSrc1());
        auto asmSrc2 = convertVal(binaryInst->getSrc2());
        auto asmDst = convertVal(binaryInst->getDst());

        switch (binaryInst->getOp())
        {

        // For Division/Modulo
        case TACKY::BinaryOp::Divide:
        case TACKY::BinaryOp::Remainder:
        {
            Assembly::RegName resultRegName =
                binaryInst->getOp() == TACKY::BinaryOp::Divide
                    ? Assembly::RegName::AX
                    : Assembly::RegName::DX;

            return {
                std::make_shared<Assembly::Mov>(asmSrc1, std::make_shared<Assembly::Reg>(Assembly::RegName::AX)),
                std::make_shared<Assembly::Cdq>(),
                std::make_shared<Assembly::Idiv>(asmSrc2),
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::Reg>(resultRegName), asmDst),
            };
        }

        // Addition/Subtraction/Multiplication
        default:
        {
            auto asmOp = convertBinop(binaryInst->getOp());

            return {
                std::make_shared<Assembly::Mov>(asmSrc1, asmDst),
                std::make_shared<Assembly::Binary>(asmOp, asmSrc2, asmDst),
            };
        }
        }
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
