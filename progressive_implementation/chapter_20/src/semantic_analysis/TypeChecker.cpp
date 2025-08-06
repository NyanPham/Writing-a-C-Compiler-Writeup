#include <unordered_set>
#include <map>

#include "TypeChecker.h"
#include "AST.h"
#include "Types.h"
#include "Symbols.h"
#include "Const.h"
#include "Initializers.h"
#include "ConstConvert.h"
#include "Rounding.h"

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

bool isLvalue(const std::shared_ptr<AST::Expression> &exp)
{
    static const std::unordered_set<AST::NodeType> lvalueNodeTypes = {
        AST::NodeType::Var,
        AST::NodeType::Dereference,
        AST::NodeType::Subscript,
        AST::NodeType::String,
        AST::NodeType::Arrow,
    };

    if (lvalueNodeTypes.count(exp->getType()) > 0)
        return true;
    if (auto dotExp = std::dynamic_pointer_cast<AST::Dot>(exp))
        return isLvalue(dotExp->getStructOrUnion());

    return false;
}

bool isZeroInt(const std::shared_ptr<Constants::Const> &c)
{
    if (auto constInt = Constants::getConstInt(*c))
        return constInt->val == 0;
    else if (auto constUInt = Constants::getConstUInt(*c))
        return constUInt->val == 0;
    else if (auto constLong = Constants::getConstLong(*c))
        return constLong->val == 0;
    else if (auto constULong = Constants::getConstULong(*c))
        return constULong->val == 0;
    else
        return false;
}

bool isNullPointerConstant(const std::shared_ptr<AST::Expression> &exp)
{
    if (auto c = std::dynamic_pointer_cast<AST::Constant>(exp))
    {
        return isZeroInt(c->getConst());
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

    if (e1->getDataType().value() == e2->getDataType().value())
        return e1->getDataType().value();
    else if (isNullPointerConstant(e1))
        return e2->getDataType().value();
    else if (isNullPointerConstant(e2))
        return e1->getDataType().value();
    else if (
        (
            Types::isPointerType(e1->getDataType().value()) && Types::isVoidType(*Types::getPointerType(e1->getDataType().value())->referencedType) && Types::isPointerType(e2->getDataType().value())) ||
        (Types::isPointerType(e2->getDataType().value()) && Types::isVoidType(*Types::getPointerType(e2->getDataType().value())->referencedType) && Types::isPointerType(e1->getDataType().value())))
        return Types::makePointerType(std::make_shared<Types::DataType>(Types::makeVoidType()));
    else
        throw std::runtime_error("Expressions have incompatible types");
}

std::shared_ptr<AST::Expression> convertByAssignment(const std::shared_ptr<AST::Expression> &exp, const Types::DataType &tgtType)
{
    if (!exp->getDataType().has_value())
        throw std::runtime_error("Internal error: expression is not typechecked");

    if (exp->getDataType().value() == tgtType)
        return exp;
    else if (Types::isArithmetic(exp->getDataType().value()) && Types::isArithmetic(tgtType))
        return convertTo(exp, tgtType);
    else if (isNullPointerConstant(exp) && Types::isPointerType(tgtType))
        return convertTo(exp, tgtType);
    else if (
        (Types::isPointerType(tgtType) && Types::isVoidType(*Types::getPointerType(tgtType)->referencedType) && Types::isPointerType(exp->getDataType().value())) ||
        (Types::isPointerType(tgtType) && Types::isPointerType(exp->getDataType().value()) && Types::isVoidType(*Types::getPointerType(exp->getDataType().value())->referencedType)))
    {
        return convertTo(exp, tgtType);
    }
    else
    {
        throw std::runtime_error("Cannot convert type for assignment");
    }
}

Types::DataType
TypeChecker::getCommonType(const Types::DataType &t1, const Types::DataType &t2)
{
    auto type1 = Types::isCharacter(t1) ? Types::makeIntType() : t1;
    auto type2 = Types::isCharacter(t2) ? Types::makeIntType() : t2;

    if (type1 == type2)
        return type1;

    if (Types::isDoubleType(type1) || Types::isDoubleType(type2))
        return Types::makeDoubleType();

    if (Types::getSize(type1, _typeTable) == Types::getSize(type2, _typeTable))
    {
        if (Types::isSigned(type1))
            return type2;
        else
            return type1;
    }

    if (Types::getSize(type1, _typeTable) > Types::getSize(type2, _typeTable))
        return type1;
    else
        return type2;
}

/*
Convert a constant to static initializer, performing type converion if needed.
*/
Symbols::InitialValue TypeChecker::toStaticInit(const Types::DataType &varType, const std::shared_ptr<AST::Initializer> &init)
{
    auto initList = staticInitHelper(std::make_shared<Types::DataType>(varType), init);
    return Symbols::makeInitial(initList);
}

std::shared_ptr<AST::Initializer>
TypeChecker::makeZeroInit(const Types::DataType &type)
{
    auto scalar = [type](const std::shared_ptr<Constants::Const> &c)
    {
        auto singleInit = std::make_shared<AST::SingleInit>(std::make_shared<AST::Constant>(c));
        singleInit->setDataType(std::make_optional(type));
        return singleInit;
    };

    if (auto arrType = Types::getArrayType(type))
    {
        auto inits = std::vector<std::shared_ptr<AST::Initializer>>();
        for (int i = 0; i < arrType->size; ++i)
        {
            inits.push_back(makeZeroInit(arrType->elemType));
        }
        auto compoundInit = std::make_shared<AST::CompoundInit>(inits);
        compoundInit->setDataType(std::make_optional(type));
        return compoundInit;
    }

    else if (auto strctType = Types::getStructType(type))
    {
        auto memberTypes = _typeTable.getMemberTypes(strctType->tag);
        auto zeroInits = std::vector<std::shared_ptr<AST::Initializer>>();
        for (const auto &memberType : memberTypes)
            zeroInits.push_back(makeZeroInit(memberType));
        auto compoundInit = std::make_shared<AST::CompoundInit>(zeroInits);
        compoundInit->setDataType(std::make_optional(type));
        return compoundInit;
    }

    else if (auto unionType = Types::getUnionType(type))
    {
        auto memberTypes = _typeTable.getMemberTypes(unionType->tag);
        auto zeroInits = std::vector<std::shared_ptr<AST::Initializer>>{makeZeroInit(memberTypes[0])};
        auto compoundInit = std::make_shared<AST::CompoundInit>(zeroInits);
        compoundInit->setDataType(std::make_optional(type));
        return compoundInit;
    }

    else if (Types::isCharType(type) || Types::isSCharType(type))
    {
        return scalar(std::make_shared<Constants::Const>(Constants::ConstChar(0)));
    }
    else if (Types::isIntType(type))
    {
        return scalar(std::make_shared<Constants::Const>(Constants::ConstInt(0)));
    }
    else if (Types::isLongType(type))
    {
        return scalar(std::make_shared<Constants::Const>(Constants::ConstLong(0)));
    }
    else if (Types::isUCharType(type))
    {
        return scalar(std::make_shared<Constants::Const>(Constants::ConstUChar(0)));
    }
    else if (Types::isUIntType(type))
    {
        return scalar(std::make_shared<Constants::Const>(Constants::ConstUInt(0)));
    }
    else if (Types::isULongType(type) || Types::isPointerType(type))
    {
        return scalar(std::make_shared<Constants::Const>(Constants::ConstULong(0)));
    }
    else if (Types::isDoubleType(type))
    {
        return scalar(std::make_shared<Constants::Const>(Constants::ConstDouble{0.0}));
    }
    else // FunType or Void
    {
        throw std::runtime_error("Cannot create zero initializer for type: " + Types::dataTypeToString(type));
    }
}

TypeTableNS::TypeDef TypeChecker::buildStructDef(const std::vector<std::shared_ptr<AST::MemberDeclaration>> &members)
{
    int currentSize = 0;
    int currentAlignment = 1;
    std::map<std::string, TypeTableNS::MemberEntry> memberDefs{};
    std::vector<std::string> memberOrder;

    for (const auto &member : members)
    {
        auto memberName = member->getMemberName();
        auto memberType = member->getMemberType();
        auto memberAlignment = Types::getAlignment(memberType, _typeTable);

        auto offset = Rounding::roundAwayFromZero(memberAlignment, currentSize);
        auto memberEntry = TypeTableNS::MemberEntry(memberType, offset);

        memberDefs[memberName] = memberEntry;
        memberOrder.push_back(memberName);

        currentSize = offset + Types::getSize(memberType, _typeTable);
        currentAlignment = std::max(currentAlignment, memberAlignment);
    }

    auto finalSize = Rounding::roundAwayFromZero(currentAlignment, currentSize);

    return TypeTableNS::TypeDef(currentAlignment, finalSize, memberDefs, memberOrder);
}

TypeTableNS::TypeDef TypeChecker::buildUnionDef(const std::vector<std::shared_ptr<AST::MemberDeclaration>> &members)
{
    int currentSize = 0;
    int currentAlignment = 1;
    std::map<std::string, TypeTableNS::MemberEntry> memberDefs{};
    std::vector<std::string> memberOrder;

    for (const auto &member : members)
    {
        auto memberName = member->getMemberName();
        auto memberType = member->getMemberType();
        auto memberSize = Types::getSize(memberType, _typeTable);
        auto memberAlignment = Types::getAlignment(memberType, _typeTable);

        // All union members have offset 0
        auto memberEntry = TypeTableNS::MemberEntry(memberType, 0);
        memberDefs[memberName] = memberEntry;
        memberOrder.push_back(memberName);

        currentSize = std::max(currentSize, memberSize);
        currentAlignment = std::max(currentAlignment, memberAlignment);
    }

    auto finalSize = Rounding::roundAwayFromZero(currentAlignment, currentSize);

    return TypeTableNS::TypeDef(currentAlignment, finalSize, memberDefs, memberOrder);
}

void TypeChecker::validateTypeDefinition(const std::shared_ptr<AST::TypeDeclaration> &typeDef)
{
    auto strctOrUnion = typeDef->getStructOrUnion();
    auto tag = typeDef->getTag();
    auto members = typeDef->getMembers();

    // first check for conflicting definition in type table
    auto entry = _typeTable.findOpt(tag);
    if (entry.has_value())
    {
        auto kind = entry->kind;
        auto contents = entry->optTypeDef;

        // did we declare this tag with the same sort of type (struct vs. union) both times?
        if (kind != strctOrUnion)
            throw std::runtime_error("Conflicting declaration of " + tag + ": defined as " + (kind == AST::Which::Struct ? "struct" : "union") + " and then as " + (strctOrUnion == AST::Which::Struct ? "struct" : "union"));

        // Did we include a member list both times?
        if (!members.empty() && contents.has_value())
            throw std::runtime_error("Contents of tag {tag} defined twice");
    }

    // check for duplicate number names
    std::unordered_set<std::string> memberNames{};
    for (const auto &member : members)
    {
        auto memberName = member->getMemberName();
        auto memberType = member->getMemberType();
        if (memberNames.count(memberName))
            throw std::runtime_error("Duplicate declaration of member " + memberName + " in structure " + tag);
        else
            memberNames.insert(memberName);
        // validate member type
        validateType(memberType);
        if (Types::isFunType(*memberType))
        {
            // this is redundant, we'd already reject this in parser
            throw std::runtime_error("Can't declare structure member with function type");
        }
        else
        {
            if (Types::isComplete(*memberType, _typeTable))
                ; // Do nothing
            else
                throw std::runtime_error("Cannot declare structure member with incomplete type");
        }
    }
}

void TypeChecker::validateType(const std::shared_ptr<Types::DataType> &type)
{
    if (auto arrType = Types::getArrayType(*type))
    {
        if (Types::isComplete(*arrType->elemType, _typeTable))
            validateType(arrType->elemType);
        else
            throw std::runtime_error("Array of incomplete type");
    }
    else if (auto ptrType = Types::getPointerType(*type))
    {
        validateType(ptrType->referencedType);
    }
    else if (auto fnType = Types::getFunType(*type))
    {
        for (auto &paramType : fnType->paramTypes)
        {
            validateType(paramType);
        }

        validateType(fnType->retType);
    }
    else if (auto structType = Types::getStructType(*type))
    {
        auto optEntry = _typeTable.findOpt(structType->tag);

        if (optEntry.has_value() && optEntry->kind == AST::Which::Union)
            throw std::runtime_error("Tag previously specified union, now specifies struct");
        else
            // Otherwise, either previously added as struct or, if we're just processing its definition now, not at all
            return;
    }
    else if (auto unionType = Types::getUnionType(*type))
    {
        auto optEntry = _typeTable.findOpt(unionType->tag);

        if (optEntry.has_value() && optEntry->kind == AST::Which::Struct)
            throw std::runtime_error("Tag previously specified struct, now specifies union");
        else
            // Otherwise, either previously added as union or, if we're just processing its definition now, not at all
            return;
    }
    else if (
        Types::isCharType(*type) ||
        Types::isSCharType(*type) ||
        Types::isUCharType(*type) ||
        Types::isIntType(*type) ||
        Types::isLongType(*type) ||
        Types::isUIntType(*type) ||
        Types::isULongType(*type) ||
        Types::isDoubleType(*type) ||
        Types::isVoidType(*type))
    {
        return;
    }
    else
    {
        throw std::runtime_error("Internal error: invalid type to validate");
    }
}

std::shared_ptr<AST::Expression>
TypeChecker::typeCheckScalar(const std::shared_ptr<AST::Expression> &exp)
{
    auto typedExp = typeCheckAndConvert(exp);
    if (Types::isScalar(typedExp->getDataType().value()))
        return typedExp;
    else
        throw std::runtime_error("A scalar operand is required");
}

std::shared_ptr<AST::Dot> TypeChecker::typeCheckDotOperator(const std::shared_ptr<AST::Dot> &dot)
{
    auto typedStrctOrUnion = typeCheckAndConvert(dot->getStructOrUnion());

    // Look up definition of base struct/union in the type table
    std::string tag = "";
    if (auto structType = Types::getStructType(typedStrctOrUnion->getDataType().value()))
        tag = structType->tag;
    else if (auto unionType = Types::getUnionType(typedStrctOrUnion->getDataType().value()))
        tag = unionType->tag;
    else
        throw std::runtime_error("Dot operator can only be applied to expressions with structure or union type");

    // typecheck_and_convert already validated that this structure type is complete
    auto innerTypMembers = _typeTable.getMembers(tag);
    Types::DataType memberTyp;

    if (innerTypMembers.count(dot->getMember()))
        memberTyp = *innerTypMembers[dot->getMember()].memberType;
    else
        throw std::runtime_error("Struct/union type" + tag + " has no member " + dot->getMember());

    auto dotExp = std::make_shared<AST::Dot>(typedStrctOrUnion, dot->getMember());
    dotExp->setDataType(std::make_optional(memberTyp));
    return dotExp;
}

std::shared_ptr<AST::Arrow> TypeChecker::typeCheckArrowOperator(const std::shared_ptr<AST::Arrow> &arrow)
{
    auto typedStrctOrUnionPtr = typeCheckAndConvert(arrow->getStructOrUnion());

    // Validate that this is a pointer to a complete type
    if (!Types::isCompletePointer(typedStrctOrUnionPtr->getDataType().value(), _typeTable))
    {
        throw std::runtime_error("Arrow operator can only be applied to pointers to structure or union types");
    }

    // Make sure it's a pointer to a struct or union type specifically
    auto ptrType = Types::getPointerType(typedStrctOrUnionPtr->getDataType().value());
    std::string tag = "";

    if (auto structType = Types::getStructType(*ptrType->referencedType))
        tag = structType->tag;
    else if (auto unionType = Types::getUnionType(*ptrType->referencedType))
        tag = unionType->tag;
    else
        throw std::runtime_error("Arrow operator can only be applied to pointers to complete  structure or union types");

    // figure out member type
    auto innerTypMembers = _typeTable.getMembers(tag);
    Types::DataType memberTyp;
    if (innerTypMembers.count(arrow->getMember()))
        memberTyp = *innerTypMembers[arrow->getMember()].memberType;
    else
        throw std::runtime_error("Struct/union type" + tag + " has no member " + arrow->getMember());

    auto arrowExp = std::make_shared<AST::Arrow>(typedStrctOrUnionPtr, arrow->getMember());
    arrowExp->setDataType(std::make_optional(memberTyp));
    return arrowExp;
}

std::shared_ptr<AST::SizeOfT>
TypeChecker::typeCheckSizeOfT(const std::shared_ptr<AST::SizeOfT> &sizeOfT)
{
    validateType(sizeOfT->getTypeName());
    if (Types::isComplete(*sizeOfT->getTypeName(), _typeTable))
    {
        auto typeCheckedSizeOfT = std::make_shared<AST::SizeOfT>(sizeOfT->getTypeName());
        typeCheckedSizeOfT->setDataType(std::make_optional(Types::makeULongType()));
        return typeCheckedSizeOfT;
    }
    else
    {
        throw std::runtime_error("Can't apply sizeof to incomplete type");
    }
}

std::shared_ptr<AST::SizeOf>
TypeChecker::typeCheckSizeOf(const std::shared_ptr<AST::SizeOf> &sizeOf)
{
    auto typedInner = typeCheckExp(sizeOf->getInnerExp());
    if (Types::isComplete(typedInner->getDataType().value(), _typeTable))
    {
        auto sizeOfExp = std::make_shared<AST::SizeOf>(typedInner);
        sizeOfExp->setDataType(std::make_optional(Types::makeULongType()));
        return sizeOfExp;
    }
    else
    {
        throw std::runtime_error("Can't apply sizeof to incomplete type");
    }
}

std::shared_ptr<AST::String>
TypeChecker::typeCheckString(const std::shared_ptr<AST::String> &string)
{
    auto e = std::make_shared<AST::String>(string->getStr());
    e->setDataType(std::make_optional(Types::makeArrayType(std::make_shared<Types::DataType>(Types::makeCharType()), string->getStr().size() + 1)));
    return e;
}

std::shared_ptr<AST::Initializer>
TypeChecker::typeCheckInit(const Types::DataType &targetType, const std::shared_ptr<AST::Initializer> &init)
{
    if (auto singleInit = std::dynamic_pointer_cast<AST::SingleInit>(init))
    {
        if (Types::isArrayType(targetType) && singleInit->getExp()->getType() == AST::NodeType::String)
        {
            auto arrType = Types::getArrayType(targetType);
            auto string = std::dynamic_pointer_cast<AST::String>(singleInit->getExp());
            if (!Types::isCharacter(*arrType->elemType))
            {
                throw std::runtime_error("Can't initialize non-character type with string literal");
            }
            else if (string->getStr().size() > arrType->size)
            {
                throw std::runtime_error("Too many characters in string literal");
            }
            else
            {
                auto typecheckedInit = std::make_shared<AST::SingleInit>(string);
                typecheckedInit->setDataType(std::make_optional(targetType));
                return typecheckedInit;
            }
        }
        else
        {
            auto typeCheckedExp = typeCheckAndConvert(singleInit->getExp());
            auto castExp = convertByAssignment(typeCheckedExp, targetType);
            auto checkedSingleInit = std::make_shared<AST::SingleInit>(castExp);
            checkedSingleInit->setDataType(std::make_optional(targetType));
            return checkedSingleInit;
        }
    }
    else if (auto compoundInit = std::dynamic_pointer_cast<AST::CompoundInit>(init))
    {
        if (auto arrType = Types::getArrayType(targetType))
        {
            if (compoundInit->getInits().size() > arrType->size)
            {
                throw std::runtime_error("Too many values in initializer");
            }
            else
            {
                auto typeCheckedInits = std::vector<std::shared_ptr<AST::Initializer>>();
                for (const auto &init : compoundInit->getInits())
                {
                    typeCheckedInits.push_back(typeCheckInit(*arrType->elemType, init));
                }
                auto padding = std::vector<std::shared_ptr<AST::Initializer>>();

                for (int i = 0; i < (arrType->size - compoundInit->getInits().size()); i++)
                {
                    padding.push_back(makeZeroInit(*arrType->elemType));
                }

                typeCheckedInits.insert(typeCheckedInits.end(), padding.begin(), padding.end());
                auto typeCheckedCompoundInit = std::make_shared<AST::CompoundInit>(typeCheckedInits);
                typeCheckedCompoundInit->setDataType(std::make_optional(targetType));
                return typeCheckedCompoundInit;
            }
        }
        else if (auto strctType = Types::getStructType(targetType))
        {
            auto memberTypes = _typeTable.getMemberTypes(strctType->tag);
            if (compoundInit->getInits().size() > memberTypes.size())
            {
                throw std::runtime_error("Too many values in structure initializer");
            }
            else
            {
                auto inits = compoundInit->getInits();
                auto initializedMembers = std::vector<Types::DataType>(memberTypes.begin(), memberTypes.begin() + inits.size());
                auto uninitializedMembers = std::vector<Types::DataType>(memberTypes.begin() + inits.size(), memberTypes.end());

                auto typeCheckedMembers = std::vector<std::shared_ptr<AST::Initializer>>();

                for (size_t i = 0; i < initializedMembers.size(); i++)
                {
                    auto memberType = initializedMembers[i];
                    auto init = inits[i];
                    typeCheckedMembers.push_back(typeCheckInit(memberType, init));
                }

                auto padding = std::vector<std::shared_ptr<AST::Initializer>>();

                for (size_t i = 0; i < uninitializedMembers.size(); i++)
                {
                    auto memberType = uninitializedMembers[i];
                    padding.push_back(makeZeroInit(memberType));
                }

                typeCheckedMembers.insert(typeCheckedMembers.end(), padding.begin(), padding.end());
                auto compoundInit = std::make_shared<AST::CompoundInit>(typeCheckedMembers);
                compoundInit->setDataType(std::make_optional(targetType));
                return compoundInit;
            }
        }
        else if (auto unionType = Types::getUnionType(targetType))
        {
            // Compound initializer for union must have exactly one element; it initializes the union's first member
            if (compoundInit->getInits().size() != 1)
            {
                throw std::runtime_error("Initializer list for union must have exactly one element");
            }
            else
            {
                auto elem = compoundInit->getInits()[0];
                auto memberType = _typeTable.getMemberTypes(unionType->tag)[0];
                auto typeCheckedMember = typeCheckInit(memberType, elem);
                // don't need to zero out trailing padding for non-static unions
                auto compoundInit = std::make_shared<AST::CompoundInit>(std::vector<std::shared_ptr<AST::Initializer>>{typeCheckedMember});
                compoundInit->setDataType(std::make_optional(targetType));
                return compoundInit;
            }
        }
        else
        {
            throw std::runtime_error("Can't initialize scalar value from compound initializer");
        }
    }
    else
    {
        throw std::runtime_error("Unsupported initializer type");
    }
}

std::shared_ptr<AST::Cast>
TypeChecker::typeCheckCast(const std::shared_ptr<AST::Cast> &cast)
{
    auto targetType = cast->getTargetType();
    validateType(std::make_shared<Types::DataType>(targetType));
    auto typedInner = typeCheckAndConvert(cast->getExp());

    if (
        (Types::isPointerType(targetType) && Types::isDoubleType(typedInner->getDataType().value())) ||
        (Types::isDoubleType(targetType) && Types::isPointerType(typedInner->getDataType().value())))
        throw std::runtime_error("Cannot cast between pointer and double");

    if (Types::isVoidType(targetType))
    {
        auto castExp = std::make_shared<AST::Cast>(targetType, typedInner);
        castExp->setDataType(std::make_optional(targetType));
        return castExp;
    }

    if (!Types::isScalar(targetType))
        throw std::runtime_error("Can only cast to scalar types or void");
    else if (!Types::isScalar(typedInner->getDataType().value()))
        throw std::runtime_error("Can only cast scalar expressions to non-void type");

    auto castExp = std::make_shared<AST::Cast>(targetType, typedInner);
    castExp->setDataType(std::make_optional(targetType));
    return castExp;
}

std::shared_ptr<AST::Unary>
TypeChecker::typeCheckNot(const std::shared_ptr<AST::Unary> &notUnary)
{
    if (notUnary->getOp() != AST::UnaryOp::Not)
        throw std::runtime_error("Internal error: typeCheckNot called with non-Not operator");

    auto typedInner = typeCheckScalar(notUnary->getExp());
    auto notExp = std::make_shared<AST::Unary>(AST::UnaryOp::Not, typedInner);
    notExp->setDataType(std::make_optional(Types::makeIntType()));
    return notExp;
}

std::shared_ptr<AST::Unary>
TypeChecker::typeCheckComplement(const std::shared_ptr<AST::Unary> &complUnary)
{
    if (complUnary->getOp() != AST::UnaryOp::Complement)
        throw std::runtime_error("Internal error: typeCheckComplement called with non-Complement operator");

    auto typedInner = typeCheckAndConvert(complUnary->getExp());
    if (!Types::isInteger(typedInner->getDataType().value()))
        throw std::runtime_error("Bitwise complement only valid for integer types");

    // promote character types to int
    typedInner = Types::isCharacter(typedInner->getDataType().value()) ? convertTo(typedInner, Types::makeIntType()) : typedInner;

    auto complExp = std::make_shared<AST::Unary>(AST::UnaryOp::Complement, typedInner);
    complExp->setDataType(typedInner->getDataType());
    return complExp;
}

std::shared_ptr<AST::Unary>
TypeChecker::typeCheckNegate(const std::shared_ptr<AST::Unary> &negUnary)
{
    if (negUnary->getOp() != AST::UnaryOp::Negate)
        throw std::runtime_error("Internal error: typeCheckNegate called with non-Negate operator");

    auto typedInner = typeCheckAndConvert(negUnary->getExp());

    if (Types::isArithmetic(typedInner->getDataType().value()))
    {
        // promote character types to int
        typedInner = Types::isCharacter(typedInner->getDataType().value()) ? convertTo(typedInner, Types::makeIntType()) : typedInner;

        auto negExp = std::make_shared<AST::Unary>(AST::UnaryOp::Negate, typedInner);
        negExp->setDataType(typedInner->getDataType());
        return negExp;
    }

    throw std::runtime_error("Can only negate arithmetic types");
    return nullptr;
}

std::shared_ptr<AST::Unary>
TypeChecker::typeCheckIncrDecr(const std::shared_ptr<AST::Unary> &incrDecrUnary)
{
    auto typedInner = typeCheckAndConvert(incrDecrUnary->getExp());
    if (isLvalue(typedInner) && (Types::isArithmetic(typedInner->getDataType().value()) || Types::isCompletePointer(typedInner->getDataType().value(), _typeTable)))
    {
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
    auto typedE1 = typeCheckScalar(logicalBinary->getExp1());
    auto typedE2 = typeCheckScalar(logicalBinary->getExp2());
    auto typedBinExp = std::make_shared<AST::Binary>(logicalBinary->getOp(), typedE1, typedE2);
    typedBinExp->setDataType(std::make_optional(Types::makeIntType()));
    return typedBinExp;
}

std::shared_ptr<AST::Binary>
TypeChecker::typeCheckAddition(const std::shared_ptr<AST::Binary> &addition)
{
    auto typedE1 = typeCheckAndConvert(addition->getExp1());
    auto typedE2 = typeCheckAndConvert(addition->getExp2());

    if (Types::isArithmetic(typedE1->getDataType().value()) && Types::isArithmetic(typedE2->getDataType().value()))
    {
        auto commonType = getCommonType(typedE1->getDataType().value(), typedE2->getDataType().value());
        auto convertedE1 = convertTo(typedE1, commonType);
        auto convertedE2 = convertTo(typedE2, commonType);
        auto addExp = std::make_shared<AST::Binary>(AST::BinaryOp::Add, convertedE1, convertedE2);
        addExp->setDataType(std::make_optional(commonType));
        return addExp;
    }
    else if (Types::isCompletePointer(typedE1->getDataType().value(), _typeTable) && Types::isInteger(typedE2->getDataType().value()))
    {
        auto convertedE2 = convertTo(typedE2, Types::makeLongType());
        auto addExp = std::make_shared<AST::Binary>(AST::BinaryOp::Add, typedE1, convertedE2);
        addExp->setDataType(typedE1->getDataType());
        return addExp;
    }
    else if (Types::isCompletePointer(typedE2->getDataType().value(), _typeTable) && Types::isInteger(typedE1->getDataType().value()))
    {
        auto convertedE1 = convertTo(typedE1, Types::makeLongType());
        auto addExp = std::make_shared<AST::Binary>(AST::BinaryOp::Add, convertedE1, typedE2);
        addExp->setDataType(typedE2->getDataType());
        return addExp;
    }
    else
    {
        throw std::runtime_error("Invalid operands for addition");
    }
}

std::shared_ptr<AST::Binary>
TypeChecker::typeCheckSubtraction(const std::shared_ptr<AST::Binary> &subtraction)
{
    auto typedE1 = typeCheckAndConvert(subtraction->getExp1());
    auto typedE2 = typeCheckAndConvert(subtraction->getExp2());

    if (Types::isArithmetic(typedE1->getDataType().value()) && Types::isArithmetic(typedE2->getDataType().value()))
    {
        auto commonType = getCommonType(typedE1->getDataType().value(), typedE2->getDataType().value());
        auto convertedE1 = convertTo(typedE1, commonType);
        auto convertedE2 = convertTo(typedE2, commonType);
        auto subExp = std::make_shared<AST::Binary>(AST::BinaryOp::Subtract, convertedE1, convertedE2);
        subExp->setDataType(std::make_optional(commonType));
        return subExp;
    }
    else if (Types::isCompletePointer(typedE1->getDataType().value(), _typeTable) && Types::isInteger(typedE2->getDataType().value()))
    {
        auto convertedE2 = convertTo(typedE2, Types::makeLongType());
        auto subExp = std::make_shared<AST::Binary>(AST::BinaryOp::Subtract, typedE1, convertedE2);
        subExp->setDataType(typedE1->getDataType());
        return subExp;
    }
    else if (Types::isCompletePointer(typedE1->getDataType().value(), _typeTable) && typedE1->getDataType().value() == typedE2->getDataType().value())
    {
        auto subExp = std::make_shared<AST::Binary>(AST::BinaryOp::Subtract, typedE1, typedE2);
        subExp->setDataType(std::make_optional(Types::makeLongType()));
        return subExp;
    }
    else
    {
        throw std::runtime_error("Invalid operands for subtraction");
    }
}

std::shared_ptr<AST::Binary>
TypeChecker::typeCheckMultiplicative(const std::shared_ptr<AST::Binary> &multiplicative)
{
    auto typedE1 = typeCheckAndConvert(multiplicative->getExp1());
    auto typedE2 = typeCheckAndConvert(multiplicative->getExp2());

    if (Types::isArithmetic(typedE1->getDataType().value()) && Types::isArithmetic(typedE2->getDataType().value()))
    {
        auto commonType = getCommonType(typedE1->getDataType().value(), typedE2->getDataType().value());
        auto convertedE1 = convertTo(typedE1, commonType);
        auto convertedE2 = convertTo(typedE2, commonType);
        auto binExp = std::make_shared<AST::Binary>(multiplicative->getOp(), convertedE1, convertedE2);

        if (multiplicative->getOp() == AST::BinaryOp::Remainder && Types::isDoubleType(commonType))
        {
            throw std::runtime_error("Can't apply \% to double");
        }
        else if (multiplicative->getOp() == AST::BinaryOp::Multiply ||
                 multiplicative->getOp() == AST::BinaryOp::Divide ||
                 multiplicative->getOp() == AST::BinaryOp::Remainder)
        {
            binExp->setDataType(std::make_optional(commonType));
            return binExp;
        }
        else
        {
            throw std::runtime_error("Can only multiply arithmetic types");
        }
    }

    throw std::runtime_error("Multiplicative operations not permitted on pointers");
}

std::shared_ptr<AST::Binary>
TypeChecker::typeCheckEquality(const std::shared_ptr<AST::Binary> &equality)
{
    auto typedE1 = typeCheckAndConvert(equality->getExp1());
    auto typedE2 = typeCheckAndConvert(equality->getExp2());

    Types::DataType commonType;

    if (Types::isPointerType(typedE1->getDataType().value()) || Types::isPointerType(typedE2->getDataType().value()))
    {
        commonType = getCommonPointerType(typedE1, typedE2);
    }
    else if (Types::isArithmetic(typedE1->getDataType().value()) && Types::isArithmetic(typedE2->getDataType().value()))
    {
        commonType = getCommonType(typedE1->getDataType().value(), typedE2->getDataType().value());
    }
    else
    {
        throw std::runtime_error("Invalid operands for equality");
    }

    auto convertedE1 = convertTo(typedE1, commonType);
    auto convertedE2 = convertTo(typedE2, commonType);
    auto binExp = std::make_shared<AST::Binary>(equality->getOp(), convertedE1, convertedE2);
    binExp->setDataType(std::make_optional(Types::makeIntType()));
    return binExp;
}

std::shared_ptr<AST::Binary>
TypeChecker::typeCheckComparison(const std::shared_ptr<AST::Binary> &comparison)
{
    auto typedE1 = typeCheckAndConvert(comparison->getExp1());
    auto typedE2 = typeCheckAndConvert(comparison->getExp2());

    Types::DataType commonType;

    if (Types::isArithmetic(typedE1->getDataType().value()) && Types::isArithmetic(typedE2->getDataType().value()))
    {
        commonType = getCommonType(typedE1->getDataType().value(), typedE2->getDataType().value());
    }
    else if (Types::isPointerType(typedE1->getDataType().value()) && typedE1->getDataType().value() == typedE2->getDataType().value())
    {
        commonType = typedE1->getDataType().value();
    }
    else
    {
        throw std::runtime_error("Invalid types for comparison");
    }

    auto convertedE1 = convertTo(typedE1, commonType);
    auto convertedE2 = convertTo(typedE2, commonType);
    auto binaryExp = std::make_shared<AST::Binary>(comparison->getOp(), convertedE1, convertedE2);
    binaryExp->setDataType(std::make_optional(Types::makeIntType()));
    return binaryExp;
}

std::shared_ptr<AST::Binary>
TypeChecker::typeCheckBitwise(const std::shared_ptr<AST::Binary> &bitwise)
{
    auto typedE1 = typeCheckAndConvert(bitwise->getExp1());
    auto typedE2 = typeCheckAndConvert(bitwise->getExp2());

    if (!(Types::isInteger(typedE1->getDataType().value()) && Types::isInteger(typedE2->getDataType().value())))
    {
        throw std::runtime_error("Both operands of bitwise operation must be integers");
    }
    else
    {
        auto commonType = getCommonType(typedE1->getDataType().value(), typedE2->getDataType().value());
        auto convertedE1 = convertTo(typedE1, commonType);
        auto convertedE2 = convertTo(typedE2, commonType);
        auto binExp = std::make_shared<AST::Binary>(bitwise->getOp(), convertedE1, convertedE2);
        binExp->setDataType(std::make_optional(commonType));
        return binExp;
    }
}

std::shared_ptr<AST::Binary>
TypeChecker::typeCheckBitShift(const std::shared_ptr<AST::Binary> &bitShiftBinary)
{
    auto typedE1 = typeCheckAndConvert(bitShiftBinary->getExp1());
    auto typedE2 = typeCheckAndConvert(bitShiftBinary->getExp2());

    if (!(Types::isInteger(typedE1->getDataType().value()) && Types::isInteger(typedE2->getDataType().value())))
    {
        throw std::runtime_error("Both operands of bitshift operation must be integers");
    }
    else
    {
        // promote borht operands from character to int type
        typedE1 = Types::isCharacter(typedE1->getDataType().value()) ? convertTo(typedE1, Types::makeIntType()) : typedE1;
        typedE2 = Types::isCharacter(typedE2->getDataType().value()) ? convertTo(typedE2, Types::makeIntType()) : typedE2;

        // Don't perform usual arithmetic conversions; result has type of left operand
        auto typedBinExp = std::make_shared<AST::Binary>(bitShiftBinary->getOp(), typedE1, typedE2);
        typedBinExp->setDataType(typedE1->getDataType());
        return typedBinExp;
    }
}

std::shared_ptr<AST::Dereference>
TypeChecker::typeCheckDereference(const std::shared_ptr<AST::Dereference> &dereference)
{
    auto typedInner = typeCheckAndConvert(dereference->getInnerExp());

    if (Types::isPointerType(*typedInner->getDataType()) && Types::isVoidType(*Types::getPointerType(*typedInner->getDataType())->referencedType))
        throw std::runtime_error("Can't dereference pointer to void");
    else if (auto pointerType = Types::getPointerType(typedInner->getDataType().value()))
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
    auto typedInner = typeCheckExp(addrOf->getInnerExp());
    if (isLvalue(typedInner))
    {
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

std::shared_ptr<AST::Subscript>
TypeChecker::typeCheckSubscript(const std::shared_ptr<AST::Subscript> &subscript)
{
    auto typedE1 = typeCheckAndConvert(subscript->getExp1());
    auto typedE2 = typeCheckAndConvert(subscript->getExp2());

    Types::DataType ptrType;
    std::shared_ptr<AST::Expression> convertedE1;
    std::shared_ptr<AST::Expression> convertedE2;

    if (Types::isCompletePointer(typedE1->getDataType().value(), _typeTable) && Types::isInteger(typedE2->getDataType().value()))
    {
        ptrType = typedE1->getDataType().value();
        convertedE1 = typedE1;
        convertedE2 = convertTo(typedE2, Types::makeLongType());
    }
    else if (Types::isCompletePointer(typedE2->getDataType().value(), _typeTable) && Types::isInteger(typedE1->getDataType().value()))
    {
        ptrType = typedE2->getDataType().value();
        convertedE1 = convertTo(typedE1, Types::makeLongType());
        convertedE2 = typedE2;
    }
    else
    {
        throw std::runtime_error("Invalid types for subscript operation");
    }

    Types::DataType resultType;

    if (auto _ptrType = Types::getPointerType(ptrType))
    {
        resultType = *_ptrType->referencedType;
    }
    else
    {
        throw std::runtime_error("Internal error: expected pointer type for subscript operation");
    }

    auto subscriptExp = std::make_shared<AST::Subscript>(convertedE1, convertedE2);
    subscriptExp->setDataType(std::make_optional(resultType));
    return subscriptExp;
}

std::shared_ptr<AST::Expression>
TypeChecker::typeCheckAndConvert(const std::shared_ptr<AST::Expression> &exp)
{
    auto typedExp = typeCheckExp(exp);
    const auto &dt = typedExp->getDataType().value();

    if ((Types::isStructType(dt) || Types::isUnionType(dt)) && !Types::isComplete(dt, _typeTable))
        throw std::runtime_error("Incomplete structure type not permitted here");

    if (auto arrType = Types::getArrayType(dt))
    {
        auto addrExp = std::make_shared<AST::AddrOf>(typedExp);
        addrExp->setDataType(std::make_optional(Types::makePointerType(arrType->elemType)));
        return addrExp;
    }

    return typedExp;
}

std::vector<std::shared_ptr<Initializers::StaticInit>>
TypeChecker::staticInitHelper(const std::shared_ptr<Types::DataType> &varType, const std::shared_ptr<AST::Initializer> &init)
{
    if (init->getType() == AST::NodeType::SingleInit && std::dynamic_pointer_cast<AST::SingleInit>(init)->getExp()->getType() == AST::NodeType::String)
    {
        if (Types::isArrayType(*varType))
        {
            auto arrType = Types::getArrayType(*varType);
            auto singleInit = std::dynamic_pointer_cast<AST::SingleInit>(init);
            auto string = std::dynamic_pointer_cast<AST::String>(singleInit->getExp());

            if (Types::isCharacter(*arrType->elemType))
            {
                auto n = arrType->size - (int)(string->getStr().size());

                if (n == 0)
                    return {
                        std::make_shared<Initializers::StaticInit>(Initializers::StringInit{string->getStr(), false}),
                    };
                else if (n == 1)
                    return {
                        std::make_shared<Initializers::StaticInit>(Initializers::StringInit{string->getStr(), true}),
                    };
                else if (n > 1)
                    return {
                        std::make_shared<Initializers::StaticInit>(Initializers::StringInit{string->getStr(), true}),
                        std::make_shared<Initializers::StaticInit>(Initializers::ZeroInit{static_cast<size_t>(n - 1)}),
                    };
                else
                    throw std::runtime_error("string is too long for initialize");
            }
            else
            {
                throw std::runtime_error("Can't initailize array of non-character type with string literal");
            }
        }

        if (Types::isPointerType(*varType) && Types::isCharType(*Types::getPointerType(*varType)->referencedType))
        {
            auto singleInit = std::dynamic_pointer_cast<AST::SingleInit>(init);
            auto string = std::dynamic_pointer_cast<AST::String>(singleInit->getExp());

            auto strId = _symbolTable.addString(string->getStr());
            return {
                std::make_shared<Initializers::StaticInit>(Initializers::PointerInit(strId)),
            };
        }

        throw std::runtime_error("String literal can only initialize char or decay to pointer");
    }

    else if (Types::isStructType(*varType) && init->getType() == AST::NodeType::CompoundInit)
    {
        auto strctType = Types::getStructType(*varType);
        auto compoundInit = std::dynamic_pointer_cast<AST::CompoundInit>(init);

        auto members = _typeTable.getFlattenMembers(strctType->tag);

        if (compoundInit->getInits().size() > members.size())
        {
            throw std::runtime_error("Too many elements in struct initializer");
        }
        else
        {
            int currentOffset = 0;
            auto currentInits = std::vector<std::shared_ptr<Initializers::StaticInit>>{};

            for (size_t i = 0; i < compoundInit->getInits().size(); i++)
            {
                auto init = compoundInit->getInits()[i];
                auto memb = members[i];
                auto padding = std::vector<std::shared_ptr<Initializers::StaticInit>>{};
                if (currentOffset < memb.offset)
                    padding.push_back(std::make_shared<Initializers::StaticInit>(Initializers::ZeroInit{static_cast<size_t>(memb.offset - currentOffset)}));

                auto moreStaticInits = staticInitHelper(memb.memberType, init);
                currentInits.insert(currentInits.end(), padding.begin(), padding.end());
                currentInits.insert(currentInits.end(), moreStaticInits.begin(), moreStaticInits.end());
                currentOffset = memb.offset + Types::getSize(memb.memberType, _typeTable);
            }

            auto structSize = Types::getSize(*varType, _typeTable);

            if (currentOffset < structSize)
                currentInits.push_back(std::make_shared<Initializers::StaticInit>(Initializers::ZeroInit{static_cast<size_t>(structSize - currentOffset)}));

            return currentInits;
        }
    }

    else if (Types::isUnionType(*varType) && init->getType() == AST::NodeType::CompoundInit)
    {
        auto unionType = Types::getUnionType(*varType);
        auto compoundInit = std::dynamic_pointer_cast<AST::CompoundInit>(init);

        // Union initializer list must have one element, initializing first member
        if (compoundInit->getInits().size() != 1)
        {
            throw std::runtime_error("Compound initializer for union must have exactly one value");
        }
        else
        {
            auto elem = compoundInit->getInits()[0];

            auto memberType = _typeTable.getMemberTypes(unionType->tag)[0];
            auto unionSize = Types::getSize(*varType, _typeTable);

            // recursively initialize this member
            auto unionInit = staticInitHelper(std::make_shared<Types::DataType>(memberType), elem);
            // if member size < total union size, add trailing padding
            auto initializedSize = Types::getSize(memberType, _typeTable);

            if (initializedSize < unionSize)
                unionInit.push_back(std::make_shared<Initializers::StaticInit>(Initializers::ZeroInit{static_cast<size_t>(unionSize - initializedSize)}));

            return unionInit;
        }
    }

    else if ((Types::isStructType(*varType) || Types::isUnionType(*varType)) && init->getType() == AST::NodeType::SingleInit)
    {
        throw std::runtime_error("Can't initialize static structure or union from scalar value");
    }

    else if (Types::isArrayType(*varType) && init->getType() == AST::NodeType::SingleInit)
    {
        throw std::runtime_error("Can't initialize array from scalar value");
    }

    else if (
        init->getType() == AST::NodeType::SingleInit &&
        [&]
        {
            auto singleInit = std::dynamic_pointer_cast<AST::SingleInit>(init);
            if (!singleInit)
                return false;
            auto constExp = std::dynamic_pointer_cast<AST::Constant>(singleInit->getExp());
            return constExp && isZeroInt(constExp->getConst());
        }())
    {
        return Initializers::zero(*varType, _typeTable);
    }
    else if (Types::isPointerType(*varType))
    {
        throw std::runtime_error("Invalid static initializer for pointer");
    }
    else if (auto singleInit = std::dynamic_pointer_cast<AST::SingleInit>(init))
    {
        if (auto constExp = std::dynamic_pointer_cast<AST::Constant>(singleInit->getExp()))
        {
            if (Types::isArithmetic(*varType))
            {
                std::shared_ptr<Initializers::StaticInit> initVal = nullptr;
                auto convertedC = ConstConvert::convert(*varType, constExp->getConst());

                if (auto constChar = Constants::getConstChar(*convertedC))
                    initVal = std::make_shared<Initializers::StaticInit>(Initializers::CharInit{constChar->val});
                else if (auto constInt = Constants::getConstInt(*convertedC))
                    initVal = std::make_shared<Initializers::StaticInit>(Initializers::IntInit{constInt->val});
                else if (auto constLong = Constants::getConstLong(*convertedC))
                    initVal = std::make_shared<Initializers::StaticInit>(Initializers::LongInit{constLong->val});
                else if (auto constUChar = Constants::getConstUChar(*convertedC))
                    initVal = std::make_shared<Initializers::StaticInit>(Initializers::UCharInit{constUChar->val});
                else if (auto constUInt = Constants::getConstUInt(*convertedC))
                    initVal = std::make_shared<Initializers::StaticInit>(Initializers::UIntInit{constUInt->val});
                else if (auto constULong = Constants::getConstULong(*convertedC))
                    initVal = std::make_shared<Initializers::StaticInit>(Initializers::ULongInit{constULong->val});
                else if (auto constDouble = Constants::getConstDouble(*convertedC))
                    initVal = std::make_shared<Initializers::StaticInit>(Initializers::DoubleInit{constDouble->val});
                else
                {
                    throw std::runtime_error("invalid static initializer");
                }

                return {initVal};
            }
            else
            {
                throw std::runtime_error("Internal error: should have already rejected initializer with type");
            }
        }
        else
        {
            throw std::runtime_error("non-constant initializer");
        }
    }
    else if (auto compoundInit = std::dynamic_pointer_cast<AST::CompoundInit>(init))
    {
        if (auto arrType = Types::getArrayType(*varType))
        {
            auto staticInits = std::vector<std::shared_ptr<Initializers::StaticInit>>{};
            for (auto &init : compoundInit->getInits())
            {
                auto subInits = staticInitHelper(arrType->elemType, init);
                staticInits.insert(staticInits.end(), subInits.begin(), subInits.end());
            }

            auto n = arrType->size - (ssize_t)(compoundInit->getInits().size());
            auto padding = std::vector<std::shared_ptr<Initializers::StaticInit>>{};

            if (n == 0)
            {
                // No padding needed
            }
            else if (n > 0)
            {
                auto zeroBytes = (size_t)(Types::getSize(*arrType->elemType, _typeTable) * n);
                padding.push_back(std::make_shared<Initializers::StaticInit>(Initializers::ZeroInit{zeroBytes}));
            }
            else
            {
                throw std::runtime_error("Too many values in initializer for array of type " + Types::dataTypeToString(*varType));
            }

            staticInits.insert(staticInits.end(), padding.begin(), padding.end());
            return staticInits;
        }
        else
        {
            throw std::runtime_error("Can't use compound initialzier for object with scalar type");
        }
    }
    else
    {
        throw std::runtime_error("Invalid initializer type");
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
    auto typedLhs = typeCheckAndConvert(assignment->getLeftExp());
    if (isLvalue(typedLhs))
    {
        auto lhsType = typedLhs->getDataType().value();
        auto typedRhs = typeCheckAndConvert(assignment->getRightExp());
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
    auto typedLhs = typeCheckAndConvert(compoundAssign->getLeftExp());
    if (isLvalue(typedLhs))
    {
        auto lhsType = typedLhs->getDataType().value();
        auto typedRhs = typeCheckAndConvert(compoundAssign->getRightExp());
        auto rhsType = typedRhs->getDataType().value();

        // %= and compound bitwise ops only permit integer types
        if (
            (compoundAssign->getOp() == AST::BinaryOp::Remainder ||
             compoundAssign->getOp() == AST::BinaryOp::BitwiseAnd ||
             compoundAssign->getOp() == AST::BinaryOp::BitwiseOr ||
             compoundAssign->getOp() == AST::BinaryOp::BitwiseXor ||
             compoundAssign->getOp() == AST::BinaryOp::BitShiftLeft ||
             compoundAssign->getOp() == AST::BinaryOp::BitShiftRight) &&
            (!Types::isInteger(*typedLhs->getDataType()) || !Types::isInteger(*typedRhs->getDataType())))
        {
            throw std::runtime_error("Operand of compound assignment only supports integer operands");
        }

        // *= and /= only support arithmetic types
        if (
            (compoundAssign->getOp() == AST::BinaryOp::Multiply || compoundAssign->getOp() == AST::BinaryOp::Divide) &&
            (!Types::isArithmetic(lhsType) || !Types::isArithmetic(rhsType)))
        {
            throw std::runtime_error("Operand of compound assignment does not support pointer operands");
        }

        // += and -= require either two arithmetic operators, or pointer on LHS and integer on RHS
        if (
            (compoundAssign->getOp() == AST::BinaryOp::Add || compoundAssign->getOp() == AST::BinaryOp::Subtract) &&
            !(Types::isArithmetic(lhsType) && Types::isArithmetic(rhsType)) &&
            !(Types::isCompletePointer(lhsType, _typeTable) && Types::isInteger(rhsType)))
        {
            throw std::runtime_error("Invalid types for +=/-=");
        }

        Types::DataType resultType;
        std::shared_ptr<AST::Expression> convertedRhs;

        if (compoundAssign->getOp() == AST::BinaryOp::BitShiftLeft || compoundAssign->getOp() == AST::BinaryOp::BitShiftRight)
        {
            // Apply integer type promotions to >>= and <<=, but don't convert to common type
            lhsType = Types::isCharacter(lhsType) ? Types::makeIntType() : lhsType;
            resultType = lhsType;
            convertedRhs = Types::isCharacter(typedRhs->getDataType().value()) ? convertTo(typedRhs, Types::makeIntType()) : typedRhs;
        }
        else if (Types::isPointerType(lhsType))
        {
            // For += and -= with pointers, convert RHS to Long and leave LHS type as result type
            resultType = lhsType;
            convertedRhs = convertTo(typedRhs, Types::makeLongType());
        }
        else
        {
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
    auto typedExp = typeCheckAndConvert(postfixDecr->getExp());
    if (isLvalue(typedExp) && (Types::isArithmetic(typedExp->getDataType().value()) || Types::isCompletePointer(typedExp->getDataType().value(), _typeTable)))
    {
        // Result has same value as e; no conversions required.
        // We need to convert integer "1" to their common type, but that will always be the same type as e, at least w/ types we've added so far
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
    auto typedExp = typeCheckAndConvert(postfixIncr->getExp());
    if (isLvalue(typedExp) && (Types::isArithmetic(typedExp->getDataType().value()) || Types::isCompletePointer(typedExp->getDataType().value(), _typeTable)))
    {
        // Same deal as postfix decrement
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
    auto typedCondition = typeCheckScalar(conditional->getCondition());
    auto typedThen = typeCheckAndConvert(conditional->getThen());
    auto typedElse = typeCheckAndConvert(conditional->getElse());

    Types::DataType resultType;

    if (Types::isVoidType(typedThen->getDataType().value()) && Types::isVoidType(typedElse->getDataType().value()))
        resultType = Types::makeVoidType();
    else if (Types::isPointerType(typedThen->getDataType().value()) || Types::isPointerType(typedElse->getDataType().value()))
        resultType = getCommonPointerType(typedThen, typedElse);
    else if (Types::isArithmetic(typedThen->getDataType().value()) && Types::isArithmetic(typedElse->getDataType().value()))
        resultType = getCommonType(typedThen->getDataType().value(), typedElse->getDataType().value());
    else if (typedThen->getDataType().value() == typedElse->getDataType().value())
        // only other option is structure/union types, this is fine if they're identical
        // (typecheck_and_convert already validated that they're complete)
        resultType = typedThen->getDataType().value();
    else
        throw std::runtime_error("Invalid operands for conditional");

    auto convertedThen = convertTo(typedThen, resultType);
    auto convertedElse = convertTo(typedElse, resultType);

    auto typedConditional = std::make_shared<AST::Conditional>(typedCondition, convertedThen, convertedElse);
    typedConditional->setDataType(std::make_optional(resultType));
    return typedConditional;
}

std::shared_ptr<AST::FunctionCall>
TypeChecker::typeCheckFunctionCall(const std::shared_ptr<AST::FunctionCall> &funCall)
{
    auto sType = _symbolTable.get(funCall->getName()).type;

    if (auto fTypeOpt = Types::getFunType(sType))
    {
        const Types::FunType &fType = *fTypeOpt;
        if (fType.paramTypes.size() != funCall->getArgs().size())
            throw std::runtime_error("Function called with wrong number of arguments: " + funCall->getName());

        auto convertedArgs = std::vector<std::shared_ptr<AST::Expression>>();
        for (size_t i{0}; i < fType.paramTypes.size(); i++)
        {
            auto paramType = fType.paramTypes[i];
            auto arg = funCall->getArgs()[i];
            auto newArg = convertByAssignment(typeCheckAndConvert(arg), *paramType);
            convertedArgs.push_back(newArg);
        }

        auto typedCall = std::make_shared<AST::FunctionCall>(funCall->getName(), convertedArgs);
        typedCall->setDataType(std::make_optional(*fType.retType));
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
    case AST::NodeType::String:
    {
        return typeCheckString(std::dynamic_pointer_cast<AST::String>(exp));
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
            return typeCheckAddition(binary);
        case AST::BinaryOp::Subtract:
            return typeCheckSubtraction(binary);
        case AST::BinaryOp::Multiply:
        case AST::BinaryOp::Divide:
        case AST::BinaryOp::Remainder:
            return typeCheckMultiplicative(binary);
        case AST::BinaryOp::BitwiseAnd:
        case AST::BinaryOp::BitwiseOr:
        case AST::BinaryOp::BitwiseXor:
            return typeCheckBitwise(binary);
        case AST::BinaryOp::Equal:
        case AST::BinaryOp::NotEqual:
            return typeCheckEquality(binary);
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
    case AST::NodeType::Subscript:
    {
        return typeCheckSubscript(std::dynamic_pointer_cast<AST::Subscript>(exp));
    }
    case AST::NodeType::SizeOfT:
    {
        return typeCheckSizeOfT(std::dynamic_pointer_cast<AST::SizeOfT>(exp));
    }
    case AST::NodeType::SizeOf:
    {
        return typeCheckSizeOf(std::dynamic_pointer_cast<AST::SizeOf>(exp));
    }
    case AST::NodeType::Dot:
    {
        return typeCheckDotOperator(std::dynamic_pointer_cast<AST::Dot>(exp));
    }
    case AST::NodeType::Arrow:
    {
        return typeCheckArrowOperator(std::dynamic_pointer_cast<AST::Arrow>(exp));
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
    case AST::NodeType::TypeDeclaration:
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
        auto returnStmt = std::dynamic_pointer_cast<AST::Return>(stmt);

        if (returnStmt->getOptValue().has_value())
        {
            if (Types::isVoidType(retType))
            {
                throw std::runtime_error("function with void return type cannot return a value");
            }
            else
            {
                auto typedExp = convertByAssignment(typeCheckAndConvert(returnStmt->getOptValue().value()), retType);
                return std::make_shared<AST::Return>(typedExp);
            }
        }
        else
        {
            if (Types::isVoidType(retType))
            {
                return std::make_shared<AST::Return>();
            }
            else
            {
                throw std::runtime_error("function with non-void return type must return a value");
            }
        }
    }
    case AST::NodeType::ExpressionStmt:
    {
        auto newExp = typeCheckAndConvert(std::dynamic_pointer_cast<AST::ExpressionStmt>(stmt)->getExp());
        return std::make_shared<AST::ExpressionStmt>(newExp);
    }
    case AST::NodeType::If:
    {
        auto ifStmt = std::dynamic_pointer_cast<AST::If>(stmt);
        auto newCond = typeCheckScalar(ifStmt->getCondition());
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
        auto typedExp = typeCheckAndConvert(caseStmt->getValue());
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
        auto typedControl = typeCheckAndConvert(switchStmt->getControl());

        if (!Types::isInteger(typedControl->getDataType().value()))
            throw std::runtime_error("Controlling expression in switch must have integer type");

        // Perform integer promotions on controlling expression
        typedControl = Types::isCharacter(typedControl->getDataType().value()) ? convertTo(typedControl, Types::makeIntType()) : typedControl;
        auto typedBody = typeCheckStatement(switchStmt->getBody(), retType);

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
        auto newCond = typeCheckScalar(whileStmt->getCondition());
        auto newBody = typeCheckStatement(whileStmt->getBody(), retType);

        return std::make_shared<AST::While>(newCond, newBody, whileStmt->getId());
    }
    case AST::NodeType::DoWhile:
    {
        auto doWhileStmt = std::dynamic_pointer_cast<AST::DoWhile>(stmt);
        auto newBody = typeCheckStatement(doWhileStmt->getBody(), retType);
        auto newCond = typeCheckScalar(doWhileStmt->getCondition());

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
                newInit = std::make_shared<AST::InitExp>(typeCheckAndConvert(initExp->getOptExp().value()));
        }

        auto newCond = std::optional<std::shared_ptr<AST::Expression>>{std::nullopt};
        if (forStmt->getOptCondition().has_value())
        {
            newCond = std::make_optional(typeCheckScalar(forStmt->getOptCondition().value()));
        }

        auto newPost = std::optional<std::shared_ptr<AST::Expression>>{std::nullopt};
        if (forStmt->getOptPost().has_value())
        {
            newPost = std::make_optional(typeCheckAndConvert(forStmt->getOptPost().value()));
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

std::shared_ptr<AST::TypeDeclaration>
TypeChecker::typeCheckTypeDecl(const std::shared_ptr<AST::TypeDeclaration> &typeDecl)
{
    // first validate the definition
    validateTypeDefinition(typeDecl);
    auto structOrUnion = typeDecl->getStructOrUnion();
    auto tag = typeDecl->getTag();
    auto members = typeDecl->getMembers();

    /*
        Next, the build type table entry. We can skip this if we've already
        defined this type (including its contents). But if it's not already in
        the type table, we'll add it now, even if this is just a declaration
        (rather than a definition), so we can distinguish conflicting struct/union
        declarations
    */

    auto t = structOrUnion == AST::Which::Struct
                 ? Types::makeStructType(tag)
                 : Types::makeUnionType(tag);

    if (!Types::isComplete(t, _typeTable))
    {
        std::optional<TypeTableNS::TypeDef> typeDef;

        if (members.empty())
            typeDef = std::nullopt;
        else
            typeDef = structOrUnion == AST::Which::Struct
                          ? std::make_optional(buildStructDef(members))
                          : std::make_optional(buildUnionDef(members));

        _typeTable.addTypeDefinition(tag, TypeTableNS::TypeEntry(structOrUnion, typeDef));
    }

    // actual conversion to new AST node is trivial
    return std::make_shared<AST::TypeDeclaration>(structOrUnion, tag, members);
}

std::shared_ptr<AST::VariableDeclaration>
TypeChecker::typeCheckLocalVarDecl(const std::shared_ptr<AST::VariableDeclaration> &varDecl)
{
    if (Types::isVoidType(varDecl->getVarType()))
        throw std::runtime_error("No void declarations");
    else
        validateType(std::make_shared<Types::DataType>(varDecl->getVarType()));

    if ((!varDecl->getOptStorageClass().has_value() || varDecl->getOptStorageClass().value() != AST::StorageClass::Extern) && !Types::isComplete(varDecl->getVarType(), _typeTable))
    {
        // can't define a variable with an incomplete type
        throw std::runtime_error("Cannot define a variable with an incomplete type");
    }

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
            auto zeroInit = Symbols::Initial(Initializers::zero(varDecl->getVarType(), _typeTable));

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
            auto newInit = typeCheckInit(varDecl->getVarType(), varDecl->getOptInit().value());
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
    else if (auto typeDecl = std::dynamic_pointer_cast<AST::TypeDeclaration>(decl))
        return typeCheckTypeDecl(typeDecl);
    else
        throw std::runtime_error("Internal Error: Unknown type of declaration!");
}

std::shared_ptr<AST::VariableDeclaration>
TypeChecker::typeCheckFileScopeVarDecl(const std::shared_ptr<AST::VariableDeclaration> &varDecl)
{
    if (Types::isVoidType(varDecl->getVarType()))
        throw std::runtime_error("void variables not allowed");
    else
        validateType(std::make_shared<Types::DataType>(varDecl->getVarType()));

    Symbols::InitialValue defaultInit =
        varDecl->getOptStorageClass().has_value() && varDecl->getOptStorageClass().value() == AST::StorageClass::Extern
            ? Symbols::makeNoInitializer()
            : Symbols::makeTentative();

    Symbols::InitialValue staticInit =
        varDecl->getOptInit().has_value()
            ? toStaticInit(varDecl->getVarType(), varDecl->getOptInit().value())
            : defaultInit;

    if (!(Types::isComplete(varDecl->getVarType(), _typeTable) || Symbols::isNoInitializer(staticInit)))
    {
        // note: some compilers permit tentative definition with incomplete type, if it's completed later in the file. we don't.
        throw std::runtime_error("Can't define a variable with an incomplete type");
    }

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
    validateType(std::make_shared<Types::DataType>(funDecl->getFunType()));

    auto _paramTypes = std::vector<std::shared_ptr<Types::DataType>>();
    auto _returnType = std::shared_ptr<Types::DataType>(nullptr);
    auto _funType = std::shared_ptr<Types::FunType>(nullptr);

    if (auto fnType = Types::getFunType(funDecl->getFunType()))
    {
        if (Types::isArrayType(*fnType->retType))
            throw std::runtime_error("A function cannot return an array");
        else
        {
            for (const auto &paramType : fnType->paramTypes)
            {
                if (auto arrType = Types::getArrayType(*paramType))
                    _paramTypes.push_back(std::make_shared<Types::DataType>(Types::makePointerType(arrType->elemType)));
                else if (Types::isVoidType(*paramType))
                    throw std::runtime_error("No void params allowed");
                else
                    _paramTypes.push_back(paramType);
            }
        }

        _returnType = fnType->retType;
        _funType = std::make_shared<Types::FunType>(Types::FunType{_paramTypes, _returnType});
    }
    else
    {
        throw std::runtime_error("Internal error: function has non function type");
    }

    bool hasBody = funDecl->getOptBody().has_value();

    if (hasBody && !(
                       (Types::isVoidType(*_returnType) || Types::isComplete(*_returnType, _typeTable)) &&
                       [&]()
                       {
                           for (const auto &paramType : _paramTypes)
                           {
                               if (!Types::isComplete(*paramType, _typeTable))
                                   return false;
                           }
                           return true;
                       }()))
    {
        throw std::runtime_error("Can't define a function with incomplete return type or parameter type");
    }

    bool global = !funDecl->getOptStorageClass().has_value() || (funDecl->getOptStorageClass().has_value() && funDecl->getOptStorageClass().value() != AST::StorageClass::Static);
    bool alreadyDefined = hasBody;

    if (_symbolTable.exists(funDecl->getName()))
    {
        auto oldDecl = _symbolTable.get(funDecl->getName());

        if (*_funType != Types::getFunType(oldDecl.type).value())
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

    _symbolTable.addFunction(funDecl->getName(), *_funType, alreadyDefined, global);

    if (hasBody)
    {
        for (size_t i{0}; i < _paramTypes.size(); i++)
        {
            auto &param = funDecl->getParams()[i];
            auto &type = _paramTypes[i];
            _symbolTable.addAutomaticVar(param, *type);
        }

        auto newBlock = typeCheckBlock(funDecl->getOptBody().value(), *_returnType);
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
    else if (auto typeDecl = std::dynamic_pointer_cast<AST::TypeDeclaration>(decl))
        return typeCheckTypeDecl(typeDecl);
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
