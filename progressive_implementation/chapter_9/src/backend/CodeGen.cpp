#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include "CodeGen.h"
#include "TACKY.h"
#include "Assembly.h"

std::vector<std::shared_ptr<Assembly::Instruction>> CodeGen::passParams(const std::vector<std::string> &params)
{
    std::vector<std::string> regParams(
        params.begin(),
        params.begin() + std::min<size_t>(params.size(), 6));

    std::vector<std::string> stackParams(
        params.begin() + std::min<size_t>(params.size(), 6),
        params.end());

    std::vector<std::shared_ptr<Assembly::Instruction>> insts{};

    // pass params in regsiters
    for (int i = 0; i < regParams.size(); i++)
    {
        auto r = PARAM_PASSING_REGS[i];
        auto asmParam = std::make_shared<Assembly::Pseudo>(regParams[i]);
        insts.push_back(std::make_shared<Assembly::Mov>(std::make_shared<Assembly::Reg>(r), asmParam));
    }

    // pass params on the stack
    for (int i = 0; i < stackParams.size(); i++)
    {
        auto stack = std::make_shared<Assembly::Stack>(16 + 8 * i);
        auto asmParam = std::make_shared<Assembly::Pseudo>(stackParams[i]);
        insts.push_back(std::make_shared<Assembly::Mov>(stack, asmParam));
    }

    return insts;
}

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

std::vector<std::shared_ptr<Assembly::Instruction>> CodeGen::convertFunCall(const std::shared_ptr<TACKY::FunCall> &fnCall)
{
    std::vector<std::shared_ptr<TACKY::Val>> regArgs(
        fnCall->getArgs().begin(),
        fnCall->getArgs().begin() + std::min<size_t>(fnCall->getArgs().size(), 6));

    std::vector<std::shared_ptr<TACKY::Val>> stackArgs(
        fnCall->getArgs().begin() + std::min<size_t>(fnCall->getArgs().size(), 6),
        fnCall->getArgs().end());

    std::vector<std::shared_ptr<Assembly::Instruction>> insts{};

    // adjust stack alignment
    int stackPadding = stackArgs.size() % 2 == 0 ? 0 : 8;
    if (stackPadding != 0)
        insts.push_back(std::make_shared<Assembly::AllocateStack>(stackPadding));

    // pass arguments in registers
    for (int i{0}; i < regArgs.size(); i++)
    {
        auto r = PARAM_PASSING_REGS[i];
        auto asmArg = convertVal(regArgs[i]);
        insts.push_back(std::make_shared<Assembly::Mov>(asmArg, std::make_shared<Assembly::Reg>(r)));
    }

    // pass arguments on the stack
    std::reverse(stackArgs.begin(), stackArgs.end());
    for (const auto &tackyArg : stackArgs)
    {
        auto asmArg = convertVal(tackyArg);
        if (asmArg->getType() == Assembly::NodeType::Reg || asmArg->getType() == Assembly::NodeType::Imm)
        {
            insts.push_back(std::make_shared<Assembly::Push>(asmArg));
        }
        else
        {
            // Copy into a register before pushing
            insts.push_back(std::make_shared<Assembly::Mov>(asmArg, std::make_shared<Assembly::Reg>(Assembly::RegName::AX)));
            insts.push_back(std::make_shared<Assembly::Push>(std::make_shared<Assembly::Reg>(Assembly::RegName::AX)));
        }
    }

    // emit call function
    insts.push_back(std::make_shared<Assembly::Call>(fnCall->getFnName()));

    // adjust stack pointer
    auto bytesToRemove = 8 * (stackArgs.size()) + stackPadding;
    if (bytesToRemove != 0)
        insts.push_back(std::make_shared<Assembly::DeallocateStack>(bytesToRemove));

    // retrieve return value
    auto asmDst = convertVal(fnCall->getDst());
    insts.push_back(std::make_shared<Assembly::Mov>(
        std::make_shared<Assembly::Reg>(Assembly::RegName::AX),
        asmDst));

    return insts;
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
    case TACKY::NodeType::FunCall:
    {
        return convertFunCall(std::dynamic_pointer_cast<TACKY::FunCall>(inst));
    }
    default:
        throw std::runtime_error("Internal Error: Invalid TACKY instruction");
    }
}

std::shared_ptr<Assembly::Function> CodeGen::convertFunction(const std::shared_ptr<TACKY::Function> &fn)
{
    std::vector<std::shared_ptr<Assembly::Instruction>> insts{};

    auto paramInsts = passParams(fn->getParams());
    insts.insert(insts.end(), paramInsts.begin(), paramInsts.end());

    for (auto &inst : fn->getInstructions())
    {
        auto asmInsts = convertInstruction(inst);
        insts.insert(insts.end(), asmInsts.begin(), asmInsts.end());
    }

    return std::make_shared<Assembly::Function>(fn->getName(), insts);
}

std::shared_ptr<Assembly::Program> CodeGen::gen(std::shared_ptr<TACKY::Program> prog)
{
    std::vector<std::shared_ptr<Assembly::Function>> convertedFns{};

    for (const auto &fn : prog->getFunctions())
    {
        auto asmFnDef = convertFunction(fn);
        convertedFns.push_back(asmFnDef);
    }

    return std::make_shared<Assembly::Program>(convertedFns);
}
