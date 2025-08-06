#include "ConstantFolding.h"
#include "../Const.h"
#include "../ConstConvert.h"
#include "../Types.h"
#include "../utils/TackyPrettyPrint.h"
#include <memory>
#include <optional>
#include <iostream>

// Helper: Evaluate a cast of a constant to the type of dst
static std::shared_ptr<TACKY::Instruction> evaluateCast(
    const Constants::Const &srcConst,
    const std::shared_ptr<TACKY::Val> &dst,
    const Symbols::SymbolTable &symbolTable)
{
    auto dstType = TACKY::typeOfVal(dst, symbolTable);
    Constants::Const converted = Constants::makeIntZero();
    try
    {
        converted = *ConstConvert::convert(dstType, std::make_shared<Constants::Const>(srcConst));
    }
    catch (...)
    {
        converted = *ConstConvert::convert(dstType, std::make_shared<Constants::Const>(Constants::makeIntZero()));
    }
    return std::make_shared<TACKY::Copy>(
        std::make_shared<TACKY::Constant>(std::make_shared<Constants::Const>(converted)), dst);
}

// Helper: int_of_bool
static Constants::Const intOfBool(bool b)
{
    return b ? Constants::makeConstInt(1) : Constants::makeIntZero();
}

// Constant folding for unary operations
static Constants::Const evaluateUnop(TACKY::UnaryOp op, const Constants::Const &c)
{
    using namespace Constants;
    switch (op)
    {
    case TACKY::UnaryOp::Not:
        return intOfBool(Constants::isZero(c));
    case TACKY::UnaryOp::Complement:
        // Bitwise complement for integer types
        return std::visit([](auto &&val) -> Const
                          {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, ConstInt>)
                return ConstInt{~val.val};
            if constexpr (std::is_same_v<T, ConstLong>)
                return ConstLong{~val.val};
            if constexpr (std::is_same_v<T, ConstUInt>)
                return ConstUInt{~val.val};
            if constexpr (std::is_same_v<T, ConstULong>)
                return ConstULong{~val.val};
            if constexpr (std::is_same_v<T, ConstChar>)
                return ConstChar{static_cast<int8_t>(~val.val)};
            if constexpr (std::is_same_v<T, ConstUChar>)
                return ConstUChar{static_cast<uint8_t>(~val.val)};
            throw std::runtime_error("complement: unsupported type"); }, c);
    case TACKY::UnaryOp::Negate:
        // Arithmetic negation for numeric types
        return std::visit([](auto &&val) -> Const
                          {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, ConstInt>)
                return ConstInt{-val.val};
            if constexpr (std::is_same_v<T, ConstLong>)
                return ConstLong{-val.val};
            if constexpr (std::is_same_v<T, ConstUInt>)
                return ConstUInt{static_cast<uint32_t>(-static_cast<int32_t>(val.val))};
            if constexpr (std::is_same_v<T, ConstULong>)
                return ConstULong{static_cast<uint64_t>(-static_cast<int64_t>(val.val))};
            if constexpr (std::is_same_v<T, ConstDouble>)
                return ConstDouble{-val.val};
            if constexpr (std::is_same_v<T, ConstChar>)
                return ConstChar{static_cast<int8_t>(-val.val)};
            if constexpr (std::is_same_v<T, ConstUChar>)
                return ConstUChar{static_cast<uint8_t>(-static_cast<int8_t>(val.val))};
            throw std::runtime_error("negate: unsupported type"); }, c);
    }
    throw std::runtime_error("Unknown unary op");
}

// Constant folding for binary operations
static Constants::Const evaluateBinop(TACKY::BinaryOp op, const Constants::Const &c1, const Constants::Const &c2)
{
    using namespace Constants;
    switch (op)
    {
    case TACKY::BinaryOp::Add:
        return add(c1, c2);
    case TACKY::BinaryOp::Subtract:
        return sub(c1, c2);
    case TACKY::BinaryOp::Multiply:
        return mul(c1, c2);
    case TACKY::BinaryOp::Divide:
        try
        {
            return div(c1, c2);
        }
        catch (...)
        {
            return zeroLike(c1);
        }
    case TACKY::BinaryOp::Remainder:
        try
        {
            return modulo(c1, c2);
        }
        catch (...)
        {
            return zeroLike(c1);
        }
    case TACKY::BinaryOp::Equal:
        return intOfBool(eq(c1, c2));
    case TACKY::BinaryOp::NotEqual:
        return intOfBool(!eq(c1, c2));
    case TACKY::BinaryOp::GreaterThan:
        return intOfBool(gt(c1, c2));
    case TACKY::BinaryOp::GreaterOrEqual:
        return intOfBool(ge(c1, c2));
    case TACKY::BinaryOp::LessThan:
        return intOfBool(lt(c1, c2));
    case TACKY::BinaryOp::LessOrEqual:
        return intOfBool(le(c1, c2));
    case TACKY::BinaryOp::BitwiseAnd:
        return bitwiseAnd(c1, c2);
    case TACKY::BinaryOp::BitwiseOr:
        return bitwiseOr(c1, c2);
    case TACKY::BinaryOp::BitwiseXor:
        return bitwiseXor(c1, c2);
    case TACKY::BinaryOp::BitShiftLeft:
        return bitshiftLeft(c1, c2);
    case TACKY::BinaryOp::BitShiftRight:
        return bitshiftRight(c1, c2);
    }
    throw std::runtime_error("Unknown binary op");
}

// Try to constant fold a single instruction
static std::optional<std::shared_ptr<TACKY::Instruction>>
optimizeInstruction(const std::shared_ptr<TACKY::Instruction> &instr, const Symbols::SymbolTable &symbolTable)
{
    using namespace TACKY;
    switch (instr->getType())
    {
    case NodeType::Unary:
    {
        auto u = std::static_pointer_cast<Unary>(instr);
        if (u->getSrc()->getType() == NodeType::Constant)
        {
            auto c = *std::static_pointer_cast<Constant>(u->getSrc())->getConst();
            auto new_src = evaluateUnop(u->getOp(), c);
            return std::make_shared<Copy>(
                std::make_shared<Constant>(std::make_shared<Constants::Const>(new_src)),
                u->getDst());
        }
        break;
    }
    case NodeType::Binary:
    {
        auto b = std::static_pointer_cast<Binary>(instr);
        if (b->getSrc1()->getType() == NodeType::Constant && b->getSrc2()->getType() == NodeType::Constant)
        {
            auto c1 = *std::static_pointer_cast<Constant>(b->getSrc1())->getConst();
            auto c2 = *std::static_pointer_cast<Constant>(b->getSrc2())->getConst();
            auto new_src = evaluateBinop(b->getOp(), c1, c2);
            return std::make_shared<Copy>(
                std::make_shared<Constant>(std::make_shared<Constants::Const>(new_src)),
                b->getDst());
        }
        break;
    }
    case NodeType::JumpIfZero:
    {
        auto jz = std::static_pointer_cast<JumpIfZero>(instr);
        if (jz->getCond()->getType() == NodeType::Constant)
        {
            auto c = *std::static_pointer_cast<const Constant>(jz->getCond())->getConst();
            if (Constants::isZero(c))
                return std::make_shared<Jump>(jz->getTarget());
            else
                return nullptr;
        }
        break;
    }
    case NodeType::JumpIfNotZero:
    {
        auto jnz = std::static_pointer_cast<JumpIfNotZero>(instr);
        if (jnz->getCond()->getType() == NodeType::Constant)
        {
            auto c = *std::static_pointer_cast<Constant>(jnz->getCond())->getConst();
            if (Constants::isZero(c))
                return nullptr;
            else
                return std::make_shared<Jump>(jnz->getTarget());
        }
        break;
    }
    // Type conversions and copies
    case NodeType::Truncate:
    {
        auto conv = std::static_pointer_cast<Truncate>(instr);
        if (conv->getSrc()->getType() == NodeType::Constant)
        {
            auto c = *std::static_pointer_cast<Constant>(conv->getSrc())->getConst();
            return evaluateCast(c, conv->getDst(), symbolTable);
        }
        break;
    }
    case NodeType::SignExtend:
    {
        auto conv = std::static_pointer_cast<SignExtend>(instr);
        if (conv->getSrc()->getType() == NodeType::Constant)
        {
            auto c = *std::static_pointer_cast<Constant>(conv->getSrc())->getConst();
            return evaluateCast(c, conv->getDst(), symbolTable);
        }
        break;
    }
    case NodeType::ZeroExtend:
    {
        auto conv = std::static_pointer_cast<ZeroExtend>(instr);
        if (conv->getSrc()->getType() == NodeType::Constant)
        {
            auto c = *std::static_pointer_cast<Constant>(conv->getSrc())->getConst();
            return evaluateCast(c, conv->getDst(), symbolTable);
        }
        break;
    }
    case NodeType::DoubleToInt:
    {
        auto conv = std::static_pointer_cast<DoubleToInt>(instr);
        if (conv->getSrc()->getType() == NodeType::Constant)
        {
            auto c = *std::static_pointer_cast<Constant>(conv->getSrc())->getConst();
            return evaluateCast(c, conv->getDst(), symbolTable);
        }
        break;
    }
    case NodeType::DoubleToUInt:
    {
        auto conv = std::static_pointer_cast<DoubleToUInt>(instr);
        if (conv->getSrc()->getType() == NodeType::Constant)
        {
            auto c = *std::static_pointer_cast<Constant>(conv->getSrc())->getConst();
            return evaluateCast(c, conv->getDst(), symbolTable);
        }
        break;
    }
    case NodeType::IntToDouble:
    {
        auto conv = std::static_pointer_cast<IntToDouble>(instr);
        if (conv->getSrc()->getType() == NodeType::Constant)
        {
            auto c = *std::static_pointer_cast<Constant>(conv->getSrc())->getConst();
            return evaluateCast(c, conv->getDst(), symbolTable);
        }
        break;
    }
    case NodeType::UIntToDouble:
    {
        auto conv = std::static_pointer_cast<UIntToDouble>(instr);
        if (conv->getSrc()->getType() == NodeType::Constant)
        {
            auto c = *std::static_pointer_cast<Constant>(conv->getSrc())->getConst();
            return evaluateCast(c, conv->getDst(), symbolTable);
        }
        break;
    }
    case NodeType::Copy:
    {
        auto copy = std::static_pointer_cast<Copy>(instr);
        if (copy->getSrc()->getType() == NodeType::Constant)
        {
            auto c = *std::static_pointer_cast<Constant>(copy->getSrc())->getConst();
            return evaluateCast(c, copy->getDst(), symbolTable);
        }
        break;
    }
    default:
        break;
    }
    // If not constant foldable, return the original instruction
    return instr;
}

// Helper: Print the function body in a recognizable format
static void debugPrintFunction(const std::string &debugLabel, const std::vector<std::shared_ptr<TACKY::Instruction>> &instructions)
{
    auto func = std::make_shared<TACKY::Function>(debugLabel, false, std::vector<std::string>{"UNKNOWN"}, instructions);
    auto program = std::make_shared<TACKY::Program>(
        std::vector<std::shared_ptr<TACKY::TopLevel>>{func});
    std::cout << "=== " << debugLabel << " (constant folding) ===\n";
    TackyPrettyPrint printer;
    printer.visit(*program, false);
    std::cout << "\n";
}

std::vector<std::shared_ptr<TACKY::Instruction>>
constantFold(const std::string &debugLabel,
             const std::vector<std::shared_ptr<TACKY::Instruction>> &instructions,
             const Symbols::SymbolTable &symbolTable,
             bool debug)
{
    if (debug)
    {
        std::cout << "[ConstantFolding] Before:\n";
        debugPrintFunction(debugLabel, instructions);
    }

    std::vector<std::shared_ptr<TACKY::Instruction>> result;
    for (const auto &instr : instructions)
    {
        auto opt = optimizeInstruction(instr, symbolTable);
        if (opt && *opt)
            result.push_back(*opt);
    }

    if (debug)
    {
        std::cout << "[ConstantFolding] After:\n";
        debugPrintFunction(debugLabel, result);
    }

    return result;
}