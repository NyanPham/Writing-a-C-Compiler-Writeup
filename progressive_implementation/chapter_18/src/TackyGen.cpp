#include <vector>
#include <algorithm>
#include <iostream>

#include "TackyGen.h"
#include "Tacky.h"
#include "AST.h"
#include "UniqueIds.h"
#include "ConstConvert.h"
#include "TypeTable.h"
#include "Symbols.h"

// use this as the "result" of void expressions that don't return a result
auto dummyOperand = std::make_shared<TACKY::Constant>(std::make_shared<Constants::Const>(Constants::makeIntZero()));

ssize_t
TackyGen::getPtrScale(const Types::DataType &type)
{
    if (auto ptrType = Types::getPointerType(type))
    {
        return Types::getSize(*ptrType->referencedType, _typeTable);
    }
    else
    {
        throw std::runtime_error("Internal error: tried to get scale of non-pointer type");
    }
}

// An expression result that may or may not be lvalue converted
struct PlainOperand
{
    std::shared_ptr<TACKY::Val> val;
    std::string toString() const
    {
        return "PlainOperand";
    }
};

struct DereferencedPointer
{
    std::shared_ptr<TACKY::Val> val;
    std::string toString() const
    {
        return "DereferencedPointer";
    }
};

struct SubObject
{
    std::string base;
    int offset;

    std::string toString() const
    {
        return "SubObject(base=" + base + ", offset=" + std::to_string(offset) + ")";
    }
};

using ExpResult = std::variant<PlainOperand, DereferencedPointer, SubObject>;

std::string expResultToString(const ExpResult &expRes)
{
    return std::visit([](const auto &v)
                      { return v.toString(); }, expRes);
}

int TackyGen::getMemberOffset(const std::string &member, const Types::DataType &type)
{
    if (auto strctType = Types::getStructType(type))
    {
        auto members = _typeTable.find(strctType->tag).members;
        auto it = members.find(member);
        if (it == members.end())
            throw std::runtime_error("Internal error: failed to find member in struct");
        else
            return it->second.offset;
    }
    else
    {
        throw std::runtime_error("Internal error: tried to get offset of member within non-structure type");
    }
}

int TackyGen::getMemberPointerOffset(const std::string &member, const Types::DataType &type)
{
    if (auto ptrType = Types::getPointerType(type))
        return getMemberOffset(member, *ptrType->referencedType);
    else
        throw std::runtime_error("Internal error: trying to get member through pointer but is not a pointer type");
}

std::shared_ptr<TACKY::Constant>
TackyGen::evalSize(const Types::DataType &type)
{
    auto size = Types::getSize(type, _typeTable);
    return std::make_shared<TACKY::Constant>(std::make_shared<Constants::Const>(Constants::makeConstLong(size)));
}

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
    if (Types::getSize(dstType, _typeTable) == Types::getSize(srcType, _typeTable))
        return std::make_shared<TACKY::Copy>(src, dst);
    else if (Types::getSize(dstType, _typeTable) < Types::getSize(srcType, _typeTable))
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
    auto [evalCond, c] = emitTackyAndConvert(doLoop->getCondition());

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
    auto [evalCond, c] = emitTackyAndConvert(whileLoop->getCondition());
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
        auto [innerInsts, c] = emitTackyAndConvert(forLoop->getOptCondition().value());
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

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitFunCall(const std::shared_ptr<AST::FunctionCall> &fnCall)
{
    std::optional<std::shared_ptr<TACKY::Var>> dst = std::nullopt;

    if (Types::isVoidType(*fnCall->getDataType()))
    {
        dst = std::nullopt;
    }
    else
    {
        auto dstName = createTmp(fnCall->getDataType());
        dst = std::make_optional(std::make_shared<TACKY::Var>(dstName));
    }

    std::vector<std::shared_ptr<TACKY::Instruction>> argInsts{};
    std::vector<std::shared_ptr<TACKY::Val>> argVal{};

    for (const auto &arg : fnCall->getArgs())
    {
        auto [insts, v] = emitTackyAndConvert(arg);
        argInsts.insert(argInsts.end(), insts.begin(), insts.end());
        argVal.push_back(v);
    }

    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    insts.insert(insts.end(), argInsts.begin(), argInsts.end());
    insts.push_back(std::make_shared<TACKY::FunCall>(fnCall->getName(), argVal, dst));

    std::shared_ptr<TACKY::Val> dstVal = dst.has_value() ? std::static_pointer_cast<TACKY::Val>(dst.value()) : std::static_pointer_cast<TACKY::Val>(dummyOperand);

    return {
        insts,
        std::make_shared<ExpResult>(PlainOperand{dstVal}),
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitConditionalExp(const std::shared_ptr<AST::Conditional> &conditional)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> insts{};

    auto [evalCond, v] = emitTackyAndConvert(conditional->getCondition());
    auto [evalV1, v1] = emitTackyAndConvert(conditional->getThen());
    auto [evalV2, v2] = emitTackyAndConvert(conditional->getElse());

    auto e2Label = UniqueIds::makeLabel("conditional_else");
    auto endLabel = UniqueIds::makeLabel("conditional_end");

    std::shared_ptr<TACKY::Val> dst = nullptr;

    if (Types::isVoidType(*conditional->getDataType()))
    {
        dst = dummyOperand;
    }
    else
    {
        auto dstName = createTmp(conditional->getDataType());
        dst = std::make_shared<TACKY::Var>(dstName);
    }

    // common instructions
    insts.insert(insts.end(), evalCond.begin(), evalCond.end());
    insts.push_back(std::make_shared<TACKY::JumpIfZero>(v, e2Label));
    insts.insert(insts.end(), evalV1.begin(), evalV1.end());

    // remaining instructions
    if (Types::isVoidType(*conditional->getDataType()))
    {
        insts.push_back(std::make_shared<TACKY::Jump>(endLabel));
        insts.push_back(std::make_shared<TACKY::Label>(e2Label));
        insts.insert(insts.end(), evalV2.begin(), evalV2.end());
        insts.push_back(std::make_shared<TACKY::Label>(endLabel));
    }
    else
    {
        insts.push_back(std::make_shared<TACKY::Copy>(v1, dst));
        insts.push_back(std::make_shared<TACKY::Jump>(endLabel));
        insts.push_back(std::make_shared<TACKY::Label>(e2Label));
        insts.insert(insts.end(), evalV2.begin(), evalV2.end());
        insts.push_back(std::make_shared<TACKY::Copy>(v2, dst));
        insts.push_back(std::make_shared<TACKY::Label>(endLabel));
    }

    return {
        insts,
        std::make_shared<ExpResult>(PlainOperand{dst}),
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitPostfix(const AST::BinaryOp &op, const std::shared_ptr<AST::Expression> &exp)
{
    /*  If LHS is a variable:
            dst = lhs
            lhs = lhs <op> 1
        If LHS is a pointer:
            dst = load(ptr)
            tmp = dst <op> 1
            store(tmp, ptr)

        If LHS has a pointer type, we implement <op> with AddPtr
        otherwise with Binary instruction
    */

    // define var for result - i.e. value of lvalue BEFORE incr or decr
    auto dst = std::make_shared<TACKY::Var>(createTmp(exp->getDataType()));

    // evaluate inner to get ExpResult
    auto [innerEval, lval] = emitTackyForExp(exp);
    /*  Helper to construct Binary or AddPtr instruction from operands, depending on type
        Note that dst is destination of this instruction rather than the whole expression
        (i.e. it's the lvalue we're updating, or a temporary we'll then store to that lvalue)
    */

    auto doOp = [this, exp](const AST::BinaryOp op, const std::shared_ptr<TACKY::Val> &src, const std::shared_ptr<TACKY::Val> &dst) -> std::shared_ptr<TACKY::Instruction>
    {
        std::shared_ptr<TACKY::Constant> index = nullptr;

        if (op == AST::BinaryOp::Add)
            index = std::make_shared<TACKY::Constant>(mkConst(std::make_optional(Types::makeLongType()), 1));
        else if (op == AST::BinaryOp::Subtract)
            index = std::make_shared<TACKY::Constant>(mkConst(std::make_optional(Types::makeLongType()), -1));
        else
            throw std::runtime_error("Internal error");

        if (Types::isPointerType(*exp->getDataType()))
            return std::make_shared<TACKY::AddPtr>(src, index, getPtrScale(*exp->getDataType()), dst);
        else
        {
            auto one = std::make_shared<TACKY::Constant>(mkConst(exp->getDataType(), 1));
            return std::make_shared<TACKY::Binary>(convertBinop(op), src, one, dst);
        }
    };

    // copy result to dst and perform incr or decr
    auto operInsts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

    if (auto plainOperand = std::get_if<PlainOperand>(&(*lval)))
    {
        /*
            dst = v
            v = v + 1 // v - 1
        */
        operInsts.push_back(std::make_shared<TACKY::Copy>(plainOperand->val, dst));
        operInsts.push_back(doOp(op, plainOperand->val, plainOperand->val));
    }
    else if (auto derefPtr = std::get_if<DereferencedPointer>(&(*lval)))
    {
        /*
            dst = Load(p)
            tmp = dst + 1 // dst - 1
            Store(tmp, p)
        */
        auto tmp = std::make_shared<TACKY::Var>(createTmp(exp->getDataType()));
        operInsts.push_back(std::make_shared<TACKY::Load>(derefPtr->val, dst));
        operInsts.push_back(doOp(op, dst, tmp));
        operInsts.push_back(std::make_shared<TACKY::Store>(tmp, derefPtr->val));
    }
    else if (auto subObj = std::get_if<SubObject>(&(*lval)))
    {
        /*
            dst = CopyFromOffset(base, offset)
            tmp = dst + 1 // or dst - 1
            CopyToOffset(tmp, base, offset)
        */
        auto tmp = std::make_shared<TACKY::Var>(createTmp(exp->getDataType()));
        operInsts.push_back(std::make_shared<TACKY::CopyFromOffset>(subObj->base, subObj->offset, dst));
        operInsts.push_back(doOp(op, dst, tmp));
        operInsts.push_back(std::make_shared<TACKY::CopyToOffset>(tmp, subObj->base, subObj->offset));
    }

    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    insts.insert(insts.end(), innerEval.begin(), innerEval.end());
    insts.insert(insts.end(), operInsts.begin(), operInsts.end());

    return {
        insts,
        std::make_shared<ExpResult>(PlainOperand{dst}),
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitCompoundExpression(const AST::BinaryOp &op, const std::shared_ptr<AST::Expression> &lhs, const std::shared_ptr<AST::Expression> &rhs, const std::optional<Types::DataType> &resultType)
{
    /*
        if LHS is var w/ same type as result:
            lhs = lhs <op> rval
        if LHS is a var w/ different type:
            tmp = cast(lhs)
            tmp = tmp <op> rval
            lhs = cast(tmp)
        if LHS is pointer w/ same type:
            tmp = load(lhs_ptr)
            tmp = tmp <op> rval
            store(tmp, lhs_ptr)
        if LHS is pointer w/ diff type:
            tmp = load(lhs_ptr)
            tmp2 = cast(tmp)
            tmp2 = tmp2 <op> rval
            tmp = cast(tmp2)
            store(tmp, rhs_ptr)
        if LHS is subobject w/ same type:
            tmp = CopyFromOffset(lhs, offset)
            tmp = tmp <op> rval
            CopyToOffset(tmp, lhs, offset)
        if LHS is suboject w/ different type:
            tmp = CopyFromOffset(lhs, offset)
            tmp2 = cast(tmp)
            tmp2 = tmp2 <op> rval
            tmp = cast(tmp2)
            CopyToOffset(tmp, lhs, offset)
    */

    auto lhsType = lhs->getDataType().value();
    // evaluate LHS
    auto [evalLhs, _lhs] = emitTackyForExp(lhs);
    // evaluate RHS - type checker already added conversion to common type if one is needed
    auto [evalRhs, _rhs] = emitTackyAndConvert(rhs);
    /*
        If LHS is a variable, we can update it directly.
        If it's a dereferenced pointer, we need to load it into a temporary variable, operate on that, and then store it.
    */

    std::shared_ptr<TACKY::Val> dst = nullptr;
    auto loadInst = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    auto storeInst = std::vector<std::shared_ptr<TACKY::Instruction>>{};

    if (auto plainOperand = std::get_if<PlainOperand>(&(*_lhs)))
    {
        dst = plainOperand->val;
    }
    else if (auto derefPtr = std::get_if<DereferencedPointer>(&(*_lhs)))
    {
        dst = std::make_shared<TACKY::Var>(createTmp(lhsType));
        loadInst.push_back(std::make_shared<TACKY::Load>(derefPtr->val, dst));
        storeInst.push_back(std::make_shared<TACKY::Store>(dst, derefPtr->val));
    }
    else if (auto subObj = std::get_if<SubObject>(&(*_lhs)))
    {
        dst = std::make_shared<TACKY::Var>(createTmp(lhsType));
        loadInst.push_back(std::make_shared<TACKY::CopyFromOffset>(subObj->base, subObj->offset, dst));
        storeInst.push_back(std::make_shared<TACKY::CopyToOffset>(dst, subObj->base, subObj->offset));
    }

    /*
        If LHS type and result type are the same, we can operate on dst directly.
        Otherwise, we need to cast dst to correct type before operation, then cast result
        back and assign to dst
    */
    std::shared_ptr<TACKY::Val> resultVar = nullptr;
    auto castTo = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    auto castFrom = std::vector<std::shared_ptr<TACKY::Instruction>>{};

    if (lhsType == resultType.value())
    {
        resultVar = dst;
    }
    else
    {
        auto tmp = std::make_shared<TACKY::Var>(createTmp(resultType));
        auto castLhsToTmp = getCastInst(dst, tmp, lhsType, resultType.value());
        auto castTmpToLhs = getCastInst(tmp, dst, resultType.value(), lhsType);

        resultVar = tmp;
        castTo.push_back(castLhsToTmp);
        castFrom.push_back(castTmpToLhs);
    }

    auto doOperation = [this, resultType, op, resultVar, _rhs]() -> std::vector<std::shared_ptr<TACKY::Instruction>>
    {
        if (Types::isPointerType(*resultType))
        {
            auto scale = getPtrScale(*resultType);
            if (op == AST::BinaryOp::Add)
            {
                return {
                    std::make_shared<TACKY::AddPtr>(resultVar, _rhs, scale, resultVar),
                };
            }
            else if (op == AST::BinaryOp::Subtract)
            {
                auto negatedIndex = std::make_shared<TACKY::Var>(createTmp(Types::makeLongType()));
                return {
                    std::make_shared<TACKY::Unary>(TACKY::UnaryOp::Negate, _rhs, negatedIndex),
                    std::make_shared<TACKY::AddPtr>(resultVar, negatedIndex, scale, resultVar),
                };
            }
            else
            {
                throw std::runtime_error("Internal error in compound assignment");
            }
        }
        else
        {
            return {
                std::make_shared<TACKY::Binary>(convertBinop(op), resultVar, _rhs, resultVar),
            };
        }
    };

    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    auto operationInst = doOperation();

    insts.insert(insts.end(), evalLhs.begin(), evalLhs.end());
    insts.insert(insts.end(), evalRhs.begin(), evalRhs.end());
    insts.insert(insts.end(), loadInst.begin(), loadInst.end());
    insts.insert(insts.end(), castTo.begin(), castTo.end());
    insts.insert(insts.end(), operationInst.begin(), operationInst.end());
    insts.insert(insts.end(), castFrom.begin(), castFrom.end());
    insts.insert(insts.end(), storeInst.begin(), storeInst.end());

    return {
        insts,
        std::make_shared<ExpResult>(PlainOperand{dst}),
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitAndExp(const std::shared_ptr<AST::Binary> &binary)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> innerEval{};

    auto [innerEval1, v1] = emitTackyAndConvert(binary->getExp1());
    auto [innerEval2, v2] = emitTackyAndConvert(binary->getExp2());

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
        std::make_shared<ExpResult>(PlainOperand(dst)),
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitOrExp(const std::shared_ptr<AST::Binary> &binary)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> innerEval{};

    auto [innerEval1, v1] = emitTackyAndConvert(binary->getExp1());
    auto [innerEval2, v2] = emitTackyAndConvert(binary->getExp2());

    auto trueLabel = UniqueIds::makeLabel("or_true");
    auto endLabel = UniqueIds::makeLabel("or_end");

    auto dstName = createTmp(std::make_optional(Types::makeIntType()));
    auto dst = std::make_shared<TACKY::Var>(dstName);

    innerEval.insert(innerEval.end(), innerEval1.begin(), innerEval1.end());
    innerEval.push_back(std::make_shared<TACKY::JumpIfZero>(v1, trueLabel));
    innerEval.insert(innerEval.end(), innerEval2.begin(), innerEval2.end());
    innerEval.push_back(std::make_shared<TACKY::JumpIfZero>(v2, trueLabel));
    innerEval.push_back(std::make_shared<TACKY::Copy>(std::make_shared<TACKY::Constant>(std::make_shared<Constants::Const>(Constants::makeConstInt(0))), dst));
    innerEval.push_back(std::make_shared<TACKY::Jump>(endLabel));
    innerEval.push_back(std::make_shared<TACKY::Label>(trueLabel));
    innerEval.push_back(std::make_shared<TACKY::Copy>(std::make_shared<TACKY::Constant>(std::make_shared<Constants::Const>(Constants::makeConstInt(1))), dst));
    innerEval.push_back(std::make_shared<TACKY::Label>(endLabel));

    return {
        innerEval,
        std::make_shared<ExpResult>(PlainOperand(dst)),
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitCastExp(const std::shared_ptr<AST::Cast> &cast)
{
    auto [innerEval, res] = emitTackyAndConvert(cast->getExp());
    auto optSrcType = cast->getExp()->getDataType();
    if (!optSrcType.has_value())
        throw std::runtime_error("Internal error: Cast to funtion type");

    if (optSrcType.value() == cast->getTargetType() || Types::isVoidType(cast->getTargetType()))
    {
        return {
            innerEval,
            std::make_shared<ExpResult>(PlainOperand(res)),
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
        std::make_shared<ExpResult>(PlainOperand(dst)),
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitAssignment(const std::shared_ptr<AST::Expression> &lhs, const std::shared_ptr<AST::Expression> &rhs)
{
    auto [lhsInsts, lval] = emitTackyForExp(lhs);
    auto [rhsInsts, rval] = emitTackyAndConvert(rhs);

    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    insts.insert(insts.end(), lhsInsts.begin(), lhsInsts.end());
    insts.insert(insts.end(), rhsInsts.begin(), rhsInsts.end());

    if (auto plainOperand = std::get_if<PlainOperand>(&(*lval)))
    {
        insts.push_back(std::make_shared<TACKY::Copy>(rval, plainOperand->val));
    }
    else if (auto derefPtr = std::get_if<DereferencedPointer>(&(*lval)))
    {
        insts.push_back(std::make_shared<TACKY::Store>(rval, derefPtr->val));
    }
    else if (auto subObj = std::get_if<SubObject>(&(*lval)))
    {
        insts.push_back(std::make_shared<TACKY::CopyToOffset>(rval, subObj->base, subObj->offset));
    }

    return {
        insts,
        std::make_shared<ExpResult>(PlainOperand{rval}),
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitUnaryExp(const std::shared_ptr<AST::Unary> &unary)
{
    auto [innerEval, src] = emitTackyAndConvert(unary->getExp());

    auto dstName = createTmp(unary->getDataType());
    auto dst = std::make_shared<TACKY::Var>(dstName);
    auto op = convertUnop(unary->getOp());

    innerEval.push_back(std::make_shared<TACKY::Unary>(op, src, dst));

    return {
        innerEval,
        std::make_shared<ExpResult>(PlainOperand(dst)),
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitBinaryExp(const std::shared_ptr<AST::Binary> &binary)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> innerEval{};

    auto [innerEval1, src1] = emitTackyAndConvert(binary->getExp1());
    auto [innerEval2, src2] = emitTackyAndConvert(binary->getExp2());

    innerEval.insert(innerEval.end(), innerEval1.begin(), innerEval1.end());
    innerEval.insert(innerEval.end(), innerEval2.begin(), innerEval2.end());

    auto dstName = createTmp(binary->getDataType());
    auto dst = std::make_shared<TACKY::Var>(dstName);
    auto tackyOp = convertBinop(binary->getOp());

    innerEval.push_back(std::make_shared<TACKY::Binary>(tackyOp, src1, src2, dst));

    return {
        innerEval,
        std::make_shared<ExpResult>(PlainOperand(dst)),
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitPointerAddition(const Types::DataType &type, const std::shared_ptr<AST::Expression> &exp1, const std::shared_ptr<AST::Expression> &exp2)
{
    auto [eval1, v1] = emitTackyAndConvert(exp1);
    auto [eval2, v2] = emitTackyAndConvert(exp2);
    auto dstName = createTmp(std::make_optional(type));
    auto dst = std::make_shared<TACKY::Var>(dstName);

    std::shared_ptr<TACKY::Val> ptr = nullptr;
    std::shared_ptr<TACKY::Val> index = nullptr;

    if (type == *exp1->getDataType())
    {
        ptr = v1;
        index = v2;
    }
    else
    {
        ptr = v2;
        index = v1;
    }

    auto scale = getPtrScale(type);
    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

    insts.insert(insts.end(), eval1.begin(), eval1.end());
    insts.insert(insts.end(), eval2.begin(), eval2.end());
    insts.push_back(std::make_shared<TACKY::AddPtr>(ptr, index, scale, dst));

    return {
        insts,
        std::make_shared<ExpResult>(PlainOperand(dst)),
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitSubtractionFromPointer(const Types::DataType &type, const std::shared_ptr<AST::Expression> &ptrExp, const std::shared_ptr<AST::Expression> &indexExp)
{
    auto [eval1, ptr] = emitTackyAndConvert(ptrExp);
    auto [eval2, index] = emitTackyAndConvert(indexExp);
    auto dstName = createTmp(std::make_optional(type));
    auto dst = std::make_shared<TACKY::Var>(dstName);
    auto negatedIndex = std::make_shared<TACKY::Var>(createTmp(Types::makeLongType()));
    auto scale = getPtrScale(type);

    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    insts.insert(insts.end(), eval1.begin(), eval1.end());
    insts.insert(insts.end(), eval2.begin(), eval2.end());
    insts.push_back(std::make_shared<TACKY::Unary>(TACKY::UnaryOp::Negate, index, negatedIndex));
    insts.push_back(std::make_shared<TACKY::AddPtr>(ptr, negatedIndex, scale, dst));

    return {
        insts,
        std::make_shared<ExpResult>(PlainOperand(dst)),
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitPointerDiff(const Types::DataType &type, const std::shared_ptr<AST::Expression> &exp1, const std::shared_ptr<AST::Expression> &exp2)
{
    auto [eval1, v1] = emitTackyAndConvert(exp1);
    auto [eval2, v2] = emitTackyAndConvert(exp2);
    auto ptrDiff = std::make_shared<TACKY::Var>(createTmp(Types::makeLongType()));
    auto dstName = createTmp(std::make_optional(type));
    auto dst = std::make_shared<TACKY::Var>(dstName);
    auto scale = std::make_shared<TACKY::Constant>(std::make_shared<Constants::Const>(Constants::makeConstLong(static_cast<int64_t>(getPtrScale(*exp1->getDataType())))));

    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    insts.insert(insts.end(), eval1.begin(), eval1.end());
    insts.insert(insts.end(), eval2.begin(), eval2.end());
    insts.push_back(std::make_shared<TACKY::Binary>(TACKY::BinaryOp::Subtract, v1, v2, ptrDiff));
    insts.push_back(std::make_shared<TACKY::Binary>(TACKY::BinaryOp::Divide, ptrDiff, scale, dst));

    return {
        insts,
        std::make_shared<ExpResult>(PlainOperand(dst)),
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitDotOperator(const std::shared_ptr<AST::Dot> &dot)
{
    auto t = dot->getDataType().value();
    auto strct = dot->getStruct();
    auto member = dot->getMember();

    auto memberOffset = getMemberOffset(member, *strct->getDataType());
    auto [insts, innerObj] = emitTackyForExp(strct);

    auto plainOper = std::get_if<PlainOperand>(&(*innerObj));
    auto subObj = std::get_if<SubObject>(&(*innerObj));
    auto derefPtr = std::get_if<DereferencedPointer>(&(*innerObj));

    if (plainOper && plainOper->val->getType() == TACKY::NodeType::Var)
    {
        auto v = std::static_pointer_cast<TACKY::Var>(plainOper->val)->getName();
        return {
            insts,
            std::make_shared<ExpResult>(SubObject(v, memberOffset)),
        };
    }

    if (subObj)
    {
        return {
            insts,
            std::make_shared<ExpResult>(SubObject(subObj->base, subObj->offset + memberOffset)),
        };
    }

    if (derefPtr)
    {
        if (memberOffset == 0)
        {
            return {
                insts,
                innerObj,
            };
        }
        else
        {
            auto dst = std::make_shared<TACKY::Var>(
                createTmp(
                    std::make_optional(
                        Types::makePointerType(std::make_shared<Types::DataType>(t)))));
            auto index = std::make_shared<TACKY::Constant>(std::make_shared<Constants::Const>(Constants::makeConstLong(memberOffset)));
            auto addPtrInst = std::make_shared<TACKY::AddPtr>(derefPtr->val, index, 1, dst);
            insts.push_back(addPtrInst);

            return {
                insts,
                std::make_shared<ExpResult>(DereferencedPointer(dst)),
            };
        }
    }

    // is PlainOperand(Constant):
    throw std::runtime_error("Internal error: found dot operator applied to constant");
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitArrowOperator(const std::shared_ptr<AST::Arrow> &arrow)
{
    auto t = arrow->getDataType().value();
    auto strct = arrow->getStruct();
    auto member = arrow->getMember();

    auto memberOffset = getMemberPointerOffset(member, *strct->getDataType());
    auto [insts, ptr] = emitTackyAndConvert(strct);

    if (memberOffset == 0)
    {
        return {
            insts,
            std::make_shared<ExpResult>(DereferencedPointer(ptr)),
        };
    }
    else
    {
        auto dst = std::make_shared<TACKY::Var>(
            createTmp(
                std::make_optional(
                    Types::makePointerType(std::make_shared<Types::DataType>(t)))));
        auto index = std::make_shared<TACKY::Constant>(std::make_shared<Constants::Const>(Constants::makeConstLong(memberOffset)));
        auto addPtrInst = std::make_shared<TACKY::AddPtr>(ptr, index, 1, dst);
        insts.push_back(addPtrInst);

        return {
            insts,
            std::make_shared<ExpResult>(DereferencedPointer(dst)),
        };
    }
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitSubscript(const std::shared_ptr<AST::Subscript> &subscript)
{
    auto [insts, result] = emitPointerAddition(Types::makePointerType(std::make_shared<Types::DataType>(*subscript->getDataType())), subscript->getExp1(), subscript->getExp2());

    if (auto plainOperand = std::get_if<PlainOperand>(&(*result)))
    {
        return {
            insts,
            std::make_shared<ExpResult>(DereferencedPointer(plainOperand->val)),
        };
    }
    else
    {
        throw std::runtime_error("Internal error: expected result of pointer addition to be lvalue converted");
    }
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitDereference(const std::shared_ptr<AST::Dereference> &dereference)
{
    auto [insts, result] = emitTackyAndConvert(dereference->getInnerExp());
    return {
        insts,
        std::make_shared<ExpResult>(DereferencedPointer(result)),
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitAddrOf(const std::shared_ptr<AST::AddrOf> &addrOf)
{
    auto [insts, result] = emitTackyForExp(addrOf->getInnerExp());
    if (auto plainOperand = std::get_if<PlainOperand>(&(*result)))
    {
        auto dst = std::make_shared<TACKY::Var>(createTmp(addrOf->getDataType()));
        insts.push_back(
            std::make_shared<TACKY::GetAddress>(plainOperand->val, dst));
        return {
            insts,
            std::make_shared<ExpResult>(PlainOperand(dst)),
        };
    }
    else if (auto derefPtr = std::get_if<DereferencedPointer>(&(*result)))
    {
        return {
            insts,
            std::make_shared<ExpResult>(PlainOperand(derefPtr->val))};
    }
    else if (auto subObj = std::get_if<SubObject>(&(*result)))
    {
        auto dst = std::make_shared<TACKY::Var>(createTmp(addrOf->getDataType()));
        auto getAddr = std::make_shared<TACKY::GetAddress>(std::make_shared<TACKY::Var>(subObj->base), dst);
        insts.push_back(getAddr);
        if (subObj->offset == 0)
        {
            // skip AddrPtr if offset is 0
            return {
                insts,
                std::make_shared<ExpResult>(PlainOperand(dst)),
            };
        }
        else
        {
            auto index = std::make_shared<TACKY::Constant>(std::make_shared<Constants::Const>(Constants::makeConstLong(subObj->offset)));
            auto addPtr = std::make_shared<TACKY::AddPtr>(dst, index, 1, dst);
            insts.push_back(addPtr);
            return {
                insts,
                std::make_shared<ExpResult>(PlainOperand(dst)),
            };
        }
    }

    throw std::runtime_error("Internal error: Invalid expression for address of");
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitTackyAndConvert(const std::shared_ptr<AST::Expression> &exp)
{
    auto [insts, result] = emitTackyForExp(exp);
    if (auto plainOperand = std::get_if<PlainOperand>(&(*result)))
    {
        return {
            insts,
            plainOperand->val,
        };
    }
    else if (auto derefPtr = std::get_if<DereferencedPointer>(&(*result)))
    {
        auto dst = std::make_shared<TACKY::Var>(createTmp(exp->getDataType()));
        insts.push_back(std::make_shared<TACKY::Load>(derefPtr->val, dst));
        return {
            insts,
            dst,
        };
    }
    else if (auto subObj = std::get_if<SubObject>(&(*result)))
    {
        auto dst = std::make_shared<TACKY::Var>(createTmp(exp->getDataType()));
        insts.push_back(std::make_shared<TACKY::CopyFromOffset>(subObj->base, subObj->offset, dst));
        return {
            insts,
            dst,
        };
    }

    throw std::runtime_error("Internal error: Invalid expression for emitTackyAndConvert");
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>>
TackyGen::emitTackyForExp(const std::shared_ptr<AST::Expression> &exp)
{
    switch (exp->getType())
    {
    case AST::NodeType::Constant:
    {
        return {
            {},
            std::make_shared<ExpResult>(PlainOperand(std::make_shared<TACKY::Constant>(std::dynamic_pointer_cast<AST::Constant>(exp)->getConst())))};
    }
    case AST::NodeType::Var:
    {
        return {
            {},
            std::make_shared<ExpResult>(PlainOperand(std::make_shared<TACKY::Var>(std::dynamic_pointer_cast<AST::Var>(exp)->getName()))),
        };
    }
    case AST::NodeType::String:
    {
        auto string = std::dynamic_pointer_cast<AST::String>(exp);
        auto strId = _symbolTable.addString(string->getStr());

        return {
            {},
            std::make_shared<ExpResult>(PlainOperand(std::make_shared<TACKY::Var>(strId))),
        };
    }
    case AST::NodeType::Unary:
    {
        auto unary = std::dynamic_pointer_cast<AST::Unary>(exp);

        switch (unary->getOp())
        {
        case AST::UnaryOp::Incr:
        {
            auto constType = Types::isPointerType(*unary->getDataType()) ? std::make_optional(Types::makeLongType()) : unary->getDataType();
            return emitCompoundExpression(AST::BinaryOp::Add, unary->getExp(), mkAstConst(constType, 1), unary->getDataType());
        }
        case AST::UnaryOp::Decr:
        {
            auto constType = Types::isPointerType(*unary->getDataType()) ? std::make_optional(Types::makeLongType()) : unary->getDataType();
            return emitCompoundExpression(AST::BinaryOp::Subtract, unary->getExp(), mkAstConst(constType, 1), unary->getDataType());
        }
        default:
        {
            return emitUnaryExp(unary);
        }
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
        {
            return emitAndExp(binary);
        }
        case AST::BinaryOp::Or:
        {
            return emitOrExp(binary);
        }
        default:
        {
            if (binary->getOp() == AST::BinaryOp::Add && Types::isPointerType(*binary->getDataType()))
                return emitPointerAddition(*binary->getDataType(), binary->getExp1(), binary->getExp2());
            else if (binary->getOp() == AST::BinaryOp::Subtract && Types::isPointerType(*binary->getDataType()))
                return emitSubtractionFromPointer(*binary->getDataType(), binary->getExp1(), binary->getExp2());
            else if (binary->getOp() == AST::BinaryOp::Subtract && Types::isPointerType(*binary->getExp1()->getDataType()))
                // at least one operand is pointer, but result isn't, must be subtracting one pointer from another
                return emitPointerDiff(*binary->getDataType(), binary->getExp1(), binary->getExp2());

            return emitBinaryExp(binary);
        }
        }
    }
    case AST::NodeType::Assignment:
    {
        auto assignment = std::dynamic_pointer_cast<AST::Assignment>(exp);
        return emitAssignment(assignment->getLeftExp(), assignment->getRightExp());
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
    case AST::NodeType::Dereference:
    {
        return emitDereference(std::dynamic_pointer_cast<AST::Dereference>(exp));
    }
    case AST::NodeType::AddrOf:
    {
        return emitAddrOf(std::dynamic_pointer_cast<AST::AddrOf>(exp));
    }
    case AST::NodeType::Subscript:
    {
        return emitSubscript(std::dynamic_pointer_cast<AST::Subscript>(exp));
    }
    case AST::NodeType::SizeOfT:
    {
        return {
            std::vector<std::shared_ptr<TACKY::Instruction>>{},
            std::make_shared<ExpResult>(PlainOperand(evalSize(*std::dynamic_pointer_cast<AST::SizeOfT>(exp)->getTypeName()))),
        };
    }
    case AST::NodeType::SizeOf:
    {
        return {
            std::vector<std::shared_ptr<TACKY::Instruction>>{},
            std::make_shared<ExpResult>(PlainOperand(evalSize(*std::dynamic_pointer_cast<AST::SizeOf>(exp)->getInnerExp()->getDataType()))),
        };
    }
    case AST::NodeType::Dot:
    {
        return emitDotOperator(std::dynamic_pointer_cast<AST::Dot>(exp));
    }
    case AST::NodeType::Arrow:
    {
        return emitArrowOperator(std::dynamic_pointer_cast<AST::Arrow>(exp));
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

        auto [evalCond, c] = emitTackyAndConvert(ifStmt->getCondition());
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

        auto [evalCond, c] = emitTackyAndConvert(ifStmt->getCondition());
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
    auto [evalControl, c] = emitTackyAndConvert(switchStmt->getControl());
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
        auto returnStmt = std::dynamic_pointer_cast<AST::Return>(stmt);

        std::vector<std::shared_ptr<TACKY::Instruction>> insts{};
        std::optional<std::shared_ptr<TACKY::Val>> val;

        if (returnStmt->getOptValue().has_value())
        {
            auto [evalExp, v] = emitTackyAndConvert(returnStmt->getOptValue().value());
            insts = evalExp;
            val = std::make_optional(v);
        }
        else
        {
            insts = {};
            val = std::nullopt;
        }

        insts.push_back(std::make_shared<TACKY::Return>(val));
        return insts;
    }
    case AST::NodeType::ExpressionStmt:
    {
        auto [insts, _] = emitTackyForExp(std::dynamic_pointer_cast<AST::ExpressionStmt>(stmt)->getExp()); // Discard the evaluated v destination, we only care the side effect
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
TackyGen::emitStringInit(const std::string &s, const std::string &dst, int offset)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> insts;
    size_t len_s = s.size();
    size_t i = 0;

    while (i < len_s)
    {
        if (len_s - i >= 8)
        {
            // Pack 8 chars into a 64-bit integer (little-endian)
            uint64_t l = 0;
            for (int j = 0; j < 8; ++j)
            {
                l |= (static_cast<uint64_t>(static_cast<unsigned char>(s[i + j])) << (8 * j));
            }
            insts.push_back(
                std::make_shared<TACKY::CopyToOffset>(
                    std::make_shared<TACKY::Constant>(
                        std::make_shared<Constants::Const>(Constants::makeConstLong(static_cast<int64_t>(l)))),
                    dst,
                    offset + static_cast<int>(i)));
            i += 8;
        }
        else if (len_s - i >= 4)
        {
            // Pack 4 chars into a 32-bit integer (little-endian)
            uint32_t val = 0;
            for (int j = 0; j < 4; ++j)
            {
                val |= (static_cast<uint32_t>(static_cast<unsigned char>(s[i + j])) << (8 * j));
            }
            insts.push_back(
                std::make_shared<TACKY::CopyToOffset>(
                    std::make_shared<TACKY::Constant>(
                        std::make_shared<Constants::Const>(Constants::makeConstInt(static_cast<int32_t>(val)))),
                    dst,
                    offset + static_cast<int>(i)));
            i += 4;
        }
        else
        {
            // Single char
            int8_t c = static_cast<int8_t>(s[i]);
            insts.push_back(
                std::make_shared<TACKY::CopyToOffset>(
                    std::make_shared<TACKY::Constant>(
                        std::make_shared<Constants::Const>(Constants::makeConstChar(c))),
                    dst,
                    offset + static_cast<int>(i)));
            i += 1;
        }
    }

    return insts;
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitCompoundInit(const std::shared_ptr<AST::Initializer> &init, const std::string &name, ssize_t offset)
{
    if (auto singleInit = std::dynamic_pointer_cast<AST::SingleInit>(init))
    {
        if (singleInit->getExp()->getType() == AST::NodeType::String && Types::isArrayType(singleInit->getDataType().value()))
        {
            auto string = std::dynamic_pointer_cast<AST::String>(singleInit->getExp());
            auto arrayType = Types::getArrayType(singleInit->getDataType().value());

            auto str = string->getStr();
            auto arraySize = arrayType->size;
            auto strLen = str.size();
            auto paddingLen = arraySize - strLen;
            std::string paddedStr = str + std::string(paddingLen, '\0');
            return emitStringInit(paddedStr, name, offset);
        }

        auto [evalInit, v] = emitTackyAndConvert(singleInit->getExp());
        evalInit.push_back(std::make_shared<TACKY::CopyToOffset>(v, name, offset));
        return evalInit;
    }
    else if (auto compoundInit = std::dynamic_pointer_cast<AST::CompoundInit>(init))
    {
        auto newInits = std::vector<std::shared_ptr<TACKY::Instruction>>{};

        if (compoundInit->getDataType().has_value() && Types::isArrayType(compoundInit->getDataType().value()))
        {
            auto arrayType = Types::getArrayType(compoundInit->getDataType().value());
            for (size_t i = 0; i < compoundInit->getInits().size(); ++i)
            {
                auto elemInit = compoundInit->getInits()[i];
                auto newOffset = offset + (i * Types::getSize(*arrayType->elemType, _typeTable));
                auto innerInits = emitCompoundInit(elemInit, name, newOffset);
                newInits.insert(newInits.end(), innerInits.begin(), innerInits.end());
            }
            return newInits;
        }
        else if (compoundInit->getDataType().has_value() && Types::isStructType(compoundInit->getDataType().value()))
        {
            auto structType = Types::getStructType(compoundInit->getDataType().value());
            auto members = _typeTable.getMembers(structType->tag);
            for (size_t i = 0; i < compoundInit->getInits().size(); ++i)
            {
                auto init = compoundInit->getInits()[i];
                auto memb = members[i];
                auto memOffset = offset + memb.offset;
                auto innerInits = emitCompoundInit(init, name, memOffset);
                newInits.insert(newInits.end(), innerInits.begin(), innerInits.end());
            }
            return newInits;
        }
        else
        {
            throw std::runtime_error("Internal error: compound init has non-array type");
        }
    }
    else
    {
        throw std::invalid_argument("Internal error: Invalid initializer type");
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
    else // FunDecl or StructDecl
        return {};
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitVarDeclaration(const std::shared_ptr<AST::VariableDeclaration> &varDecl)
{
    if (varDecl->getOptInit().has_value())
    {
        if (auto singleInit = std::dynamic_pointer_cast<AST::SingleInit>(varDecl->getOptInit().value()))
        {
            if (singleInit->getExp()->getType() == AST::NodeType::String && Types::isArrayType(singleInit->getDataType().value()))
            {
                return emitCompoundInit(singleInit, varDecl->getName(), 0);
            }

            // Treat declaration with initializer as assignment
            auto lhs = std::make_shared<AST::Var>(varDecl->getName());
            lhs->setDataType(std::make_optional(varDecl->getVarType()));
            auto [evalAssignment, _] = emitAssignment(lhs, singleInit->getExp());

            return evalAssignment;
        }
        else if (auto compoundInit = std::dynamic_pointer_cast<AST::CompoundInit>(varDecl->getOptInit().value()))
        {
            return emitCompoundInit(compoundInit, varDecl->getName(), 0);
        }
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
    case AST::NodeType::StructDeclaration:
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

std::vector<std::shared_ptr<TACKY::TopLevel>>
TackyGen::convertSymbolsToTacky()
{
    std::vector<std::shared_ptr<TACKY::TopLevel>> staticVars{};

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
                        initial.value().staticInits));
            }
            else if (auto tentative = Symbols::getTentative(staticAttrs->init))
            {
                staticVars.push_back(
                    std::make_shared<TACKY::StaticVariable>(
                        name,
                        staticAttrs->global,
                        symbol.type,
                        Initializers::zero(symbol.type, _typeTable)));
            }
            else
            {
                // staticAttrs is a NoInitializer, do nothing
            }
        }
        else if (auto constAttr = Symbols::getConstAttr(symbol.attrs))
        {
            staticVars.push_back(
                std::make_shared<TACKY::StaticConstant>(
                    name,
                    symbol.type,
                    constAttr->init));
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

    auto tackyVarDefs = std::vector<std::shared_ptr<TACKY::TopLevel>>{convertSymbolsToTacky()};

    auto tackyDefs = std::vector<std::shared_ptr<TACKY::TopLevel>>{};
    std::copy(tackyVarDefs.begin(), tackyVarDefs.end(), std::back_inserter(tackyDefs));
    std::copy(tackyFnDefs.begin(), tackyFnDefs.end(), std::back_inserter(tackyDefs));

    return std::make_shared<TACKY::Program>(tackyDefs);
}
