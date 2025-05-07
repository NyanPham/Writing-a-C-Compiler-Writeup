#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include "CodeGen.h"
#include "TACKY.h"
#include "Assembly.h"
#include "AssemblySymbols.h"

std::shared_ptr<Assembly::Imm> zero()
{
    return std::make_shared<Assembly::Imm>(0);
}

std::shared_ptr<Assembly::AsmType>
CodeGen::convertType(const Types::DataType &type)
{
    if (Types::isIntType(type))
        return std::make_shared<Assembly::AsmType>(Assembly::Longword());
    else if (Types::isLongType(type))
        return std::make_shared<Assembly::AsmType>(Assembly::Quadword());
    else
        throw std::runtime_error("Internal error: converting function type to assembly");
}

std::shared_ptr<Assembly::AsmType>
CodeGen::getAsmType(const std::shared_ptr<TACKY::Val> &operand)
{
    if (auto constant = std::dynamic_pointer_cast<TACKY::Constant>(operand))
    {
        if (Constants::isConstInt(*constant->getConst()))
            return std::make_shared<Assembly::AsmType>(Assembly::Longword());
        else if (Constants::isConstLong(*constant->getConst()))
            return std::make_shared<Assembly::AsmType>(Assembly::Quadword());
        else
            throw std::runtime_error("Internal error: Invalid constant to get asm type");
    }
    else if (auto var = std::dynamic_pointer_cast<TACKY::Var>(operand))
    {
        auto symbol = _symbolTable.get(var->getName());
        return convertType(symbol.type);
    }
    else
        throw std::runtime_error("Internal error: Invalid operand to get asm type");
}

std::vector<std::shared_ptr<Assembly::Instruction>>
CodeGen::passParams(const std::vector<std::string> &params)
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
        auto asmType = getAsmType(std::make_shared<TACKY::Var>(regParams[i]));
        insts.push_back(std::make_shared<Assembly::Mov>(asmType, std::make_shared<Assembly::Reg>(r), asmParam));
    }

    // pass params on the stack
    for (int i = 0; i < stackParams.size(); i++)
    {
        auto stack = std::make_shared<Assembly::Stack>(16 + 8 * i);
        auto asmParam = std::make_shared<Assembly::Pseudo>(stackParams[i]);
        auto asmType = getAsmType(std::make_shared<TACKY::Var>(stackParams[i]));
        insts.push_back(std::make_shared<Assembly::Mov>(asmType, stack, asmParam));
    }

    return insts;
}

std::shared_ptr<Assembly::Operand>
CodeGen::convertVal(const std::shared_ptr<TACKY::Val> &val)
{
    switch (val->getType())
    {
    case TACKY::NodeType::Constant:
    {
        if (auto constInt = Constants::getConstInt(*std::dynamic_pointer_cast<TACKY::Constant>(val)->getConst()))
            return std::make_shared<Assembly::Imm>(constInt->val);
        else if (auto constLong = Constants::getConstLong(*std::dynamic_pointer_cast<TACKY::Constant>(val)->getConst()))
            return std::make_shared<Assembly::Imm>(constLong->val);
        else
            throw std::runtime_error("Internal error: Invalid constant to convert to assembly");
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

Assembly::UnaryOp
CodeGen::convertUnop(const TACKY::UnaryOp op)
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

Assembly::BinaryOp
CodeGen::convertBinop(const TACKY::BinaryOp op)
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

Assembly::CondCode
CodeGen::convertCondCode(const TACKY::BinaryOp op)
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

std::vector<std::shared_ptr<Assembly::Instruction>>
CodeGen::convertFunCall(const std::shared_ptr<TACKY::FunCall> &fnCall)
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
        insts.push_back(
            std::make_shared<Assembly::Binary>(
                Assembly::BinaryOp::Sub,
                std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                std::make_shared<Assembly::Imm>(stackPadding),
                std::make_shared<Assembly::Reg>(Assembly::RegName::SP)));

    // pass arguments in registers
    for (int i{0}; i < regArgs.size(); i++)
    {
        auto r = PARAM_PASSING_REGS[i];
        auto asmArg = convertVal(regArgs[i]);
        insts.push_back(std::make_shared<Assembly::Mov>(getAsmType(regArgs[i]), asmArg, std::make_shared<Assembly::Reg>(r)));
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
            auto asmType = getAsmType(tackyArg);
            if (Assembly::isAsmQuadword(*asmType))
            {
                insts.push_back(std::make_shared<Assembly::Push>(asmArg));
            }
            else
            {
                // Copy into a register before pushing
                insts.push_back(std::make_shared<Assembly::Mov>(asmType, asmArg, std::make_shared<Assembly::Reg>(Assembly::RegName::AX)));
                insts.push_back(std::make_shared<Assembly::Push>(std::make_shared<Assembly::Reg>(Assembly::RegName::AX)));
            }
        }
    }

    // emit call function
    insts.push_back(std::make_shared<Assembly::Call>(fnCall->getFnName()));

    // adjust stack pointer
    auto bytesToRemove = 8 * (stackArgs.size()) + stackPadding;
    if (bytesToRemove != 0)
        insts.push_back(
            std::make_shared<Assembly::Binary>(
                Assembly::BinaryOp::Add,
                std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                std::make_shared<Assembly::Imm>(bytesToRemove),
                std::make_shared<Assembly::Reg>(Assembly::RegName::SP)));

    // retrieve return value
    auto asmDst = convertVal(fnCall->getDst());
    insts.push_back(std::make_shared<Assembly::Mov>(
        getAsmType(fnCall->getDst()),
        std::make_shared<Assembly::Reg>(Assembly::RegName::AX),
        asmDst));

    return insts;
}

std::vector<std::shared_ptr<Assembly::Instruction>>
CodeGen::convertInstruction(const std::shared_ptr<TACKY::Instruction> &inst)
{
    switch (inst->getType())
    {
    case TACKY::NodeType::Copy:
    {
        auto copyInst = std::dynamic_pointer_cast<TACKY::Copy>(inst);

        auto asmType = getAsmType(copyInst->getSrc());
        auto asmSrc = convertVal(copyInst->getSrc());
        auto asmDst = convertVal(copyInst->getDst());

        return {
            std::make_shared<Assembly::Mov>(asmType, asmSrc, asmDst),
        };
    }
    case TACKY::NodeType::Return:
    {
        auto returnInst = std::dynamic_pointer_cast<TACKY::Return>(inst);

        auto asmType = getAsmType(returnInst->getValue());
        auto asmVal = convertVal(returnInst->getValue());

        return {
            std::make_shared<Assembly::Mov>(asmType, asmVal, std::make_shared<Assembly::Reg>(Assembly::RegName::AX)),
            std::make_shared<Assembly::Ret>(),
        };
    }
    case TACKY::NodeType::Unary:
    {
        auto unaryInst = std::dynamic_pointer_cast<TACKY::Unary>(inst);

        if (unaryInst->getOp() == TACKY::UnaryOp::Not)
        {
            auto srcType = getAsmType(unaryInst->getSrc());
            auto dstType = getAsmType(unaryInst->getDst());
            auto asmSrc = convertVal(unaryInst->getSrc());
            auto asmDst = convertVal(unaryInst->getDst());

            return {
                std::make_shared<Assembly::Cmp>(srcType, std::make_shared<Assembly::Imm>(0), asmSrc),
                std::make_shared<Assembly::Mov>(dstType, std::make_shared<Assembly::Imm>(0), asmDst),
                std::make_shared<Assembly::SetCC>(Assembly::CondCode::E, asmDst),
            };
        }
        else
        {
            auto asmType = getAsmType(unaryInst->getDst());
            auto asmOp = convertUnop(unaryInst->getOp());
            auto asmSrc = convertVal(unaryInst->getSrc());
            auto asmDst = convertVal(unaryInst->getDst());

            return {
                std::make_shared<Assembly::Mov>(asmType, asmSrc, asmDst),
                std::make_shared<Assembly::Unary>(asmOp, asmType, asmDst),
            };
        }
    }
    case TACKY::NodeType::Binary:
    {
        auto binaryInst = std::dynamic_pointer_cast<TACKY::Binary>(inst);

        auto srcType = getAsmType(binaryInst->getSrc1());
        auto dstType = getAsmType(binaryInst->getDst());
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
                std::make_shared<Assembly::Cmp>(srcType, asmSrc1, asmSrc2),
                std::make_shared<Assembly::Mov>(dstType, zero(), asmDst),
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
                std::make_shared<Assembly::Mov>(srcType, asmSrc1, std::make_shared<Assembly::Reg>(Assembly::RegName::AX)),
                std::make_shared<Assembly::Cdq>(srcType),
                std::make_shared<Assembly::Idiv>(srcType, asmSrc2),
                std::make_shared<Assembly::Mov>(srcType, std::make_shared<Assembly::Reg>(resultRegName), asmDst),
            };
        }

        // For Bit Shift instructions, source 2 can only be either in CX register or an Imm
        case TACKY::BinaryOp::BitShiftLeft:
        case TACKY::BinaryOp::BitShiftRight:
        {
            auto asmOp = convertBinop(binaryInst->getOp());
            auto asmType = getAsmType(binaryInst->getSrc1());

            if (asmSrc2->getType() == Assembly::NodeType::Imm)
            {
                return {
                    std::make_shared<Assembly::Mov>(asmType, asmSrc1, asmDst),
                    std::make_shared<Assembly::Binary>(asmOp, asmType, asmSrc2, asmDst),
                };
            }
            else
            {
                auto RegCX = std::make_shared<Assembly::Reg>(Assembly::RegName::CX);

                return {
                    std::make_shared<Assembly::Mov>(asmType, asmSrc1, asmDst),
                    std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Longword()), asmSrc2, RegCX),
                    std::make_shared<Assembly::Binary>(asmOp, asmType, RegCX, asmDst),
                };
            }
        }

        // Addition/Subtraction/Multiplication
        default:
        {
            auto asmOp = convertBinop(binaryInst->getOp());

            return {
                std::make_shared<Assembly::Mov>(srcType, asmSrc1, asmDst),
                std::make_shared<Assembly::Binary>(asmOp, srcType, asmSrc2, asmDst),
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

        auto asmType = getAsmType(jumpIfZeroInst->getCond());
        auto asmCond = convertVal(jumpIfZeroInst->getCond());

        return {
            std::make_shared<Assembly::Cmp>(asmType, std::make_shared<Assembly::Imm>(0), asmCond),
            std::make_shared<Assembly::JmpCC>(Assembly::CondCode::E, jumpIfZeroInst->getTarget()),
        };
    }
    case TACKY::NodeType::JumpIfNotZero:
    {
        auto jumpIfNotZeroInst = std::dynamic_pointer_cast<TACKY::JumpIfNotZero>(inst);

        auto asmType = getAsmType(jumpIfNotZeroInst->getCond());
        auto asmCond = convertVal(jumpIfNotZeroInst->getCond());

        return {
            std::make_shared<Assembly::Cmp>(asmType, std::make_shared<Assembly::Imm>(0), asmCond),
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
    case TACKY::NodeType::SignExtend:
    {
        auto signExtend = std::dynamic_pointer_cast<TACKY::SignExtend>(inst);

        auto asmSrc = convertVal(signExtend->getSrc());
        auto asmDst = convertVal(signExtend->getDst());

        return {
            std::make_shared<Assembly::Movsx>(asmSrc, asmDst),
        };
    }
    case TACKY::NodeType::Truncate:
    {
        auto truncate = std::dynamic_pointer_cast<TACKY::Truncate>(inst);

        auto asmSrc = convertVal(truncate->getSrc());
        auto asmDst = convertVal(truncate->getDst());

        return {
            std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Longword()), asmSrc, asmDst),
        };
    }
    default:
        throw std::runtime_error("Internal Error: Invalid TACKY instruction");
    }
}

std::shared_ptr<Assembly::TopLevel>
CodeGen::convertTopLevel(const std::shared_ptr<TACKY::TopLevel> &topLevel)
{
    if (auto fn = std::dynamic_pointer_cast<TACKY::Function>(topLevel))
    {
        std::vector<std::shared_ptr<Assembly::Instruction>> insts{};

        auto paramInsts = passParams(fn->getParams());
        insts.insert(insts.end(), paramInsts.begin(), paramInsts.end());

        for (auto &inst : fn->getInstructions())
        {
            auto asmInsts = convertInstruction(inst);
            insts.insert(insts.end(), asmInsts.begin(), asmInsts.end());
        }

        return std::make_shared<Assembly::Function>(fn->getName(), fn->isGlobal(), insts);
    }
    else if (auto staticVar = std::dynamic_pointer_cast<TACKY::StaticVariable>(topLevel))
    {
        return std::make_shared<Assembly::StaticVariable>(
            staticVar->getName(),
            staticVar->isGlobal(),
            Types::getAlignment(staticVar->getDataType()),
            staticVar->getInit());
    }
    else
    {
        throw std::runtime_error("Internal Error: Invalid TACKY top level");
    }
}

void CodeGen::convertSymbol(const std::string &name, const Symbols::Symbol &symbol)
{
    if (auto funAttr = Symbols::getFunAttr(symbol.attrs))
        _asmSymbolTable.addFun(name, funAttr->defined);
    else if (auto staticAttr = Symbols::getStaticAttr(symbol.attrs))
        _asmSymbolTable.addVar(name, convertType(symbol.type), true);
    else
        _asmSymbolTable.addVar(name, convertType(symbol.type), false);
}

std::shared_ptr<Assembly::Program>
CodeGen::gen(std::shared_ptr<TACKY::Program> prog)
{
    std::vector<std::shared_ptr<Assembly::TopLevel>> convertedTl{};

    for (const auto &tl : prog->getTopLevels())
    {
        auto asmTl = convertTopLevel(tl);
        convertedTl.push_back(asmTl);
    }

    for (const auto &[name, symbol] : _symbolTable.getAllSymbols())
    {
        convertSymbol(name, symbol);
    }

    return std::make_shared<Assembly::Program>(convertedTl);
}
