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
    case TACKY::UnaryOp::Not:
        throw std::runtime_error("Internal Error: Cannot convert NOT operator directly from TACKY to Assembly!");
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
    case TACKY::BinaryOp::BitwiseAnd:
        return Assembly::BinaryOp::And;
    case TACKY::BinaryOp::BitwiseOr:
        return Assembly::BinaryOp::Or;
    case TACKY::BinaryOp::BitwiseXor:
        return Assembly::BinaryOp::Xor;
    case TACKY::BinaryOp::BitShiftLeft:
        return Assembly::BinaryOp::Sal;
    case TACKY::BinaryOp::BitShiftRight:
        return Assembly::BinaryOp::Sar;
    case TACKY::BinaryOp::Divide:
    case TACKY::BinaryOp::Remainder:
    case TACKY::BinaryOp::Equal:
    case TACKY::BinaryOp::NotEqual:
    case TACKY::BinaryOp::LessThan:
    case TACKY::BinaryOp::LessOrEqual:
    case TACKY::BinaryOp::GreaterThan:
    case TACKY::BinaryOp::GreaterOrEqual:
        throw std::runtime_error("Internal Error: Shouldn't handle division like other binary operators!");
    default:
        throw std::runtime_error("Internal Error: Unknown Binary Operators!");
    }
}

Assembly::CondCode CodeGen::convertCondCode(const TACKY::BinaryOp op)
{
    switch (op)
    {
    case TACKY::BinaryOp::Equal:
        return Assembly::CondCode::E;
    case TACKY::BinaryOp::NotEqual:
        return Assembly::CondCode::NE;
    case TACKY::BinaryOp::LessThan:
        return Assembly::CondCode::L;
    case TACKY::BinaryOp::LessOrEqual:
        return Assembly::CondCode::LE;
    case TACKY::BinaryOp::GreaterThan:
        return Assembly::CondCode::G;
    case TACKY::BinaryOp::GreaterOrEqual:
        return Assembly::CondCode::GE;
    default:
        throw std::runtime_error("Internal Error: Unknown binary to cond_code!");
    }
}

std::vector<std::shared_ptr<Assembly::Instruction>> CodeGen::convertInstruction(const std::shared_ptr<TACKY::Instruction> &inst)
{
    switch (inst->getType())
    {
    case TACKY::NodeType::Copy:
    {
        auto copyInst = std::dynamic_pointer_cast<TACKY::Copy>(inst);

        auto asmSrc = convertVal(copyInst->getSrc());
        auto asmDst = convertVal(copyInst->getDst());

        return {
            std::make_shared<Assembly::Mov>(asmSrc, asmDst),
        };
    }
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

        if (unaryInst->getOp() == TACKY::UnaryOp::Not)
        {
            auto asmSrc = convertVal(unaryInst->getSrc());
            auto asmDst = convertVal(unaryInst->getDst());

            return {
                std::make_shared<Assembly::Cmp>(std::make_shared<Assembly::Imm>(0), asmSrc),
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::Imm>(0), asmDst),
                std::make_shared<Assembly::SetCC>(Assembly::CondCode::E, asmDst),
            };
        }
        else
        {
            auto asmOp = convertUnop(unaryInst->getOp());
            auto asmSrc = convertVal(unaryInst->getSrc());
            auto asmDst = convertVal(unaryInst->getDst());

            return {
                std::make_shared<Assembly::Mov>(asmSrc, asmDst),
                std::make_shared<Assembly::Unary>(asmOp, asmDst),
            };
        }
    }
    case TACKY::NodeType::Binary:
    {
        auto binaryInst = std::dynamic_pointer_cast<TACKY::Binary>(inst);

        auto asmSrc1 = convertVal(binaryInst->getSrc1());
        auto asmSrc2 = convertVal(binaryInst->getSrc2());
        auto asmDst = convertVal(binaryInst->getDst());

        switch (binaryInst->getOp())
        {
        // Relational Operators
        case TACKY::BinaryOp::Equal:
        case TACKY::BinaryOp::NotEqual:
        case TACKY::BinaryOp::LessThan:
        case TACKY::BinaryOp::LessOrEqual:
        case TACKY::BinaryOp::GreaterThan:
        case TACKY::BinaryOp::GreaterOrEqual:
        {
            auto condCode = convertCondCode(binaryInst->getOp());

            return {
                std::make_shared<Assembly::Cmp>(asmSrc1, asmSrc2),
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::Imm>(0), asmDst),
                std::make_shared<Assembly::SetCC>(condCode, asmDst),
            };
        }

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

        // For Bit Shift instructions, source 2 can only be either in CX register or an Imm
        case TACKY::BinaryOp::BitShiftLeft:
        case TACKY::BinaryOp::BitShiftRight:
        {
            auto asmOp = convertBinop(binaryInst->getOp());

            if (asmSrc2->getType() == Assembly::NodeType::Imm)
            {
                return {
                    std::make_shared<Assembly::Mov>(asmSrc1, asmDst),
                    std::make_shared<Assembly::Binary>(asmOp, asmSrc2, asmDst),
                };
            }
            else
            {
                auto RegCX = std::make_shared<Assembly::Reg>(Assembly::RegName::CX);

                return {
                    std::make_shared<Assembly::Mov>(asmSrc1, asmDst),
                    std::make_shared<Assembly::Mov>(asmSrc2, RegCX),
                    std::make_shared<Assembly::Binary>(asmOp, RegCX, asmDst),
                };
            }
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
    case TACKY::NodeType::Jump:
    {
        return {
            std::make_shared<Assembly::Jmp>(std::dynamic_pointer_cast<TACKY::Jump>(inst)->getTarget()),
        };
    }
    case TACKY::NodeType::JumpIfZero:
    {
        auto jumpIfZeroInst = std::dynamic_pointer_cast<TACKY::JumpIfZero>(inst);
        auto asmCond = convertVal(jumpIfZeroInst->getCond());

        return {
            std::make_shared<Assembly::Cmp>(std::make_shared<Assembly::Imm>(0), asmCond),
            std::make_shared<Assembly::JmpCC>(Assembly::CondCode::E, jumpIfZeroInst->getTarget()),
        };
    }
    case TACKY::NodeType::JumpIfNotZero:
    {
        auto jumpIfNotZeroInst = std::dynamic_pointer_cast<TACKY::JumpIfNotZero>(inst);
        auto asmCond = convertVal(jumpIfNotZeroInst->getCond());

        return {
            std::make_shared<Assembly::Cmp>(std::make_shared<Assembly::Imm>(0), asmCond),
            std::make_shared<Assembly::JmpCC>(Assembly::CondCode::NE, jumpIfNotZeroInst->getTarget()),
        };
    }
    case TACKY::NodeType::Label:
    {
        return {
            std::make_shared<Assembly::Label>(std::dynamic_pointer_cast<TACKY::Label>(inst)->getName()),
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
