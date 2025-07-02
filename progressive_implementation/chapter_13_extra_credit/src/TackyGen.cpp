#include <vector>
#include <algorithm>
#include <iostream>

#include "TackyGen.h"
#include "Tacky.h"
#include "AST.h"
#include "UniqueIds.h"
#include "ConstConvert.h"

std::string
TackyGen::createTmp(const std::optional<Types::DataType> &type)
{
    if (!type.has_value())
        throw std::runtime_error("Internal error: type is null");

    auto name = UniqueIds::makeTemporary();
    _symbolTable.addAutomaticVar(name, type.value());
    return name;
}

std::shared_ptr<Constants::Const>
TackyGen::mkConst(const std::optional<Types::DataType> &type, int64_t i)
{
    if (!type.has_value())
        throw std::runtime_error("Internal error: type is not defined");

    auto asInt = std::make_shared<Constants::Const>(Constants::ConstInt(static_cast<int32_t>(i)));
    return ConstConvert::convert(*type, asInt);
}

std::shared_ptr<AST::Constant>
TackyGen::mkAstConst(const std::optional<Types::DataType> &type, int64_t i)
{
    return std::make_shared<AST::Constant>(mkConst(type, i));
}

std::shared_ptr<TACKY::Instruction>
TackyGen::getCastInst(const std::shared_ptr<TACKY::Val> &src, const std::shared_ptr<TACKY::Val> &dst, const Types::DataType &srcType, const Types::DataType &dstType)
{
    if (Types::isDoubleType(dstType))
    {
        if (Types::isSigned(srcType))
            return std::make_shared<TACKY::IntToDouble>(src, dst);
        else
            return std::make_shared<TACKY::UIntToDouble>(src, dst);
    }
    else if (Types::isDoubleType(srcType))
    {
        if (Types::isSigned(dstType))
            return std::make_shared<TACKY::DoubleToInt>(src, dst);
        else
            return std::make_shared<TACKY::DoubleToUInt>(src, dst);
    }

    // Cast between int types. Note: assumes src and dst have different types
    if (Types::getSize(dstType) == Types::getSize(srcType))
        return std::make_shared<TACKY::Copy>(src, dst);
    else if (Types::getSize(dstType) < Types::getSize(srcType))
        return std::make_shared<TACKY::Truncate>(src, dst);
    else if (Types::isSigned(srcType))
        return std::make_shared<TACKY::SignExtend>(src, dst);
    else
        return std::make_shared<TACKY::ZeroExtend>(src, dst);
}

std::string TackyGen::breakLabel(const std::string &id)
{
    return "break." + id;
}

std::string TackyGen::continueLabel(const std::string &id)
{
    return "continue." + id;
}

TACKY::UnaryOp TackyGen::convertUnop(AST::UnaryOp op)
{
    switch (op)
    {
    case AST::UnaryOp::Complement:
        return TACKY::UnaryOp::Complement;
    case AST::UnaryOp::Negate:
        return TACKY::UnaryOp::Negate;
    case AST::UnaryOp::Not:
        return TACKY::UnaryOp::Not;
    case AST::UnaryOp::Incr:
    case AST::UnaryOp::Decr:
        throw std::runtime_error("Internal error: Should handle ++/-- operator separately!");
    default:
        throw std::invalid_argument("Internal error: Invalid operator");
    }
}

TACKY::BinaryOp TackyGen::convertBinop(AST::BinaryOp op)
{
    switch (op)
    {
    case AST::BinaryOp::Add:
        return TACKY::BinaryOp::Add;
    case AST::BinaryOp::Subtract:
        return TACKY::BinaryOp::Subtract;
    case AST::BinaryOp::Multiply:
        return TACKY::BinaryOp::Multiply;
    case AST::BinaryOp::Divide:
        return TACKY::BinaryOp::Divide;
    case AST::BinaryOp::Remainder:
        return TACKY::BinaryOp::Remainder;
    case AST::BinaryOp::And:
        return TACKY::BinaryOp::And;
    case AST::BinaryOp::Or:
        return TACKY::BinaryOp::Or;
    case AST::BinaryOp::Equal:
        return TACKY::BinaryOp::Equal;
    case AST::BinaryOp::NotEqual:
        return TACKY::BinaryOp::NotEqual;
    case AST::BinaryOp::LessThan:
        return TACKY::BinaryOp::LessThan;
    case AST::BinaryOp::LessOrEqual:
        return TACKY::BinaryOp::LessOrEqual;
    case AST::BinaryOp::GreaterThan:
        return TACKY::BinaryOp::GreaterThan;
    case AST::BinaryOp::GreaterOrEqual:
        return TACKY::BinaryOp::GreaterOrEqual;
    case AST::BinaryOp::BitwiseAnd:
        return TACKY::BinaryOp::BitwiseAnd;
    case AST::BinaryOp::BitwiseOr:
        return TACKY::BinaryOp::BitwiseOr;
    case AST::BinaryOp::BitwiseXor:
        return TACKY::BinaryOp::BitwiseXor;
    case AST::BinaryOp::BitShiftLeft:
        return TACKY::BinaryOp::BitShiftLeft;
    case AST::BinaryOp::BitShiftRight:
        return TACKY::BinaryOp::BitShiftRight;
    default:
        throw std::runtime_error("Internal Error: Invalid Binary operator!");
    }
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForDoLoop(const std::shared_ptr<AST::DoWhile> &doLoop)
{
    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

    auto startLabel = UniqueIds::makeLabel("do_loop_start");
    auto contLabel = continueLabel(doLoop->getId());
    auto brkLabel = breakLabel(doLoop->getId());
    auto bodyInsts = emitTackyForStatement(doLoop->getBody());
    auto [evalCond, c] = emitTackyForExp(doLoop->getCondition());

    insts.push_back(std::make_shared<TACKY::Label>(startLabel));
    insts.insert(insts.end(), bodyInsts.begin(), bodyInsts.end());
    insts.push_back(std::make_shared<TACKY::Label>(contLabel));
    insts.insert(insts.end(), evalCond.begin(), evalCond.end());
    insts.push_back(std::make_shared<TACKY::JumpIfNotZero>(c, startLabel));
    insts.push_back(std::make_shared<TACKY::Label>(brkLabel));

    return insts;
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForWhileLoop(const std::shared_ptr<AST::While> &whileLoop)
{
    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

    auto contLabel = continueLabel(whileLoop->getId());
    auto brkLabel = breakLabel(whileLoop->getId());
    auto [evalCond, c] = emitTackyForExp(whileLoop->getCondition());
    auto bodyInsts = emitTackyForStatement(whileLoop->getBody());

    insts.push_back(std::make_shared<TACKY::Label>(contLabel));
    insts.insert(insts.end(), evalCond.begin(), evalCond.end());
    insts.push_back(std::make_shared<TACKY::JumpIfZero>(c, brkLabel));
    insts.insert(insts.end(), bodyInsts.begin(), bodyInsts.end());
    insts.push_back(std::make_shared<TACKY::Jump>(contLabel));
    insts.push_back(std::make_shared<TACKY::Label>(brkLabel));

    return insts;
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForForLoop(const std::shared_ptr<AST::For> &forLoop)
{
    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

    auto startLabel = UniqueIds::makeLabel("for_start");
    auto contLabel = continueLabel(forLoop->getId());
    auto brkLabel = breakLabel(forLoop->getId());
    auto bodyInsts = emitTackyForStatement(forLoop->getBody());

    auto forInitInsts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    if (auto initDecl = std::dynamic_pointer_cast<AST::InitDecl>(forLoop->getInit()))
    {
        auto innerInsts = emitVarDeclaration(initDecl->getDecl());
        forInitInsts.insert(forInitInsts.end(), innerInsts.begin(), innerInsts.end());
    }
    else if (auto initExp = std::dynamic_pointer_cast<AST::InitExp>(forLoop->getInit()))
    {
        if (initExp->getOptExp().has_value())
        {
            auto [innerInsts, _] = emitTackyForExp(initExp->getOptExp().value());
            forInitInsts.insert(forInitInsts.end(), innerInsts.begin(), innerInsts.end());
        }
    }

    auto testCondInsts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    if (forLoop->getOptCondition().has_value())
    {
        auto [innerInsts, c] = emitTackyForExp(forLoop->getOptCondition().value());
        testCondInsts.insert(testCondInsts.end(), innerInsts.begin(), innerInsts.end());
        testCondInsts.push_back(std::make_shared<TACKY::JumpIfZero>(c, brkLabel));
    }

    auto postInsts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    if (forLoop->getOptPost().has_value())
    {
        auto [innerInsts, _] = emitTackyForExp(forLoop->getOptPost().value());
        postInsts.insert(postInsts.end(), innerInsts.begin(), innerInsts.end());
    }

    insts.insert(insts.end(), forInitInsts.begin(), forInitInsts.end());
    insts.push_back(std::make_shared<TACKY::Label>(startLabel));
    insts.insert(insts.end(), testCondInsts.begin(), testCondInsts.end());
    insts.insert(insts.end(), bodyInsts.begin(), bodyInsts.end());
    insts.push_back(std::make_shared<TACKY::Label>(contLabel));
    insts.insert(insts.end(), postInsts.begin(), postInsts.end());
    insts.push_back(std::make_shared<TACKY::Jump>(startLabel));
    insts.push_back(std::make_shared<TACKY::Label>(brkLabel));

    return insts;
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitFunCall(const std::shared_ptr<AST::FunctionCall> &fnCall)
{
    auto dstName = createTmp(fnCall->getDataType());
    auto dst = std::make_shared<TACKY::Var>(dstName);

    std::vector<std::shared_ptr<TACKY::Instruction>> argInsts{};
    std::vector<std::shared_ptr<TACKY::Val>> argVal{};

    for (const auto &arg : fnCall->getArgs())
    {
        auto [insts, v] = emitTackyForExp(arg);
        argInsts.insert(argInsts.end(), insts.begin(), insts.end());
        argVal.push_back(v);
    }

    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    insts.insert(insts.end(), argInsts.begin(), argInsts.end());
    insts.push_back(std::make_shared<TACKY::FunCall>(fnCall->getName(), argVal, dst));

    return {
        insts,
        dst,
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitConditionalExp(const std::shared_ptr<AST::Conditional> &conditional)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> insts{};

    auto [evalCond, v] = emitTackyForExp(conditional->getCondition());
    auto [evalV1, v1] = emitTackyForExp(conditional->getThen());
    auto [evalV2, v2] = emitTackyForExp(conditional->getElse());

    auto e2Label = UniqueIds::makeLabel("conditional_else");
    auto endLabel = UniqueIds::makeLabel("conditional_end");
    auto dstName = createTmp(conditional->getDataType());
    auto dst = std::make_shared<TACKY::Var>(dstName);

    insts.insert(insts.end(), evalCond.begin(), evalCond.end());
    insts.push_back(std::make_shared<TACKY::JumpIfZero>(v, e2Label));
    insts.insert(insts.end(), evalV1.begin(), evalV1.end());
    insts.push_back(std::make_shared<TACKY::Copy>(v1, dst));
    insts.push_back(std::make_shared<TACKY::Jump>(endLabel));
    insts.push_back(std::make_shared<TACKY::Label>(e2Label));
    insts.insert(insts.end(), evalV2.begin(), evalV2.end());
    insts.push_back(std::make_shared<TACKY::Copy>(v2, dst));
    insts.push_back(std::make_shared<TACKY::Label>(endLabel));

    return {
        insts,
        dst,
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitPostfix(const AST::BinaryOp &op, const std::shared_ptr<AST::Expression> &exp)
{
    if (auto var = std::dynamic_pointer_cast<AST::Var>(exp))
    {
        auto dst = std::make_shared<TACKY::Var>(createTmp(var->getDataType()));
        auto tackyVar = std::make_shared<TACKY::Var>(var->getName());

        std::vector<std::shared_ptr<TACKY::Instruction>> insts{
            std::make_shared<TACKY::Copy>(tackyVar, dst),
            std::make_shared<TACKY::Binary>(convertBinop(op), tackyVar, std::make_shared<TACKY::Constant>(mkConst(var->getDataType(), 1)), tackyVar),
        };

        return {
            insts,
            dst,
        };
    }

    throw std::runtime_error("Invalid lvalue");
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitCompoundExpression(const AST::BinaryOp &op, const std::shared_ptr<AST::Expression> &lhs, const std::shared_ptr<AST::Expression> &rhs, const std::optional<Types::DataType> &resultType)
{
    if (!resultType.has_value() || !lhs->getDataType().has_value())
        throw std::runtime_error("Internal error: Data type is null");

    // Make sure it's an lvalue
    if (auto var = std::dynamic_pointer_cast<AST::Var>(lhs))
    {
        // evaluate RHS - TypeChecker already added conversion to common type if one needed
        auto [evalRhs, rhsResult] = emitTackyForExp(rhs);
        auto dst = std::make_shared<TACKY::Var>(var->getName());
        auto tackyOp = convertBinop(op);

        std::vector<std::shared_ptr<TACKY::Instruction>> insts{};
        insts.insert(insts.end(), evalRhs.begin(), evalRhs.end());

        if (resultType.value() == var->getDataType().value())
        {
            // result of binary operation already has correct  destination type
            insts.push_back(std::make_shared<TACKY::Binary>(tackyOp, dst, rhsResult, dst));
        }
        else
        {
            // must convert LHS to op type, then convert result back, so we'll have
            // tmp = <cast v to result_type>
            // tmp = tmp op rhs
            // lhs = <cast tmp to lhs_type>

            auto tmp = std::make_shared<TACKY::Var>(createTmp(resultType));
            auto castLhsToTmp = getCastInst(dst, tmp, var->getDataType().value(), resultType.value());
            auto binaryInst = std::make_shared<TACKY::Binary>(tackyOp, tmp, rhsResult, tmp);
            auto castTmpToLhs = getCastInst(tmp, dst, resultType.value(), var->getDataType().value());

            insts.push_back(castLhsToTmp);
            insts.push_back(binaryInst);
            insts.push_back(castTmpToLhs);
        }

        return {
            insts,
            dst,
        };
    }
    else
    {
        throw std::runtime_error("bad lvalue in compound assignment or prefix incr/decr");
    }
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitAndExp(const std::shared_ptr<AST::Binary> &binary)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> innerEval{};

    auto [innerEval1, v1] = emitTackyForExp(binary->getExp1());
    auto [innerEval2, v2] = emitTackyForExp(binary->getExp2());

    auto falseLabel = UniqueIds::makeLabel("and_false");
    auto endLabel = UniqueIds::makeLabel("and_end");

    auto dstName = createTmp(std::make_optional(Types::makeIntType()));
    auto dst = std::make_shared<TACKY::Var>(dstName);

    innerEval.insert(innerEval.end(), innerEval1.begin(), innerEval1.end());
    innerEval.push_back(std::make_shared<TACKY::JumpIfZero>(v1, falseLabel));
    innerEval.insert(innerEval.end(), innerEval2.begin(), innerEval2.end());
    innerEval.push_back(std::make_shared<TACKY::JumpIfZero>(v2, falseLabel));
    innerEval.push_back(std::make_shared<TACKY::Copy>(std::make_shared<TACKY::Constant>(std::make_shared<Constants::Const>(Constants::makeConstInt(1))), dst));
    innerEval.push_back(std::make_shared<TACKY::Jump>(endLabel));
    innerEval.push_back(std::make_shared<TACKY::Label>(falseLabel));
    innerEval.push_back(std::make_shared<TACKY::Copy>(std::make_shared<TACKY::Constant>(std::make_shared<Constants::Const>(Constants::makeConstInt(0))), dst));
    innerEval.push_back(std::make_shared<TACKY::Label>(endLabel));

    return {
        innerEval,
        dst,
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitOrExp(const std::shared_ptr<AST::Binary> &binary)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> innerEval{};

    auto [innerEval1, v1] = emitTackyForExp(binary->getExp1());
    auto [innerEval2, v2] = emitTackyForExp(binary->getExp2());

    auto trueLabel = UniqueIds::makeLabel("or_true");
    auto endLabel = UniqueIds::makeLabel("or_end");

    auto dstName = createTmp(std::make_optional(Types::makeIntType()));
    auto dst = std::make_shared<TACKY::Var>(dstName);

    innerEval.insert(innerEval.end(), innerEval1.begin(), innerEval1.end());
    innerEval.push_back(std::make_shared<TACKY::JumpIfNotZero>(v1, trueLabel));
    innerEval.insert(innerEval.end(), innerEval2.begin(), innerEval2.end());
    innerEval.push_back(std::make_shared<TACKY::JumpIfNotZero>(v2, trueLabel));
    innerEval.push_back(std::make_shared<TACKY::Copy>(std::make_shared<TACKY::Constant>(std::make_shared<Constants::Const>(Constants::makeConstInt(0))), dst));
    innerEval.push_back(std::make_shared<TACKY::Jump>(endLabel));
    innerEval.push_back(std::make_shared<TACKY::Label>(trueLabel));
    innerEval.push_back(std::make_shared<TACKY::Copy>(std::make_shared<TACKY::Constant>(std::make_shared<Constants::Const>(Constants::makeConstInt(1))), dst));
    innerEval.push_back(std::make_shared<TACKY::Label>(endLabel));

    return {
        innerEval,
        dst,
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitCastExp(const std::shared_ptr<AST::Cast> &cast)
{
    auto [innerEval, res] = emitTackyForExp(cast->getExp());
    auto optSrcType = cast->getExp()->getDataType();
    if (!optSrcType.has_value())
        throw std::runtime_error("Internal error: Cast to funtion type");

    if (optSrcType.value() == cast->getTargetType())
    {
        return {
            innerEval,
            res,
        };
    }

    auto dstName = createTmp(cast->getTargetType());
    auto dst = std::make_shared<TACKY::Var>(dstName);
    auto castInst = getCastInst(res, dst, *optSrcType, cast->getTargetType());

    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    insts.insert(insts.end(), innerEval.begin(), innerEval.end());
    insts.push_back(castInst);

    return {
        insts,
        dst,
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitAssignment(const std::string &varName, const std::shared_ptr<AST::Expression> &rhs)
{
    auto var = std::make_shared<TACKY::Var>(varName);
    auto [rhsInsts, rhsRes] = emitTackyForExp(rhs);

    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    insts.insert(insts.end(), rhsInsts.begin(), rhsInsts.end());
    insts.push_back(std::make_shared<TACKY::Copy>(rhsRes, var));

    return {
        insts,
        var,
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitUnaryExp(const std::shared_ptr<AST::Unary> &unary)
{
    auto [innerEval, src] = emitTackyForExp(unary->getExp());

    auto op = convertUnop(unary->getOp());
    auto dstName = createTmp(unary->getDataType());
    auto dst = std::make_shared<TACKY::Var>(dstName);

    innerEval.push_back(std::make_shared<TACKY::Unary>(op, src, dst));

    return {
        innerEval,
        dst,
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitBinaryExp(const std::shared_ptr<AST::Binary> &binary)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> innerEval{};

    auto [innerEval1, src1] = emitTackyForExp(binary->getExp1());
    auto [innerEval2, src2] = emitTackyForExp(binary->getExp2());

    innerEval.insert(innerEval.end(), innerEval1.begin(), innerEval1.end());
    innerEval.insert(innerEval.end(), innerEval2.begin(), innerEval2.end());

    auto dstName = createTmp(binary->getDataType());
    auto dst = std::make_shared<TACKY::Var>(dstName);
    auto op = convertBinop(binary->getOp());

    innerEval.push_back(std::make_shared<TACKY::Binary>(op, src1, src2, dst));

    return {
        innerEval,
        dst,
    };
}

std::pair<
    std::vector<std::shared_ptr<TACKY::Instruction>>,
    std::shared_ptr<TACKY::Val>>
TackyGen::emitTackyForExp(const std::shared_ptr<AST::Expression> &exp)
{
    switch (exp->getType())
    {
    case AST::NodeType::Constant:
    {
        return {
            {},
            std::make_shared<TACKY::Constant>(std::dynamic_pointer_cast<AST::Constant>(exp)->getConst())};
    }
    case AST::NodeType::Var:
    {
        return {
            {},
            std::make_shared<TACKY::Var>(std::dynamic_pointer_cast<AST::Var>(exp)->getName()),
        };
    }
    case AST::NodeType::Unary:
    {
        auto unary = std::dynamic_pointer_cast<AST::Unary>(exp);

        switch (unary->getOp())
        {
        case AST::UnaryOp::Incr:
            return emitCompoundExpression(AST::BinaryOp::Add, unary->getExp(), mkAstConst(unary->getDataType(), 1), unary->getDataType());
        case AST::UnaryOp::Decr:
            return emitCompoundExpression(AST::BinaryOp::Subtract, unary->getExp(), mkAstConst(unary->getDataType(), 1), unary->getDataType());
        default:
            return emitUnaryExp(unary);
        }
    }
    case AST::NodeType::Cast:
        return emitCastExp(std::dynamic_pointer_cast<AST::Cast>(exp));
    case AST::NodeType::Binary:
    {
        auto binary = std::dynamic_pointer_cast<AST::Binary>(exp);
        switch (binary->getOp())
        {
        case AST::BinaryOp::And:
            return emitAndExp(binary);
        case AST::BinaryOp::Or:
            return emitOrExp(binary);
        default:
            return emitBinaryExp(binary);
        }
    }
    case AST::NodeType::Assignment:
    {
        auto assignment = std::dynamic_pointer_cast<AST::Assignment>(exp);

        if (auto lhs = std::dynamic_pointer_cast<AST::Var>(assignment->getLeftExp()))
            return emitAssignment(lhs->getName(), assignment->getRightExp());
        else
            throw std::runtime_error("Bad lvalue!");
    }

    case AST::NodeType::CompoundAssignment:
    {
        auto compoundAssignment = std::dynamic_pointer_cast<AST::CompoundAssignment>(exp);
        return emitCompoundExpression(compoundAssignment->getOp(), compoundAssignment->getLeftExp(), compoundAssignment->getRightExp(), compoundAssignment->getResultType());
    }
    case AST::NodeType::PostfixIncr:
    {
        auto postfixIncr = std::dynamic_pointer_cast<AST::PostfixIncr>(exp);
        return emitPostfix(AST::BinaryOp::Add, postfixIncr->getExp());
    }
    case AST::NodeType::PostfixDecr:
    {
        auto postfixDecr = std::dynamic_pointer_cast<AST::PostfixDecr>(exp);
        return emitPostfix(AST::BinaryOp::Subtract, postfixDecr->getExp());
    }
    case AST::NodeType::Conditional:
    {
        return emitConditionalExp(std::dynamic_pointer_cast<AST::Conditional>(exp));
    }
    case AST::NodeType::FunctionCall:
    {
        return emitFunCall(std::dynamic_pointer_cast<AST::FunctionCall>(exp));
    }
    default:
        throw std::invalid_argument("Internal error: Invalid expression");
    }
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForIfStatement(const std::shared_ptr<AST::If> &ifStmt)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> insts{};

    if (!ifStmt->getOptElseClause().has_value())
    {
        auto endLabel = UniqueIds::makeLabel("if_end");

        auto [evalCond, c] = emitTackyForExp(ifStmt->getCondition());
        auto evalThen = emitTackyForStatement(ifStmt->getThenClause());

        insts.insert(insts.end(), evalCond.begin(), evalCond.end());
        insts.push_back(std::make_shared<TACKY::JumpIfZero>(c, endLabel));
        insts.insert(insts.end(), evalThen.begin(), evalThen.end());
        insts.push_back(std::make_shared<TACKY::Label>(endLabel));
    }
    else
    {
        auto elseLabel = UniqueIds::makeLabel("if_else");
        auto endLabel = UniqueIds::makeLabel("if_end");

        auto [evalCond, c] = emitTackyForExp(ifStmt->getCondition());
        auto evalThen = emitTackyForStatement(ifStmt->getThenClause());
        auto evalElse = emitTackyForStatement(ifStmt->getOptElseClause().value());

        insts.insert(insts.end(), evalCond.begin(), evalCond.end());
        insts.push_back(std::make_shared<TACKY::JumpIfZero>(c, elseLabel));
        insts.insert(insts.end(), evalThen.begin(), evalThen.end());
        insts.push_back(std::make_shared<TACKY::Jump>(endLabel));
        insts.push_back(std::make_shared<TACKY::Label>(elseLabel));
        insts.insert(insts.end(), evalElse.begin(), evalElse.end());
        insts.push_back(std::make_shared<TACKY::Label>(endLabel));
    }

    return insts;
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForSwitch(const std::shared_ptr<AST::Switch> &switchStmt)
{
    auto brkLabel = breakLabel(switchStmt->getId());
    auto [evalControl, c] = emitTackyForExp(switchStmt->getControl());
    auto cmpResult = std::make_shared<TACKY::Var>(createTmp(switchStmt->getControl()->getDataType()));

    if (!switchStmt->getOptCases().has_value())
    {
        throw std::runtime_error("Switch case map is not defined!");
    }

    auto cases = switchStmt->getOptCases().value();

    std::vector<std::shared_ptr<TACKY::Instruction>> jumpToCases{};

    std::optional<std::string> defaultCaseId = std::nullopt;
    for (const auto &[key, id] : cases)
    {
        if (key.has_value()) // It's a case statement
        {
            jumpToCases.push_back(
                std::make_shared<TACKY::Binary>(
                    TACKY::BinaryOp::Equal,
                    std::make_shared<TACKY::Constant>(key.value()),
                    c,
                    cmpResult));

            jumpToCases.push_back(
                std::make_shared<TACKY::JumpIfNotZero>(cmpResult, id));
        }
        else
        {
            // A default case, we'll treat it later below
            defaultCaseId = std::make_optional(id);
        }
    }

    std::shared_ptr<TACKY::Instruction> defaultTacky;

    if (defaultCaseId.has_value())
        defaultTacky = std::make_shared<TACKY::Jump>(defaultCaseId.value());

    auto bodyTacky = emitTackyForStatement(switchStmt->getBody());

    std::vector<std::shared_ptr<TACKY::Instruction>> insts{};

    insts.insert(insts.end(), evalControl.begin(), evalControl.end());
    insts.insert(insts.end(), jumpToCases.begin(), jumpToCases.end());
    if (defaultTacky)
        insts.push_back(defaultTacky);
    insts.push_back(std::make_shared<TACKY::Jump>(brkLabel));
    insts.insert(insts.end(), bodyTacky.begin(), bodyTacky.end());
    insts.push_back(std::make_shared<TACKY::Label>(brkLabel));

    return insts;
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForStatement(const std::shared_ptr<AST::Statement> &stmt)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Return:
    {
        auto [insts, v] = emitTackyForExp(std::dynamic_pointer_cast<AST::Return>(stmt)->getValue());
        insts.push_back(std::make_shared<TACKY::Return>(v));

        return insts;
    }
    case AST::NodeType::ExpressionStmt:
    {
        auto [insts, v] = emitTackyForExp(std::dynamic_pointer_cast<AST::ExpressionStmt>(stmt)->getExp()); // Discard the evaluated v destination, we only care the side effect

        return insts;
    }
    case AST::NodeType::If:
    {
        return emitTackyForIfStatement(std::dynamic_pointer_cast<AST::If>(stmt));
    }
    case AST::NodeType::Compound:
    {
        auto compoundStmt = std::dynamic_pointer_cast<AST::Compound>(stmt);
        auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

        for (auto &blockItem : compoundStmt->getBlock())
        {
            auto innerInsts = emitTackyForBlockItem(blockItem);
            insts.insert(insts.end(), innerInsts.begin(), innerInsts.end());
        }

        return insts;
    }
    case AST::NodeType::LabeledStatement:
    {
        auto labeledStmt = std::dynamic_pointer_cast<AST::LabeledStatement>(stmt);

        auto insts = emitTackyForStatement(labeledStmt->getStatement());
        insts.insert(insts.begin(), std::make_shared<TACKY::Label>(labeledStmt->getLabel()));

        return insts;
    }
    case AST::NodeType::Goto:
    {
        return {
            std::make_shared<TACKY::Jump>(std::dynamic_pointer_cast<AST::Goto>(stmt)->getLabel()),
        };
    }
    case AST::NodeType::Break:
    {
        return {
            std::make_shared<TACKY::Jump>(breakLabel(std::dynamic_pointer_cast<AST::Break>(stmt)->getId())),
        };
    }
    case AST::NodeType::Continue:
    {
        return {
            std::make_shared<TACKY::Jump>(continueLabel(std::dynamic_pointer_cast<AST::Continue>(stmt)->getId())),
        };
    }
    case AST::NodeType::While:
    {
        return emitTackyForWhileLoop(std::dynamic_pointer_cast<AST::While>(stmt));
    }
    case AST::NodeType::DoWhile:
    {
        return emitTackyForDoLoop(std::dynamic_pointer_cast<AST::DoWhile>(stmt));
    }
    case AST::NodeType::For:
    {
        return emitTackyForForLoop(std::dynamic_pointer_cast<AST::For>(stmt));
    }
    case AST::NodeType::Switch:
    {
        return emitTackyForSwitch(std::dynamic_pointer_cast<AST::Switch>(stmt));
    }
    case AST::NodeType::Case:
    {
        auto caseStmt = std::dynamic_pointer_cast<AST::Case>(stmt);
        auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

        insts.push_back(std::make_shared<TACKY::Label>(caseStmt->getId()));
        auto innerInsts = emitTackyForStatement(caseStmt->getBody());
        insts.insert(insts.end(), innerInsts.begin(), innerInsts.end());

        return insts;
    }
    case AST::NodeType::Default:
    {
        auto defaultStmt = std::dynamic_pointer_cast<AST::Default>(stmt);
        auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

        insts.push_back(std::make_shared<TACKY::Label>(defaultStmt->getId()));
        auto innerInsts = emitTackyForStatement(defaultStmt->getBody());
        insts.insert(insts.end(), innerInsts.begin(), innerInsts.end());

        return insts;
    }
    case AST::NodeType::Null:
    {
        return {};
    }
    default:
        throw std::invalid_argument("Internal error: Invalid statement");
    }
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitLocalDeclaration(const std::shared_ptr<AST::Declaration> &decl)
{
    if (auto varDecl = std::dynamic_pointer_cast<AST::VariableDeclaration>(decl))
    {
        if (varDecl->getOptStorageClass().has_value())
        {
            // With storage class in local declaration, variable should have been processed in TypeChecking pass
            // thus it should be already in symbol table
            // We don't process them here.
            return {};
        }
        return emitVarDeclaration(std::dynamic_pointer_cast<AST::VariableDeclaration>(decl));
    }
    else
        return {};
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitVarDeclaration(const std::shared_ptr<AST::VariableDeclaration> &varDecl)
{
    if (varDecl->getOptInit().has_value())
    {
        // Treat declaration with initializer as assignment
        auto [evalAssignment, _] = emitAssignment(
            varDecl->getName(),
            varDecl->getOptInit().value());

        return evalAssignment;
    }

    return {};
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem)
{
    switch (blockItem->getType())
    {
    case AST::NodeType::FunctionDeclaration:
    case AST::NodeType::VariableDeclaration:
    {
        return emitLocalDeclaration(std::dynamic_pointer_cast<AST::Declaration>(blockItem));
    }
    default:
        return emitTackyForStatement(std::dynamic_pointer_cast<AST::Statement>(blockItem));
    }
}

std::optional<std::shared_ptr<TACKY::Function>>
TackyGen::emitFunDeclaration(const std::shared_ptr<AST::FunctionDeclaration> &fnDecl)
{
    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

    if (fnDecl->getOptBody().has_value())
    {
        bool global = _symbolTable.isGlobal(fnDecl->getName());

        for (auto &blockItem : fnDecl->getOptBody().value())
        {
            auto innerInsts = emitTackyForBlockItem(blockItem);
            insts.insert(insts.end(), innerInsts.begin(), innerInsts.end());
        }

        auto extraReturn = std::make_shared<TACKY::Return>(
            std::make_shared<TACKY::Constant>(
                std::make_shared<Constants::Const>(Constants::makeConstInt(0))));
        insts.push_back(extraReturn);

        return std::make_optional(std::make_shared<TACKY::Function>(fnDecl->getName(), global, fnDecl->getParams(), insts));
    }

    return std::nullopt;
}

std::vector<std::shared_ptr<TACKY::StaticVariable>>
TackyGen::convertSymbolsToTacky()
{
    std::vector<std::shared_ptr<TACKY::StaticVariable>> staticVars{};

    for (const auto &[name, symbol] : _symbolTable.getAllSymbols())
    {
        if (auto staticAttrs = Symbols::getStaticAttr(symbol.attrs))
        {
            if (auto initial = Symbols::getInitial(staticAttrs->init))
            {
                staticVars.push_back(
                    std::make_shared<TACKY::StaticVariable>(
                        name,
                        staticAttrs->global,
                        symbol.type,
                        initial.value().staticInit));
            }
            else if (auto tentative = Symbols::getTentative(staticAttrs->init))
            {
                staticVars.push_back(
                    std::make_shared<TACKY::StaticVariable>(
                        name,
                        staticAttrs->global,
                        symbol.type,
                        Initializers::zero(symbol.type)));
            }
            else
            {
                // staticAttrs is a NoInitializer, do nothing
            }
        }
        else
        {
            // Do nothing
        }
    }

    return staticVars;
}

std::shared_ptr<TACKY::Program>
TackyGen::gen(const std::shared_ptr<AST::Program> &prog)
{
    auto tackyFnDefs = std::vector<std::shared_ptr<TACKY::Function>>{};

    for (const auto &decl : prog->getDeclarations())
    {
        if (auto fnDecl = std::dynamic_pointer_cast<AST::FunctionDeclaration>(decl))
        {
            auto newFn = emitFunDeclaration(fnDecl);
            if (newFn.has_value())
                tackyFnDefs.push_back(newFn.value());
        }
    }

    auto tackyVarDefs = std::vector<std::shared_ptr<TACKY::StaticVariable>>{convertSymbolsToTacky()};

    auto tackyDefs = std::vector<std::shared_ptr<TACKY::TopLevel>>{};
    std::copy(tackyVarDefs.begin(), tackyVarDefs.end(), std::back_inserter(tackyDefs));
    std::copy(tackyFnDefs.begin(), tackyFnDefs.end(), std::back_inserter(tackyDefs));

    return std::make_shared<TACKY::Program>(tackyDefs);
}
