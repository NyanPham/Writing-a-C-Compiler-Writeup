#include <unordered_set>

#include "TypeChecker.h"
#include "AST.h"
#include "Types.h"
#include "Symbols.h"
#include "Const.h"
#include "Initializers.h"
#include "ConstConvert.h"

/*
A helper function that makes implicit conversions explicit
If an expresion already has same type as the result_type, return it unchanged
Otherwise, wrap the expression in a Cast construct
*/
std::shared_ptr<AST::Expression> convertTo(const std::shared_ptr<AST::Expression> &exp, const Types::DataType &tgtType)
{
    if (exp->getDataType() == tgtType)
        return exp;

    auto castExp = std::make_shared<AST::Cast>(tgtType, exp);
    castExp->setDataType(std::make_optional(tgtType));

    return castExp;
}

bool isPointer(const Types::DataType &type)
{
    return Types::isPointerType(type);
}

bool isArithmetic(const Types::DataType &type)
{
    if (
        Types::isIntType(type) ||
        Types::isUIntType(type) ||
        Types::isLongType(type) ||
        Types::isULongType(type) ||
        Types::isDoubleType(type))
    {
        return true;
    }

    if (Types::isFunType(type) || Types::isPointerType(type))
    {
        return false;
    }

    throw std::runtime_error("Internal error: unknown type");
}

bool isInteger(const Types::DataType &type)
{
    if (
        Types::isIntType(type) ||
        Types::isUIntType(type) ||
        Types::isLongType(type) ||
        Types::isULongType(type))
    {
        return true;
    }

    if (Types::isFunType(type) || Types::isPointerType(type) || Types::isDoubleType(type))
    {
        return false;
    }

    return false;
}

bool isLvalue(const std::shared_ptr<AST::Expression> &exp)
{
    static const std::unordered_set<AST::NodeType> lvalueNodeTypes = {
        AST::NodeType::Var,
        AST::NodeType::Dereference,
    };

    return lvalueNodeTypes.count(exp->getType()) > 0;
}

bool isNullPointerConstant(const std::shared_ptr<AST::Expression> &exp)
{
    if (auto c = std::dynamic_pointer_cast<AST::Constant>(exp))
    {
        if (auto constInt = Constants::getConstInt(*c->getConst()))
            return constInt->val == 0;
        else if (auto constUInt = Constants::getConstUInt(*c->getConst()))
            return constUInt->val == 0;
        else if (auto constLong = Constants::getConstLong(*c->getConst()))
            return constLong->val == 0;
        else if (auto constULong = Constants::getConstULong(*c->getConst()))
            return constULong->val == 0;
        else
            return false;
    }
    else
    {
        return false;
    }
}

Types::DataType getCommonPointerType(const std::shared_ptr<AST::Expression> &e1, const std::shared_ptr<AST::Expression> &e2)
{
    if (!e1->getDataType().has_value() || !e2->getDataType().has_value())
    {
        throw std::runtime_error("Internal error: expressions must be typechecked before getting the common pointer type");
    }

    if (e1->getDataType() == e2->getDataType())
        return e1->getDataType().value();
    else if (isNullPointerConstant(e1))
        return e2->getDataType().value();
    else if (isNullPointerConstant(e2))
        return e1->getDataType().value();
    else
        throw std::runtime_error("Expressions have incompatible types");
}

std::shared_ptr<AST::Expression> convertByAssignment(const std::shared_ptr<AST::Expression> &exp, const Types::DataType &tgtType)
{
    if (!exp->getDataType().has_value())
        throw std::runtime_error("Internal error: expression is not typechecked");

    if (exp->getDataType().value() == tgtType)
        return exp;
    else if (isArithmetic(exp->getDataType().value()) && isArithmetic(tgtType))
        return convertTo(exp, tgtType);
    else if (isNullPointerConstant(exp) && isPointer(tgtType))
        return convertTo(exp, tgtType);
    else
        throw std::runtime_error("Cannot convert type for assignment");
}

Types::DataType getCommonType(const Types::DataType &t1, const Types::DataType &t2)
{
    if (t1 == t2)
        return t1;

    if (Types::isDoubleType(t1) || Types::isDoubleType(t2))
        return Types::makeDoubleType();

    else if (Types::getSize(t1) == Types::getSize(t2))
    {
        if (Types::isSigned(t1))
            return t2;
        else
            return t1;
    }
    else if (Types::getSize(t1) > Types::getSize(t2))
        return t1;
    else
        return t2;
}

/*
Convert a constant to static initializer, performing type converion if needed.
*/
Symbols::InitialValue toStaticInit(const Types::DataType &varType, const std::shared_ptr<AST::Expression> &e)
{
    if (auto astConstant = std::dynamic_pointer_cast<AST::Constant>(e))
    {
        Initializers::StaticInit initVal;

        if (isPointer(varType))
        {
            if (isNullPointerConstant(astConstant))
                initVal = Initializers::ULongInit(0);
            else
                throw std::runtime_error("Static pointers can only be initialized with null pointer constants");
        }
        else
        {
            auto convertedConstant = ConstConvert::convert(varType, astConstant->getConst());

            if (auto constInt = Constants::getConstInt(*convertedConstant))
                initVal = Initializers::IntInit{constInt->val};
            else if (auto constLong = Constants::getConstLong(*convertedConstant))
                initVal = Initializers::LongInit{constLong->val};
            else if (auto constUInt = Constants::getConstUInt(*convertedConstant))
                initVal = Initializers::UIntInit{constUInt->val};
            else if (auto constULong = Constants::getConstULong(*convertedConstant))
                initVal = Initializers::ULongInit{constULong->val};
            else if (auto constDouble = Constants::getConstDouble(*convertedConstant))
                initVal = Initializers::DoubleInit{constDouble->val};
            else
                throw std::runtime_error("Internal error: invalid constant type");
        }

        return Symbols::makeInitial(initVal);
    }
    else
        throw std::runtime_error("Internal error: invalid constant type");
}

std::shared_ptr<AST::Cast>
TypeChecker::typeCheckCast(const std::shared_ptr<AST::Cast> &cast)
{
    auto targetType = cast->getTargetType();
    auto typedInner = typeCheckExp(cast->getExp());

    if (
        (Types::isPointerType(targetType) && Types::isDoubleType(typedInner->getDataType().value())) ||
        (Types::isDoubleType(targetType) && Types::isPointerType(typedInner->getDataType().value())))
        throw std::runtime_error("Cannot cast between pointer and double");

    auto castExp = std::make_shared<AST::Cast>(targetType, typedInner);
    castExp->setDataType(std::make_optional(targetType));
    return castExp;
}

std::shared_ptr<AST::Unary>
TypeChecker::typeCheckNot(const std::shared_ptr<AST::Unary> &notUnary)
{
    if (notUnary->getOp() != AST::UnaryOp::Not)
        throw std::runtime_error("Internal error: typeCheckNot called with non-Not operator");

    auto typedInner = typeCheckExp(notUnary->getExp());
    auto notExp = std::make_shared<AST::Unary>(AST::UnaryOp::Not, typedInner);
    notExp->setDataType(std::make_optional(Types::makeIntType()));
    return notExp;
}

std::shared_ptr<AST::Unary>
TypeChecker::typeCheckComplement(const std::shared_ptr<AST::Unary> &complUnary)
{
    if (complUnary->getOp() != AST::UnaryOp::Complement)
        throw std::runtime_error("Internal error: typeCheckComplement called with non-Complement operator");

    auto typedInner = typeCheckExp(complUnary->getExp());
    if (Types::isDoubleType(typedInner->getDataType().value()) || isPointer(typedInner->getDataType().value()))
        throw std::runtime_error("Bitwise complement only valid for integer types");

    auto complExp = std::make_shared<AST::Unary>(AST::UnaryOp::Complement, typedInner);
    complExp->setDataType(typedInner->getDataType());
    return complExp;
}

std::shared_ptr<AST::Unary>
TypeChecker::typeCheckNegate(const std::shared_ptr<AST::Unary> &negUnary)
{
    if (negUnary->getOp() != AST::UnaryOp::Negate)
        throw std::runtime_error("Internal error: typeCheckNegate called with non-Negate operator");

    auto typedInner = typeCheckExp(negUnary->getExp());
    if (isPointer(typedInner->getDataType().value()))
        throw std::runtime_error("Cannot negate a pointer");

    auto negExp = std::make_shared<AST::Unary>(AST::UnaryOp::Negate, typedInner);
    negExp->setDataType(typedInner->getDataType());
    return negExp;
}

std::shared_ptr<AST::Unary>
TypeChecker::typeCheckIncrDecr(const std::shared_ptr<AST::Unary> &incrDecrUnary)
{
    if (isLvalue(incrDecrUnary->getExp()))
    {
        auto typedInner = typeCheckExp(incrDecrUnary->getExp());
        auto typedExp = std::make_shared<AST::Unary>(incrDecrUnary->getOp(), typedInner);
        typedExp->setDataType(typedInner->getDataType());
        return typedExp;
    }
    else
    {
        throw std::runtime_error("Operand ++/-- must be an lvalue");
    }
    return nullptr;
}

std::shared_ptr<AST::Binary>
TypeChecker::typeCheckLogical(const std::shared_ptr<AST::Binary> &logicalBinary)
{
    auto typedE1 = typeCheckExp(logicalBinary->getExp1());
    auto typedE2 = typeCheckExp(logicalBinary->getExp2());
    auto typedBinExp = std::make_shared<AST::Binary>(logicalBinary->getOp(), typedE1, typedE2);
    typedBinExp->setDataType(std::make_optional(Types::makeIntType()));
    return typedBinExp;
}

std::shared_ptr<AST::Binary>
TypeChecker::typeCheckArithmetic(const std::shared_ptr<AST::Binary> &arithBinary)
{
    auto typedE1 = typeCheckExp(arithBinary->getExp1());
    auto typedE2 = typeCheckExp(arithBinary->getExp2());

    if (isPointer(typedE1->getDataType().value()) || isPointer(typedE2->getDataType().value()))
    {
        throw std::runtime_error("Arithmetic operations not permitted on pointers");
    }
    else
    {
        auto commonType = getCommonType(typedE1->getDataType().value(), typedE2->getDataType().value());
        auto convertedE1 = convertTo(typedE1, commonType);
        auto convertedE2 = convertTo(typedE2, commonType);
        auto binaryExp = std::make_shared<AST::Binary>(arithBinary->getOp(), convertedE1, convertedE2);

        static const std::unordered_set<AST::BinaryOp> opNotAllowedForDouble = {
            AST::BinaryOp::Remainder,
            AST::BinaryOp::BitwiseAnd,
            AST::BinaryOp::BitwiseOr,
            AST::BinaryOp::BitwiseXor};

        static const std::unordered_set<AST::BinaryOp> opForCommonType = {
            AST::BinaryOp::Add,
            AST::BinaryOp::Subtract,
            AST::BinaryOp::Multiply,
            AST::BinaryOp::Divide,
            AST::BinaryOp::Remainder,
            AST::BinaryOp::BitwiseAnd,
            AST::BinaryOp::BitwiseOr,
            AST::BinaryOp::BitwiseXor,
        };

        if (Types::isDoubleType(commonType) && opNotAllowedForDouble.count(arithBinary->getOp()))
        {
            throw std::runtime_error("Can't apply 5, &, |, ^ to double");
        }

        if (opForCommonType.count(arithBinary->getOp()))
        {
            binaryExp->setDataType(std::make_optional(commonType));
            return binaryExp;
        }

        throw std::runtime_error("Internal error: operator in arithmetic binary should be typechecked elsewhere");
    }

    return nullptr;
}

std::shared_ptr<AST::Binary>
TypeChecker::typeCheckComparison(const std::shared_ptr<AST::Binary> &comparison)
{
    auto typedE1 = typeCheckExp(comparison->getExp1());
    auto typedE2 = typeCheckExp(comparison->getExp2());

    auto commonType = (isPointer(typedE1->getDataType().value()) || isPointer(typedE2->getDataType().value()))
                          ? getCommonPointerType(typedE1, typedE2)
                          : getCommonType(typedE1->getDataType().value(), typedE2->getDataType().value());

    auto convertedE1 = convertTo(typedE1, commonType);
    auto convertedE2 = convertTo(typedE2, commonType);
    auto binaryExp = std::make_shared<AST::Binary>(comparison->getOp(), convertedE1, convertedE2);
    binaryExp->setDataType(std::make_optional(Types::makeIntType()));
    return binaryExp;
}

std::shared_ptr<AST::Binary>
TypeChecker::typeCheckBitShift(const std::shared_ptr<AST::Binary> &bitShiftBinary)
{
    auto typedE1 = typeCheckExp(bitShiftBinary->getExp1());
    auto typedE2 = typeCheckExp(bitShiftBinary->getExp2());

    if (!(isInteger(typedE1->getDataType().value()) && isInteger(typedE2->getDataType().value())))
    {
        throw std::runtime_error("Both operands of bitshift operation must be integers");
    }
    else
    {
        // Don't perform usual arithmetic conversions; result has type of left operand
        auto typedBinExp = std::make_shared<AST::Binary>(bitShiftBinary->getOp(), typedE1, typedE2);
        typedBinExp->setDataType(typedE1->getDataType());
        return typedBinExp;
    }
}

std::shared_ptr<AST::Dereference>
TypeChecker::typeCheckDereference(const std::shared_ptr<AST::Dereference> &dereference)
{
    auto typedInner = typeCheckExp(dereference->getInnerExp());

    if (auto pointerType = Types::getPointerType(typedInner->getDataType().value()))
    {
        auto derefExp = std::make_shared<AST::Dereference>(typedInner);
        derefExp->setDataType(std::make_optional(*pointerType->referencedType));
        return derefExp;
    }
    else
    {
        throw std::runtime_error("Tried to dereference non-pointer");
    }
}

std::shared_ptr<AST::AddrOf>
TypeChecker::typeCheckAddrOf(const std::shared_ptr<AST::AddrOf> &addrOf)
{
    if (addrOf->getInnerExp()->getType() == AST::NodeType::Dereference || addrOf->getInnerExp()->getType() == AST::NodeType::Var)
    {
        auto typedInner = typeCheckExp(addrOf->getInnerExp());
        auto innerType = std::make_shared<Types::DataType>(typedInner->getDataType().value());
        auto addrExp = std::make_shared<AST::AddrOf>(typedInner);
        addrExp->setDataType(std::make_optional(Types::makePointerType(innerType)));
        return addrExp;
    }
    else
    {
        throw std::runtime_error("Cannot take address of non-lvalue");
    }
}

std::shared_ptr<AST::Var>
TypeChecker::typeCheckVar(const std::shared_ptr<AST::Var> &var)
{
    auto vType = _symbolTable.get(var->getName()).type;
    auto e = std::make_shared<AST::Var>(var->getName());

    if (Types::isFunType(vType))
        throw std::runtime_error("Tried to use function name as variable");

    e->setDataType(std::make_optional(vType));
    return e;
}

std::shared_ptr<AST::Constant>
TypeChecker::typeCheckConstant(const std::shared_ptr<AST::Constant> &c)
{
    auto e = std::make_shared<AST::Constant>(c->getConst());

    e->setDataType(std::make_optional(Constants::typeOfConst(*c->getConst())));
    return e;
}

std::shared_ptr<AST::Assignment>
TypeChecker::typeCheckAssignment(const std::shared_ptr<AST::Assignment> &assignment)
{
    if (isLvalue(assignment->getLeftExp()))
    {
        auto typedLhs = typeCheckExp(assignment->getLeftExp());
        auto lhsType = typedLhs->getDataType().value();
        auto typedRhs = typeCheckExp(assignment->getRightExp());
        auto convertedRhs = convertByAssignment(typedRhs, lhsType);
        auto assignExp = std::make_shared<AST::Assignment>(typedLhs, convertedRhs);
        assignExp->setDataType(std::make_optional(lhsType));
        return assignExp;
    }
    else
    {
        throw std::runtime_error("Left hand side of assignment is invalid lvalue");
    }
}

std::shared_ptr<AST::CompoundAssignment>
TypeChecker::typeCheckCompoundAssignment(const std::shared_ptr<AST::CompoundAssignment> &compoundAssign)
{
    if (isLvalue(compoundAssign->getLeftExp()))
    {
        auto typedLhs = typeCheckExp(compoundAssign->getLeftExp());
        auto typedRhs = typeCheckExp(compoundAssign->getRightExp());

        if (!typedLhs->getDataType().has_value() || !typedRhs->getDataType().has_value())
            throw std::runtime_error("Compound assignment operands have no data type");

        auto lhsType = typedLhs->getDataType().value();
        auto rhsType = typedRhs->getDataType().value();

        if (
            (compoundAssign->getOp() == AST::BinaryOp::Remainder ||
             compoundAssign->getOp() == AST::BinaryOp::BitwiseAnd ||
             compoundAssign->getOp() == AST::BinaryOp::BitwiseOr ||
             compoundAssign->getOp() == AST::BinaryOp::BitwiseXor ||
             compoundAssign->getOp() == AST::BinaryOp::BitShiftLeft ||
             compoundAssign->getOp() == AST::BinaryOp::BitShiftRight) &&
            (!isInteger(*typedLhs->getDataType()) || !isInteger(*typedRhs->getDataType())))
        {
            throw std::runtime_error("Operand of compound assignment only supports integer operands");
        }

        if (
            (compoundAssign->getOp() == AST::BinaryOp::Multiply || compoundAssign->getOp() == AST::BinaryOp::Divide) &&
            (isPointer(lhsType) || isPointer(rhsType)))
        {
            throw std::runtime_error("Operand of compound assignment does not support pointer operands");
        }

        Types::DataType resultType;
        std::shared_ptr<AST::Expression> convertedRhs;

        if (compoundAssign->getOp() == AST::BinaryOp::BitShiftLeft || compoundAssign->getOp() == AST::BinaryOp::BitShiftRight)
        {
            resultType = lhsType;
            convertedRhs = typedRhs;
        }
        else
        {
            // We perform usual arithmetic conversions for every compound assignment operator
            // EXCEPT Left/Right bitshift
            auto commonType = getCommonType(lhsType, rhsType);
            resultType = commonType;
            convertedRhs = convertTo(typedRhs, commonType);
        }

        // IMPORTANT: this may involve several implicit casts:
        // from RHS type to common type (represented w/ explicit convert_to)
        // from LHS type to common type (NOT directly represented in AST)
        // from common_type back to LHS type on assignment (NOT directly represented in AST)
        // We cannot add Cast expressions for the last two because LHS should be evaluated only once,
        // so we don't have two separate places to put Cast expression in this AST node. But we have
        // enough type information to allow us to insert these casts during TACKY generation
        auto typedCompoundAssign = std::make_shared<AST::CompoundAssignment>(compoundAssign->getOp(), typedLhs, convertedRhs, std::make_optional(resultType));
        typedCompoundAssign->setDataType(std::make_optional(lhsType));
        return typedCompoundAssign;
    }
    else
    {
        throw std::runtime_error("Left-hand side of compound assignment must be an lvalue");
    }
}

std::shared_ptr<AST::PostfixDecr>
TypeChecker::typeCheckPostfixDecr(const std::shared_ptr<AST::PostfixDecr> &postfixDecr)
{
    if (isLvalue(postfixDecr->getExp()))
    {
        // Result has same value as e; no conversions required.
        // We need to convert integer "1" to their common type, but that will always be the same type as e, at least w/ types we've added so far

        auto typedExp = typeCheckExp(postfixDecr->getExp());
        auto resultType = typedExp->getDataType().value();
        auto typedPostfixDecr = std::make_shared<AST::PostfixDecr>(typedExp);
        typedPostfixDecr->setDataType(std::make_optional(resultType));
        return typedPostfixDecr;
    }
    else
    {
        throw std::runtime_error("Operand of postfix -- must be an lvalue");
    }
}

std::shared_ptr<AST::PostfixIncr>
TypeChecker::typeCheckPostfixIncr(const std::shared_ptr<AST::PostfixIncr> &postfixIncr)
{
    if (isLvalue(postfixIncr->getExp()))
    {
        // Same deal as postfix decrement

        auto typedExp = typeCheckExp(postfixIncr->getExp());
        auto resultType = typedExp->getDataType().value();
        auto typedPostfixIncr = std::make_shared<AST::PostfixIncr>(typedExp);
        typedPostfixIncr->setDataType(std::make_optional(resultType));
        return typedPostfixIncr;
    }
    else
    {
        throw std::runtime_error("Operand of postfix ++ must be an lvalue");
    }
}

std::shared_ptr<AST::Conditional>
TypeChecker::typeCheckConditional(const std::shared_ptr<AST::Conditional> &conditional)
{
    auto typedCondition = typeCheckExp(conditional->getCondition());
    auto typedThen = typeCheckExp(conditional->getThen());
    auto typedElse = typeCheckExp(conditional->getElse());

    if (!typedThen->getDataType().has_value() || !typedElse->getDataType().has_value())
        throw std::runtime_error("Conditional expression branches have no data type");

    auto commonType = (isPointer(typedThen->getDataType().value()) || isPointer(typedElse->getDataType().value()))
                          ? getCommonPointerType(typedThen, typedElse)
                          : getCommonType(typedThen->getDataType().value(), typedElse->getDataType().value());

    auto convertedThen = convertTo(typedThen, commonType);
    auto convertedElse = convertTo(typedElse, commonType);

    auto typedConditional = std::make_shared<AST::Conditional>(typedCondition, convertedThen, convertedElse);
    typedConditional->setDataType(std::make_optional(commonType));
    return typedConditional;
}

std::shared_ptr<AST::FunctionCall>
TypeChecker::typeCheckFunctionCall(const std::shared_ptr<AST::FunctionCall> &funCall)
{
    auto sType = _symbolTable.get(funCall->getName()).type;

    if (auto fType = Types::getFunType(sType))
    {
        if (fType->paramTypes.size() != funCall->getArgs().size())
            throw std::runtime_error("Function called with wrong number of arguments: " + funCall->getName());

        auto convertedArgs = std::vector<std::shared_ptr<AST::Expression>>();
        for (size_t i{0}; i < fType->paramTypes.size(); i++)
        {
            auto paramType = fType->paramTypes[i];
            auto arg = funCall->getArgs()[i];
            auto newArg = convertByAssignment(typeCheckExp(arg), *paramType);
            convertedArgs.push_back(newArg);
        }

        auto typedCall = std::make_shared<AST::FunctionCall>(funCall->getName(), convertedArgs);
        typedCall->setDataType(std::make_optional(*fType->retType));
        return typedCall;
    }

    throw std::runtime_error("Tried to use variable as function name");
}

std::shared_ptr<AST::Expression>
TypeChecker::typeCheckExp(const std::shared_ptr<AST::Expression> &exp)
{
    switch (exp->getType())
    {
    case AST::NodeType::Var:
    {
        return typeCheckVar(std::dynamic_pointer_cast<AST::Var>(exp));
    }
    case AST::NodeType::Constant:
    {
        return typeCheckConstant(std::dynamic_pointer_cast<AST::Constant>(exp));
    }
    case AST::NodeType::Cast:
    {
        return typeCheckCast(std::dynamic_pointer_cast<AST::Cast>(exp));
    }
    case AST::NodeType::Unary:
    {
        auto unary = std::dynamic_pointer_cast<AST::Unary>(exp);
        switch (unary->getOp())
        {
        case AST::UnaryOp::Not:
            return typeCheckNot(unary);
        case AST::UnaryOp::Complement:
            return typeCheckComplement(unary);
        case AST::UnaryOp::Negate:
            return typeCheckNegate(unary);
        default:
            return typeCheckIncrDecr(unary);
        }
    }
    case AST::NodeType::Binary:
    {
        auto binary = std::dynamic_pointer_cast<AST::Binary>(exp);
        switch (binary->getOp())
        {
        case AST::BinaryOp::And:
        case AST::BinaryOp::Or:
            return typeCheckLogical(binary);
        case AST::BinaryOp::Add:
        case AST::BinaryOp::Subtract:
        case AST::BinaryOp::Multiply:
        case AST::BinaryOp::Divide:
        case AST::BinaryOp::Remainder:
        case AST::BinaryOp::BitwiseAnd:
        case AST::BinaryOp::BitwiseOr:
        case AST::BinaryOp::BitwiseXor:
            return typeCheckArithmetic(binary);
        case AST::BinaryOp::Equal:
        case AST::BinaryOp::NotEqual:
        case AST::BinaryOp::GreaterThan:
        case AST::BinaryOp::GreaterOrEqual:
        case AST::BinaryOp::LessThan:
        case AST::BinaryOp::LessOrEqual:
            return typeCheckComparison(binary);
        case AST::BinaryOp::BitShiftLeft:
        case AST::BinaryOp::BitShiftRight:
            return typeCheckBitShift(binary);
        default:
            throw std::runtime_error("Internal Error: Unknown binary operator!");
        }
    }
    case AST::NodeType::Assignment:
    {
        return typeCheckAssignment(std::dynamic_pointer_cast<AST::Assignment>(exp));
    }
    case AST::NodeType::CompoundAssignment:
    {
        return typeCheckCompoundAssignment(std::dynamic_pointer_cast<AST::CompoundAssignment>(exp));
    }
    case AST::NodeType::PostfixDecr:
    {
        return typeCheckPostfixDecr(std::dynamic_pointer_cast<AST::PostfixDecr>(exp));
    }
    case AST::NodeType::PostfixIncr:
    {
        return typeCheckPostfixIncr(std::dynamic_pointer_cast<AST::PostfixIncr>(exp));
    }
    case AST::NodeType::Conditional:
    {
        return typeCheckConditional(std::dynamic_pointer_cast<AST::Conditional>(exp));
    }
    case AST::NodeType::FunctionCall:
    {
        return typeCheckFunctionCall(std::dynamic_pointer_cast<AST::FunctionCall>(exp));
    }
    case AST::NodeType::Dereference:
    {
        return typeCheckDereference(std::dynamic_pointer_cast<AST::Dereference>(exp));
    }
    case AST::NodeType::AddrOf:
    {
        return typeCheckAddrOf(std::dynamic_pointer_cast<AST::AddrOf>(exp));
    }
    default:
        throw std::runtime_error("Internal Error: Unknown type of expression!");
    }
}

AST::Block
TypeChecker::typeCheckBlock(const AST::Block &blk, const Types::DataType &retType)
{
    AST::Block newBlock;
    newBlock.reserve(blk.size());

    for (const auto &blkItm : blk)
        newBlock.push_back(typeCheckBlockItem(blkItm, retType));

    return newBlock;
}

std::shared_ptr<AST::BlockItem>
TypeChecker::typeCheckBlockItem(const std::shared_ptr<AST::BlockItem> &blkItem, const Types::DataType &retType)
{
    switch (blkItem->getType())
    {
    case AST::NodeType::VariableDeclaration:
    case AST::NodeType::FunctionDeclaration:
    {
        return typeCheckLocalDecl(std::dynamic_pointer_cast<AST::Declaration>(blkItem));
    }
    default:
    {
        return typeCheckStatement(std::dynamic_pointer_cast<AST::Statement>(blkItem), retType);
    }
    }
}

std::shared_ptr<AST::Statement>
TypeChecker::typeCheckStatement(const std::shared_ptr<AST::Statement> &stmt, const Types::DataType &retType)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Return:
    {
        auto typedExp = typeCheckExp(std::dynamic_pointer_cast<AST::Return>(stmt)->getValue());
        return std::make_shared<AST::Return>(convertByAssignment(typedExp, retType));
    }
    case AST::NodeType::ExpressionStmt:
    {
        auto newExp = typeCheckExp(std::dynamic_pointer_cast<AST::ExpressionStmt>(stmt)->getExp());
        return std::make_shared<AST::ExpressionStmt>(newExp);
    }
    case AST::NodeType::If:
    {
        auto ifStmt = std::dynamic_pointer_cast<AST::If>(stmt);
        auto newCond = typeCheckExp(ifStmt->getCondition());
        auto newThenCls = typeCheckStatement(ifStmt->getThenClause(), retType);
        auto newOptElseCls = std::optional<std::shared_ptr<AST::Statement>>{std::nullopt};
        if (ifStmt->getOptElseClause().has_value())
        {
            newOptElseCls = typeCheckStatement(ifStmt->getOptElseClause().value(), retType);
        }

        return std::make_shared<AST::If>(newCond, newThenCls, newOptElseCls);
    }
    case AST::NodeType::LabeledStatement:
    {
        auto labeledStmt = std::dynamic_pointer_cast<AST::LabeledStatement>(stmt);
        auto newStmt = typeCheckStatement(labeledStmt->getStatement(), retType);

        return std::make_shared<AST::LabeledStatement>(labeledStmt->getLabel(), newStmt);
    }
    case AST::NodeType::Case:
    {
        auto caseStmt = std::dynamic_pointer_cast<AST::Case>(stmt);

        // Note: e must be converted to type of controlling expression in enclosing switch;
        // We do that during CollectSwitchCases pass
        auto typedExp = typeCheckExp(caseStmt->getValue());
        auto newBody = typeCheckStatement(caseStmt->getBody(), retType);

        if (Types::isDoubleType(*typedExp->getDataType()))
            throw std::runtime_error("Case expression cannot be double");

        return std::make_shared<AST::Case>(typedExp, newBody, caseStmt->getId());
    }
    case AST::NodeType::Default:
    {
        auto defaultStmt = std::dynamic_pointer_cast<AST::Default>(stmt);
        auto typedBody = typeCheckStatement(defaultStmt->getBody(), retType);

        return std::make_shared<AST::Default>(typedBody, defaultStmt->getId());
    }
    case AST::NodeType::Switch:
    {
        auto switchStmt = std::dynamic_pointer_cast<AST::Switch>(stmt);
        auto typedControl = typeCheckExp(switchStmt->getControl());
        auto typedBody = typeCheckStatement(switchStmt->getBody(), retType);

        if (!isInteger(typedControl->getDataType().value()))
            throw std::runtime_error("Controlling expression in switch must have integer type");

        return std::make_shared<AST::Switch>(
            typedControl,
            typedBody,
            switchStmt->getOptCases(),
            switchStmt->getId());
    }
    case AST::NodeType::Compound:
    {
        auto newBlock = typeCheckBlock(std::dynamic_pointer_cast<AST::Compound>(stmt)->getBlock(), retType);

        return std::make_shared<AST::Compound>(newBlock);
    }
    case AST::NodeType::While:
    {
        auto whileStmt = std::dynamic_pointer_cast<AST::While>(stmt);
        auto newCond = typeCheckExp(whileStmt->getCondition());
        auto newBody = typeCheckStatement(whileStmt->getBody(), retType);

        return std::make_shared<AST::While>(newCond, newBody, whileStmt->getId());
    }
    case AST::NodeType::DoWhile:
    {
        auto doWhileStmt = std::dynamic_pointer_cast<AST::DoWhile>(stmt);
        auto newBody = typeCheckStatement(doWhileStmt->getBody(), retType);
        auto newCond = typeCheckExp(doWhileStmt->getCondition());

        return std::make_shared<AST::DoWhile>(newBody, newCond, doWhileStmt->getId());
    }
    case AST::NodeType::For:
    {
        auto forStmt = std::dynamic_pointer_cast<AST::For>(stmt);

        std::shared_ptr<AST::ForInit> newInit = std::make_shared<AST::InitExp>(std::nullopt);
        if (auto initDecl = std::dynamic_pointer_cast<AST::InitDecl>(forStmt->getInit()))
        {
            if (initDecl->getDecl()->getOptStorageClass().has_value())
                throw std::runtime_error("Storage class not permitted on declaration in for loop headers");
            else
                newInit = std::make_shared<AST::InitDecl>(typeCheckLocalVarDecl(initDecl->getDecl()));
        }
        else if (auto initExp = std::dynamic_pointer_cast<AST::InitExp>(forStmt->getInit()))
        {
            if (initExp->getOptExp().has_value())
                newInit = std::make_shared<AST::InitExp>(typeCheckExp(initExp->getOptExp().value()));
        }

        auto newCond = std::optional<std::shared_ptr<AST::Expression>>{std::nullopt};
        if (forStmt->getOptCondition().has_value())
        {
            newCond = std::make_optional(typeCheckExp(forStmt->getOptCondition().value()));
        }

        auto newPost = std::optional<std::shared_ptr<AST::Expression>>{std::nullopt};
        if (forStmt->getOptPost().has_value())
        {
            newPost = std::make_optional(typeCheckExp(forStmt->getOptPost().value()));
        }

        auto newBody = typeCheckStatement(forStmt->getBody(), retType);

        return std::make_shared<AST::For>(
            newInit,
            newCond,
            newPost,
            newBody,
            forStmt->getId());
    }
    case AST::NodeType::Null:
    case AST::NodeType::Break:
    case AST::NodeType::Continue:
    case AST::NodeType::Goto:
        return stmt;
    default:
        throw std::runtime_error("Internal Error: Unknown type of statement!");
    }
}

std::shared_ptr<AST::VariableDeclaration>
TypeChecker::typeCheckLocalVarDecl(const std::shared_ptr<AST::VariableDeclaration> &varDecl)
{
    if (varDecl->getOptStorageClass().has_value())
    {
        switch (varDecl->getOptStorageClass().value())
        {
        case AST::StorageClass::Extern:
        {
            if (varDecl->getOptInit().has_value())
                throw std::runtime_error("Initializer on local extern declaration");
            else
            {
                auto optSymbol = _symbolTable.getOpt(varDecl->getName());
                if (optSymbol.has_value())
                {
                    // If an external local variable is already in the symbol table,
                    // we check if it's a variable, and don't need to add it
                    auto symbol = optSymbol.value();
                    if (symbol.type != varDecl->getVarType())
                        throw std::runtime_error("Variable redeclared with different type");
                }
                else
                    _symbolTable.addStaticVar(varDecl->getName(), varDecl->getVarType(), Symbols::makeNoInitializer(), true);
            }
            return std::make_shared<AST::VariableDeclaration>(varDecl->getName(), std::nullopt, varDecl->getVarType(), varDecl->getOptStorageClass());
        }
        case AST::StorageClass::Static:
        {
            auto zeroInit = Symbols::Initial(Initializers::zero(varDecl->getVarType()));

            Symbols::InitialValue staticInit =
                varDecl->getOptInit().has_value()
                    ? toStaticInit(varDecl->getVarType(), varDecl->getOptInit().value())
                    : zeroInit;

            _symbolTable.addStaticVar(varDecl->getName(), varDecl->getVarType(), staticInit, false);

            // Note, we won't actually use init in subsequen passes, so we can drop it
            return std::make_shared<AST::VariableDeclaration>(varDecl->getName(), std::nullopt, varDecl->getVarType(), varDecl->getOptStorageClass());
        }
        default:
            throw std::runtime_error("Internal error: Unknown storage class");
        }
    }
    else
    {
        _symbolTable.addAutomaticVar(varDecl->getName(), varDecl->getVarType());
        if (varDecl->getOptInit().has_value())
        {
            auto newInit = convertByAssignment(typeCheckExp(varDecl->getOptInit().value()), varDecl->getVarType());
            return std::make_shared<AST::VariableDeclaration>(varDecl->getName(), newInit, varDecl->getVarType(), std::nullopt);
        }

        return varDecl;
    }
}

std::shared_ptr<AST::Declaration>
TypeChecker::typeCheckLocalDecl(const std::shared_ptr<AST::Declaration> &decl)
{
    if (auto varDecl = std::dynamic_pointer_cast<AST::VariableDeclaration>(decl))
        return typeCheckLocalVarDecl(varDecl);
    else if (auto funDecl = std::dynamic_pointer_cast<AST::FunctionDeclaration>(decl))
        return typeCheckFunDecl(funDecl);
    else
        throw std::runtime_error("Internal Error: Unknown type of declaration!");
}

std::shared_ptr<AST::VariableDeclaration>
TypeChecker::typeCheckFileScopeVarDecl(const std::shared_ptr<AST::VariableDeclaration> &varDecl)
{
    Symbols::InitialValue defaultInit =
        varDecl->getOptStorageClass().has_value() && varDecl->getOptStorageClass().value() == AST::StorageClass::Extern
            ? Symbols::makeNoInitializer()
            : Symbols::makeTentative();

    Symbols::InitialValue staticInit =
        varDecl->getOptInit().has_value()
            ? toStaticInit(varDecl->getVarType(), varDecl->getOptInit().value())
            : defaultInit;

    bool currGlobal = !varDecl->getOptStorageClass().has_value() || (varDecl->getOptStorageClass().has_value() && varDecl->getOptStorageClass().value() != AST::StorageClass::Static);

    if (_symbolTable.exists(varDecl->getName()))
    {
        auto oldDecl = _symbolTable.get(varDecl->getName());
        if (oldDecl.type != varDecl->getVarType())
            throw std::runtime_error("Variable redeclared with differnt type: " + varDecl->getName());

        if (auto staticAttrs = getStaticAttr(oldDecl.attrs))
        {
            if (varDecl->getOptStorageClass().has_value() && varDecl->getOptStorageClass().value() == AST::StorageClass::Extern)
                currGlobal = staticAttrs->global;
            else if (staticAttrs->global != currGlobal)
                throw std::runtime_error("Conflicting variable linkage: " + varDecl->getName());

            if (Symbols::isInitial(staticAttrs->init))
            {
                if (Symbols::isInitial(staticInit))
                    throw std::runtime_error("Conflicting global variable definition" + varDecl->getName());
                else
                    staticInit = staticAttrs->init;
            }
            else if (Symbols::isTentative(staticAttrs->init) && !Symbols::isInitial(staticInit))
                staticInit = Symbols::makeTentative();
        }
        else
            throw std::runtime_error("Internal error: File scope variable previously declared as local variable: " + varDecl->getName());
    }

    _symbolTable.addStaticVar(varDecl->getName(), varDecl->getVarType(), staticInit, currGlobal);

    // it's ok to drop the initializer as it's never used after this pass
    return std::make_shared<AST::VariableDeclaration>(varDecl->getName(), std::nullopt, varDecl->getVarType(), varDecl->getOptStorageClass());
}

std::shared_ptr<AST::FunctionDeclaration>
TypeChecker::typeCheckFunDecl(const std::shared_ptr<AST::FunctionDeclaration> &funDecl)
{
    bool hasBody = funDecl->getOptBody().has_value();
    bool alreadyDefined = hasBody;
    bool global = !funDecl->getOptStorageClass().has_value() || (funDecl->getOptStorageClass().has_value() && funDecl->getOptStorageClass().value() != AST::StorageClass::Static);

    if (_symbolTable.exists(funDecl->getName()))
    {
        auto oldDecl = _symbolTable.get(funDecl->getName());

        if (funDecl->getFunType() != Types::getFunType(oldDecl.type).value())
            throw std::runtime_error("Incompatible function declaration!");

        if (auto funAttrs = getFunAttr(oldDecl.attrs))
        {
            if (funAttrs->defined && hasBody)
                throw std::runtime_error("Function is defined more than once: " + funDecl->getName());
            else if (funAttrs->global && funDecl->getOptStorageClass().has_value() && funDecl->getOptStorageClass().value() == AST::StorageClass::Static)
                throw std::runtime_error("Static function declaration follows non-static: " + funDecl->getName());
            else
            {
                alreadyDefined = hasBody || funAttrs->defined;
                global = funAttrs->global;
            }
        }
        else
            throw std::runtime_error("Internal error: symbol has function type but not function attributes");
    }

    _symbolTable.addFunction(funDecl->getName(), funDecl->getFunType(), alreadyDefined, global);

    auto optFunType = Types::getFunType(funDecl->getFunType());
    if (!optFunType)
        throw std::runtime_error("Internal error: function has non-function type");

    auto [paramTypes, returnType] = optFunType.value();

    if (hasBody)
    {
        for (size_t i{0}; i < paramTypes.size(); i++)
        {
            auto &param = funDecl->getParams()[i];
            auto &type = paramTypes[i];
            _symbolTable.addAutomaticVar(param, *type);
        }

        auto newBlock = typeCheckBlock(funDecl->getOptBody().value(), *returnType);
        return std::make_shared<AST::FunctionDeclaration>(funDecl->getName(), funDecl->getParams(), newBlock, funDecl->getFunType(), funDecl->getOptStorageClass());
    }

    return std::make_shared<AST::FunctionDeclaration>(funDecl->getName(), funDecl->getParams(), std::nullopt, funDecl->getFunType(), funDecl->getOptStorageClass());
}

std::shared_ptr<AST::Declaration>
TypeChecker::typeCheckGlobalDecl(const std::shared_ptr<AST::Declaration> &decl)
{
    if (auto funDecl = std::dynamic_pointer_cast<AST::FunctionDeclaration>(decl))
        return typeCheckFunDecl(funDecl);
    else if (auto varDecl = std::dynamic_pointer_cast<AST::VariableDeclaration>(decl))
        return typeCheckFileScopeVarDecl(varDecl);
    else
        throw std::runtime_error("Internal Error: Unknown type of declaration!");
}

std::shared_ptr<AST::Program>
TypeChecker::typeCheck(const std::shared_ptr<AST::Program> &prog)
{
    std::vector<std::shared_ptr<AST::Declaration>> checkedDecls;
    checkedDecls.reserve(prog->getDeclarations().size());

    for (const auto &decl : prog->getDeclarations())
    {
        auto newDecl = typeCheckGlobalDecl(decl);
        checkedDecls.push_back(newDecl);
    }

    return std::make_shared<AST::Program>(checkedDecls);
}
