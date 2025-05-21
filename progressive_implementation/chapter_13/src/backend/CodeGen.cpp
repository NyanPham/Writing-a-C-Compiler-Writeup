#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <tuple>

#include "CodeGen.h"
#include "TACKY.h"
#include "Const.h"
#include "Assembly.h"
#include "AssemblySymbols.h"
#include "UniqueIds.h"

std::shared_ptr<Assembly::Imm> zero()
{
    return std::make_shared<Assembly::Imm>(0);
}

std::tuple<
    std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>>,
    std::vector<std::shared_ptr<Assembly::Operand>>,
    std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>>>
CodeGen::classifyParameters(const std::vector<std::shared_ptr<TACKY::Val>> &tackyVals)
{
    std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>> intRegArgs;
    std::vector<std::shared_ptr<Assembly::Operand>> dblRegArgs;
    std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>> stackArgs;

    for (const auto &v : tackyVals)
    {
        auto operand = convertVal(v);
        auto asmType = getAsmType(v);
        auto typedOperand = std::make_pair(asmType, operand);

        if (Assembly::isAsmDouble(*asmType))
        {
            if (dblRegArgs.size() < DBL_PARAM_PASSING_REGS.size())
            {
                dblRegArgs.push_back(operand);
            }
            else
            {
                stackArgs.push_back(typedOperand);
            }
        }
        else
        {
            if (intRegArgs.size() < INT_PARAM_PASSING_REGS.size())
            {
                intRegArgs.push_back(typedOperand);
            }
            else
            {
                stackArgs.push_back(typedOperand);
            }
        }
    }

    return std::make_tuple(intRegArgs, dblRegArgs, stackArgs);
}

std::string CodeGen::addConstant(double dbl, size_t alignment)
{
    // We if we've defined this double already
    if (_constants.find(dbl) != _constants.end())
    {
        const auto &[name, oldAlignment] = _constants.find(dbl)->second;
        // Update alignment to max of current and new
        _constants.at(dbl).second = std::max(alignment, oldAlignment);
        return name;
    }

    // We haven't defined it yet, add it to the table
    auto name = UniqueIds::makeLabel("dbl");
    _constants.emplace(dbl, std::make_pair(name, alignment));
    return name;
}

std::shared_ptr<Types::DataType>
CodeGen::tackyType(const std::shared_ptr<TACKY::Val> &operand)
{
    if (auto constant = std::dynamic_pointer_cast<TACKY::Constant>(operand))
    {
        return std::make_shared<Types::DataType>(Constants::typeOfConst(*constant->getConst()));
    }
    else if (auto var = std::dynamic_pointer_cast<TACKY::Var>(operand))
    {
        auto entry = _symbolTable.get(var->getName());
        return std::make_shared<Types::DataType>(entry.type);
    }
    else
        throw std::runtime_error("Internal error: invalid operand to get tacky type");
}

std::shared_ptr<Assembly::AsmType>
CodeGen::convertType(const Types::DataType &type)
{
    if (Types::isIntType(type) || Types::isUIntType(type))
        return std::make_shared<Assembly::AsmType>(Assembly::Longword());
    else if (Types::isLongType(type) || Types::isULongType(type))
        return std::make_shared<Assembly::AsmType>(Assembly::Quadword());
    else if (Types::isDoubleType(type))
        return std::make_shared<Assembly::AsmType>(Assembly::Double());
    else
        throw std::runtime_error("Internal error: converting function type to assembly");
}

std::shared_ptr<Assembly::AsmType>
CodeGen::getAsmType(const std::shared_ptr<TACKY::Val> &operand)
{
    return convertType(*tackyType(operand));
}

std::vector<std::shared_ptr<Assembly::Instruction>>
CodeGen::passParams(const std::vector<std::shared_ptr<TACKY::Val>> &params)
{
    auto [intRegParams, dblRegParams, stackParams] = classifyParameters(params);
    std::vector<std::shared_ptr<Assembly::Instruction>> insts{};

    // pass params in INTEGER regsiters
    for (int i = 0; i < intRegParams.size(); i++)
    {
        auto r = INT_PARAM_PASSING_REGS[i];
        auto [paramType, param] = intRegParams[i];
        insts.push_back(std::make_shared<Assembly::Mov>(paramType, std::make_shared<Assembly::Reg>(r), param));
    }

    // pass params in DOUBLE regsiters
    for (int i = 0; i < dblRegParams.size(); i++)
    {
        auto r = DBL_PARAM_PASSING_REGS[i];
        auto param = dblRegParams[i];
        insts.push_back(std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Double()), std::make_shared<Assembly::Reg>(r), param));
    }

    // pass params on the stack
    // first param passed on stack has index 0 and is passed at Stack(16)
    for (int i = 0; i < stackParams.size(); i++)
    {
        auto stack = std::make_shared<Assembly::Stack>(16 + 8 * i);
        auto [paramType, param] = stackParams[i];
        insts.push_back(std::make_shared<Assembly::Mov>(paramType, stack, param));
    }

    return insts;
}

std::shared_ptr<Assembly::Operand>
CodeGen::convertVal(const std::shared_ptr<TACKY::Val> &val)
{
    if (auto constant = std::dynamic_pointer_cast<TACKY::Constant>(val))
    {
        if (auto constInt = Constants::getConstInt(*constant->getConst()))
            return std::make_shared<Assembly::Imm>(constInt->val);
        else if (auto constLong = Constants::getConstLong(*constant->getConst()))
            return std::make_shared<Assembly::Imm>(constLong->val);
        else if (auto constUInt = Constants::getConstUInt(*constant->getConst()))
            return std::make_shared<Assembly::Imm>(constUInt->val);
        else if (auto constULong = Constants::getConstULong(*constant->getConst()))
            return std::make_shared<Assembly::Imm>(constULong->val);
        else if (auto constDouble = Constants::getConstDouble(*constant->getConst()))
            return std::make_shared<Assembly::Data>(addConstant(constDouble->val, 8));
        else
            throw std::runtime_error("Internal error: Invalid constant to convert to assembly");
    }
    else if (auto var = std::dynamic_pointer_cast<TACKY::Var>(val))
    {
        return std::make_shared<Assembly::Pseudo>(var->getName());
    }
    else
        throw std::runtime_error("Internal error: Invalid value to convert to assembly");
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
    case TACKY::BinaryOp::Divide:
        return Assembly::BinaryOp::DivDouble; // NB should only be called for operands on double
    case TACKY::BinaryOp::BitwiseAnd:
        return Assembly::BinaryOp::And;
    case TACKY::BinaryOp::BitwiseOr:
        return Assembly::BinaryOp::Or;
    case TACKY::BinaryOp::BitwiseXor:
        return Assembly::BinaryOp::Xor;
    case TACKY::BinaryOp::Remainder:
    case TACKY::BinaryOp::Equal:
    case TACKY::BinaryOp::NotEqual:
    case TACKY::BinaryOp::LessThan:
    case TACKY::BinaryOp::LessOrEqual:
    case TACKY::BinaryOp::GreaterThan:
    case TACKY::BinaryOp::GreaterOrEqual:
    case TACKY::BinaryOp::BitShiftLeft:
    case TACKY::BinaryOp::BitShiftRight:
        throw std::runtime_error("Internal Error: Shouldn't handle like other binary operators!");
    default:
        throw std::runtime_error("Internal Error: Unknown Binary Operators!");
    }
}

Assembly::BinaryOp
CodeGen::convertShiftOp(const TACKY::BinaryOp op, bool isSigned)
{
    /*
        NOTE: Sal/Shl are actually the same operations;
        we use different mnemonics for symmetry with Sar/Shr, which are distinct.
    */
    if (op == TACKY::BinaryOp::BitShiftLeft)
    {
        if (isSigned)
            return Assembly::BinaryOp::Sal;
        else
            return Assembly::BinaryOp::Shl;
    }

    if (op == TACKY::BinaryOp::BitShiftRight)
    {
        if (isSigned)
            return Assembly::BinaryOp::Sar;
        else
            return Assembly::BinaryOp::Shr;
    }

    throw std::runtime_error("Internal error: Not a bitwise shift operation");
}

Assembly::CondCode
CodeGen::convertCondCode(const TACKY::BinaryOp op, bool isSigned)
{
    switch (op)
    {
    case TACKY::BinaryOp::Equal:
        return Assembly::CondCode::E;
    case TACKY::BinaryOp::NotEqual:
        return Assembly::CondCode::NE;
    case TACKY::BinaryOp::LessThan:
        return isSigned ? Assembly::CondCode::L : Assembly::CondCode::B;
    case TACKY::BinaryOp::LessOrEqual:
        return isSigned ? Assembly::CondCode::LE : Assembly::CondCode::BE;
    case TACKY::BinaryOp::GreaterThan:
        return isSigned ? Assembly::CondCode::G : Assembly::CondCode::A;
    case TACKY::BinaryOp::GreaterOrEqual:
        return isSigned ? Assembly::CondCode::GE : Assembly::CondCode::AE;
    default:
        throw std::runtime_error("Internal Error: Unknown binary to cond_code!");
    }
}

std::vector<std::shared_ptr<Assembly::Instruction>>
CodeGen::convertFunCall(const std::shared_ptr<TACKY::FunCall> &fnCall)
{
    auto [intRegArgs, dblRegArgs, stackArgs] = classifyParameters(fnCall->getArgs());
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

    // pass arguments in INTEGER registers
    for (int i{0}; i < intRegArgs.size(); i++)
    {
        auto r = INT_PARAM_PASSING_REGS[i];
        auto [asmType, asmArg] = intRegArgs[i];
        insts.push_back(std::make_shared<Assembly::Mov>(asmType, asmArg, std::make_shared<Assembly::Reg>(r)));
    }

    // pass arguments in DOUBLE registers
    for (int i{0}; i < dblRegArgs.size(); i++)
    {
        auto r = DBL_PARAM_PASSING_REGS[i];
        insts.push_back(std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Double()), dblRegArgs[i], std::make_shared<Assembly::Reg>(r)));
    }

    // pass arguments on the stack
    std::reverse(stackArgs.begin(), stackArgs.end());
    for (const auto &[asmType, asmArg] : stackArgs)
    {
        if (asmArg->getType() == Assembly::NodeType::Reg || asmArg->getType() == Assembly::NodeType::Imm)
        {
            insts.push_back(std::make_shared<Assembly::Push>(asmArg));
        }
        else
        {
            if (Assembly::isAsmQuadword(*asmType) || Assembly::isAsmDouble(*asmType))
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
    auto asmDstType = getAsmType(fnCall->getDst());
    auto returnReg = Assembly::isAsmDouble(*asmDstType)
                         ? Assembly::RegName::XMM0
                         : Assembly::RegName::AX;

    insts.push_back(std::make_shared<Assembly::Mov>(
        asmDstType,
        std::make_shared<Assembly::Reg>(returnReg),
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
        auto returnReg = Assembly::isAsmDouble(*asmType) ? Assembly::RegName::XMM0 : Assembly::RegName::AX;

        return {
            std::make_shared<Assembly::Mov>(asmType, asmVal, std::make_shared<Assembly::Reg>(returnReg)),
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

            if (Assembly::isAsmDouble(*srcType))
            {
                return {
                    std::make_shared<Assembly::Binary>(
                        Assembly::BinaryOp::Xor,
                        std::make_shared<Assembly::AsmType>(Assembly::Double()),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0)),
                    std::make_shared<Assembly::Cmp>(srcType, asmSrc, std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0)),
                    std::make_shared<Assembly::Mov>(dstType, zero(), asmDst),
                    std::make_shared<Assembly::SetCC>(Assembly::CondCode::E, asmDst),
                };
            }
            else
            {
                return {
                    std::make_shared<Assembly::Cmp>(srcType, zero(), asmSrc),
                    std::make_shared<Assembly::Mov>(dstType, zero(), asmDst),
                    std::make_shared<Assembly::SetCC>(Assembly::CondCode::E, asmDst),
                };
            }
        }
        else if (unaryInst->getOp() == TACKY::UnaryOp::Negate && Types::isDoubleType(*tackyType(unaryInst->getSrc())))
        {
            auto asmSrc = convertVal(unaryInst->getSrc());
            auto asmDst = convertVal(unaryInst->getDst());
            auto negativeZero = addConstant(-0.0, 16);

            return {
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Double()), asmSrc, asmDst),
                std::make_shared<Assembly::Binary>(
                    Assembly::BinaryOp::Xor,
                    std::make_shared<Assembly::AsmType>(Assembly::Double()),
                    std::make_shared<Assembly::Data>(negativeZero),
                    asmDst),
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
            auto isSigned = Assembly::isAsmDouble(*srcType)
                                ? false
                                : Types::isSigned(*tackyType(binaryInst->getSrc1()));
            auto condCode = convertCondCode(binaryInst->getOp(), isSigned);

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
            if (!Assembly::isAsmDouble(*srcType))
            {
                Assembly::RegName resultRegName =
                    binaryInst->getOp() == TACKY::BinaryOp::Divide
                        ? Assembly::RegName::AX
                        : Assembly::RegName::DX;

                if (Types::isSigned(*tackyType(binaryInst->getSrc1())))
                {
                    return {
                        std::make_shared<Assembly::Mov>(srcType, asmSrc1, std::make_shared<Assembly::Reg>(Assembly::RegName::AX)),
                        std::make_shared<Assembly::Cdq>(srcType),
                        std::make_shared<Assembly::Idiv>(srcType, asmSrc2),
                        std::make_shared<Assembly::Mov>(srcType, std::make_shared<Assembly::Reg>(resultRegName), asmDst),
                    };
                }
                else
                {
                    return {
                        std::make_shared<Assembly::Mov>(srcType, asmSrc1, std::make_shared<Assembly::Reg>(Assembly::RegName::AX)),
                        std::make_shared<Assembly::Mov>(srcType, zero(), std::make_shared<Assembly::Reg>(Assembly::RegName::DX)),
                        std::make_shared<Assembly::Div>(srcType, asmSrc2),
                        std::make_shared<Assembly::Mov>(srcType, std::make_shared<Assembly::Reg>(resultRegName), asmDst),
                    };
                }
            }

            auto asmOp = convertBinop(binaryInst->getOp());

            return {
                std::make_shared<Assembly::Mov>(srcType, asmSrc1, asmDst),
                std::make_shared<Assembly::Binary>(asmOp, srcType, asmSrc2, asmDst),
            };
        }

        // For Bit Shift instructions, source 2 can only be either in CX register or an Imm
        case TACKY::BinaryOp::BitShiftLeft:
        case TACKY::BinaryOp::BitShiftRight:
        {
            auto isSigned = Types::isSigned(*tackyType(binaryInst->getSrc1()));
            auto asmOp = convertShiftOp(binaryInst->getOp(), isSigned);
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

        if (Assembly::isAsmDouble(*asmType))
        {
            return {
                std::make_shared<Assembly::Binary>(
                    Assembly::BinaryOp::Xor,
                    std::make_shared<Assembly::AsmType>(Assembly::Double()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0)),
                std::make_shared<Assembly::Cmp>(asmType, asmCond, std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0)),
                std::make_shared<Assembly::JmpCC>(Assembly::CondCode::E, jumpIfZeroInst->getTarget()),
            };
        }

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

        if (Assembly::isAsmDouble(*asmType))
        {
            return {
                std::make_shared<Assembly::Binary>(
                    Assembly::BinaryOp::Xor,
                    std::make_shared<Assembly::AsmType>(Assembly::Double()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0)),
                std::make_shared<Assembly::Cmp>(asmType, asmCond, std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0)),
                std::make_shared<Assembly::JmpCC>(Assembly::CondCode::NE, jumpIfNotZeroInst->getTarget()),
            };
        }

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
    case TACKY::NodeType::ZeroExtend:
    {
        auto zeroExt = std::dynamic_pointer_cast<TACKY::ZeroExtend>(inst);

        auto asmSrc = convertVal(zeroExt->getSrc());
        auto asmDst = convertVal(zeroExt->getDst());

        return {
            std::make_shared<Assembly::MovZeroExtend>(asmSrc, asmDst),
        };
    }
    case TACKY::NodeType::IntToDouble:
    {
        auto int2Dbl = std::dynamic_pointer_cast<TACKY::IntToDouble>(inst);

        auto asmSrc = convertVal(int2Dbl->getSrc());
        auto asmDst = convertVal(int2Dbl->getDst());
        auto t = getAsmType(int2Dbl->getSrc());

        return {
            std::make_shared<Assembly::Cvtsi2sd>(t, asmSrc, asmDst),
        };
    }
    case TACKY::NodeType::DoubleToInt:
    {
        auto cvt = std::dynamic_pointer_cast<TACKY::DoubleToInt>(inst);

        auto asmSrc = convertVal(cvt->getSrc());
        auto asmDst = convertVal(cvt->getDst());
        auto t = getAsmType(cvt->getDst());

        return {
            std::make_shared<Assembly::Cvttsd2si>(t, asmSrc, asmDst),
        };
    }
    case TACKY::NodeType::UIntToDouble:
    {
        auto cvt = std::dynamic_pointer_cast<TACKY::UIntToDouble>(inst);

        auto asmSrc = convertVal(cvt->getSrc());
        auto asmDst = convertVal(cvt->getDst());

        if (Types::isUIntType(*tackyType(cvt->getSrc())))
        {
            return {
                std::make_shared<Assembly::MovZeroExtend>(asmSrc, std::make_shared<Assembly::Reg>(Assembly::RegName::R9)),
                std::make_shared<Assembly::Cvtsi2sd>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), std::make_shared<Assembly::Reg>(Assembly::RegName::R9), asmDst),
            };
        }
        else
        {
            auto outOfBound = UniqueIds::makeLabel("ulong2dbl.oob");
            auto endLbl = UniqueIds::makeLabel("ulong2dbl.end");
            auto r1 = std::make_shared<Assembly::Reg>(Assembly::RegName::R8);
            auto r2 = std::make_shared<Assembly::Reg>(Assembly::RegName::R9);

            return {
                std::make_shared<Assembly::Cmp>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), zero(), asmSrc),
                std::make_shared<Assembly::JmpCC>(Assembly::CondCode::L, outOfBound),
                std::make_shared<Assembly::Cvtsi2sd>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), asmSrc, asmDst),
                std::make_shared<Assembly::Jmp>(endLbl),
                std::make_shared<Assembly::Label>(outOfBound),
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), asmSrc, r1),
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), r1, r2),
                std::make_shared<Assembly::Unary>(Assembly::UnaryOp::ShrOneOp, std::make_shared<Assembly::AsmType>(Assembly::Quadword()), r2),
                std::make_shared<Assembly::Binary>(Assembly::BinaryOp::And, std::make_shared<Assembly::AsmType>(Assembly::Quadword()), std::make_shared<Assembly::Imm>(1), r1),
                std::make_shared<Assembly::Binary>(Assembly::BinaryOp::Or, std::make_shared<Assembly::AsmType>(Assembly::Quadword()), r1, r2),
                std::make_shared<Assembly::Cvtsi2sd>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), r2, asmDst),
                std::make_shared<Assembly::Binary>(Assembly::BinaryOp::Add, std::make_shared<Assembly::AsmType>(Assembly::Double()), asmDst, asmDst),
                std::make_shared<Assembly::Label>(endLbl),
            };
        }
    }
    case TACKY::NodeType::DoubleToUInt:
    {
        auto cvt = std::dynamic_pointer_cast<TACKY::DoubleToUInt>(inst);

        auto asmSrc = convertVal(cvt->getSrc());
        auto asmDst = convertVal(cvt->getDst());

        if (Types::isUIntType(*tackyType(cvt->getDst())))
        {
            return {
                std::make_shared<Assembly::Cvttsd2si>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), asmSrc, std::make_shared<Assembly::Reg>(Assembly::RegName::R9)),
                std::make_shared<Assembly::MovZeroExtend>(std::make_shared<Assembly::Reg>(Assembly::RegName::R9), asmDst),
            };
        }
        else
        {
            auto outOfBound = UniqueIds::makeLabel("dbl2ulong.oob");
            auto endLbl = UniqueIds::makeLabel("dbl2ulong.end");
            auto upperBound = addConstant(9223372036854775808.0, 8);
            auto upperBoundAsInt = std::make_shared<Assembly::Imm>(std::numeric_limits<int64_t>::min());
            auto r = std::make_shared<Assembly::Reg>(Assembly::RegName::R9);
            auto x = std::make_shared<Assembly::Reg>(Assembly::RegName::XMM7);

            return {
                std::make_shared<Assembly::Cmp>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), std::make_shared<Assembly::Data>(upperBound), asmSrc),
                std::make_shared<Assembly::JmpCC>(Assembly::CondCode::AE, outOfBound),
                std::make_shared<Assembly::Cvttsd2si>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), asmSrc, asmDst),
                std::make_shared<Assembly::Jmp>(endLbl),
                std::make_shared<Assembly::Label>(outOfBound),
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Double()), asmSrc, x),
                std::make_shared<Assembly::Binary>(Assembly::BinaryOp::Sub, std::make_shared<Assembly::AsmType>(Assembly::Double()), std::make_shared<Assembly::Data>(upperBound), x),
                std::make_shared<Assembly::Cvttsd2si>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), x, asmDst),
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), upperBoundAsInt, r),
                std::make_shared<Assembly::Binary>(Assembly::BinaryOp::Add, std::make_shared<Assembly::AsmType>(Assembly::Quadword()), r, asmDst),
                std::make_shared<Assembly::Label>(endLbl),
            };
        }
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

        std::vector<std::shared_ptr<TACKY::Val>> paramsAsTacky;
        const auto &params = fn->getParams();
        paramsAsTacky.reserve(params.size());
        std::transform(params.begin(), params.end(), std::back_inserter(paramsAsTacky),
                       [](const std::string &name)
                       {
                           return std::make_shared<TACKY::Var>(name);
                       });

        auto paramInsts = passParams(paramsAsTacky);
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

std::shared_ptr<Assembly::StaticConstant>
CodeGen::convertConstant(double key, const std::pair<std::string, size_t> &constant)
{
    _asmSymbolTable.addConstant(constant.first, std::make_shared<Assembly::AsmType>(Assembly::Double()));
    return std::make_shared<Assembly::StaticConstant>(
        constant.first,
        constant.second,
        Initializers::DoubleInit(key));
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
    // Clear the hashtable (necessary if we're compiling multiple sources)
    _constants.clear();
    std::vector<std::shared_ptr<Assembly::TopLevel>> convertedTl{};

    for (const auto &tl : prog->getTopLevels())
    {
        auto asmTl = convertTopLevel(tl);
        convertedTl.push_back(asmTl);
    }

    std::vector<std::shared_ptr<Assembly::StaticConstant>> convertedConstants{};

    for (const auto &[key, constant] : _constants)
    {
        convertedConstants.push_back(convertConstant(key, constant));
    }

    for (const auto &[name, symbol] : _symbolTable.getAllSymbols())
    {
        convertSymbol(name, symbol);
    }

    convertedTl.insert(convertedTl.begin(), convertedConstants.begin(), convertedConstants.end());
    return std::make_shared<Assembly::Program>(convertedTl);
}
