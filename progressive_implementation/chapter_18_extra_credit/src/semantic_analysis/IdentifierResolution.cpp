#include <optional>
#include <memory>
#include <string>
#include <map>

#include "AST.h"
#include "IdentifierResolution.h"
#include "UniqueIds.h"

IdMap IdentifierResolution::copyIdentifierMap(const IdMap &idMap)
{
    IdMap newIdMap = {};

    for (const auto &entry : idMap)
    {
        newIdMap[entry.first] = {
            entry.second.uniqueName,
            false,
            entry.second.hasLinkage,
        };
    }

    return newIdMap;
}

/*
    Map from user-defined structure/union tags to unique ones
    NOTE: at this stage we don't distinguish between structures and unions
    in the map, or complain if the same tag declares a struct and union in
    the same scope - we'll catch that error during type checking
*/
TagMap IdentifierResolution::copyTagMap(const TagMap &tagMap)
{
    // return a copy of the map with from_current_block set to false for every entry
    TagMap newTagMap = {};

    for (const auto &entry : tagMap)
    {
        newTagMap[entry.first] = {
            entry.second.uniqueTag,
            false,
        };
    }

    return newTagMap;
}

/* Replace structure/union tags in type specifiers */
Types::DataType IdentifierResolution::resolveType(const Types::DataType &type, TagMap &tagMap)
{
    if (auto strctType = Types::getStructType(type))
    {
        auto it = tagMap.find(strctType->tag);
        if (it != tagMap.end())
        {
            auto uniqueTag = it->second.uniqueTag;
            return Types::makeStructType(uniqueTag);
        }
        else
            throw std::runtime_error("specified undeclared structure type");
    }
    else if (auto unionType = Types::getUnionType(type))
    {
        auto it = tagMap.find(unionType->tag);
        if (it != tagMap.end())
        {
            auto uniqueTag = it->second.uniqueTag;
            return Types::makeUnionType(uniqueTag);
        }
        else
            throw std::runtime_error("specified undeclared union type");
    }
    else if (auto ptrType = Types::getPointerType(type))
    {
        return Types::makePointerType(std::make_shared<Types::DataType>(resolveType(*ptrType->referencedType, tagMap)));
    }
    else if (auto arrType = Types::getArrayType(type))
    {
        auto resolvedElemType = resolveType(*arrType->elemType, tagMap);
        return Types::makeArrayType(std::make_shared<Types::DataType>(resolvedElemType), arrType->size);
    }
    else if (auto funType = Types::getFunType(type))
    {
        std::vector<std::shared_ptr<Types::DataType>> resolvedParamTypes{};
        for (auto paramType : funType->paramTypes)
        {
            resolvedParamTypes.push_back(std::make_shared<Types::DataType>(resolveType(*paramType, tagMap)));
        }
        auto resolvedRetType = std::make_shared<Types::DataType>(resolveType(*funType->retType, tagMap));
        return Types::makeFunType(resolvedParamTypes, resolvedRetType);
    }
    else
    {
        return type;
    }
}

std::shared_ptr<AST::Initializer> IdentifierResolution::resolveInitializer(const std::shared_ptr<AST::Initializer> &init, IdMap &idMap, TagMap &tagMap)
{
    if (auto singleInit = std::dynamic_pointer_cast<AST::SingleInit>(init))
    {
        return std::make_shared<AST::SingleInit>(resolveExp(singleInit->getExp(), idMap, tagMap));
    }
    else if (auto compoundInit = std::dynamic_pointer_cast<AST::CompoundInit>(init))
    {
        std::vector<std::shared_ptr<AST::Initializer>> resolvedInits;
        resolvedInits.reserve(compoundInit->getInits().size());
        for (const auto &innerInit : compoundInit->getInits())
        {
            resolvedInits.push_back(resolveInitializer(innerInit, idMap, tagMap));
        }
        return std::make_shared<AST::CompoundInit>(std::move(resolvedInits));
    }
    else
    {
        throw std::runtime_error("Internal Error: Unknown Initializer type!");
    }
}

std::shared_ptr<AST::ForInit> IdentifierResolution::resolveForInit(const std::shared_ptr<AST::ForInit> &forInit, IdMap &idMap, TagMap &tagMap)
{
    switch (forInit->getType())
    {
    case AST::NodeType::InitDecl:
    {
        auto initDecl = std::dynamic_pointer_cast<AST::InitDecl>(forInit);
        return std::make_shared<AST::InitDecl>(resolveLocalVarDeclaration(initDecl->getDecl(), idMap, tagMap));
    }
    case AST::NodeType::InitExp:
    {
        auto initExp = std::dynamic_pointer_cast<AST::InitExp>(forInit);
        return std::make_shared<AST::InitExp>(resolveOptionalExp(initExp->getOptExp(), idMap, tagMap));
    }
    default:
        throw std::runtime_error("Internal Error: Unknown ForInit type!");
    }
}

std::optional<std::shared_ptr<AST::Expression>> IdentifierResolution::resolveOptionalExp(const std::optional<std::shared_ptr<AST::Expression>> &optExp, IdMap &idMap, TagMap &tagMap)
{
    if (optExp.has_value())
    {
        return std::make_optional(resolveExp(optExp.value(), idMap, tagMap));
    }
    else
    {
        return std::nullopt;
    }
}

std::shared_ptr<AST::Expression>
IdentifierResolution::resolveExp(const std::shared_ptr<AST::Expression> &exp, IdMap &idMap, TagMap &tagMap)
{
    switch (exp->getType())
    {
    case AST::NodeType::Assignment:
    {
        auto assignment = std::dynamic_pointer_cast<AST::Assignment>(exp);
        return std::make_shared<AST::Assignment>(
            resolveExp(assignment->getLeftExp(), idMap, tagMap),
            resolveExp(assignment->getRightExp(), idMap, tagMap),
            assignment->getDataType());
    }
    case AST::NodeType::CompoundAssignment:
    {
        auto compoundAssignment = std::dynamic_pointer_cast<AST::CompoundAssignment>(exp);
        return std::make_shared<AST::CompoundAssignment>(
            compoundAssignment->getOp(),
            resolveExp(compoundAssignment->getLeftExp(), idMap, tagMap),
            resolveExp(compoundAssignment->getRightExp(), idMap, tagMap),
            compoundAssignment->getDataType());
    }
    case AST::NodeType::PostfixIncr:
    {
        auto postfixIncr = std::dynamic_pointer_cast<AST::PostfixIncr>(exp);
        return std::make_shared<AST::PostfixIncr>(
            resolveExp(postfixIncr->getExp(), idMap, tagMap),
            postfixIncr->getDataType());
    }
    case AST::NodeType::PostfixDecr:
    {
        auto postfixDecr = std::dynamic_pointer_cast<AST::PostfixDecr>(exp);
        return std::make_shared<AST::PostfixDecr>(
            resolveExp(postfixDecr->getExp(), idMap, tagMap),
            postfixDecr->getDataType());
    }
    case AST::NodeType::Var:
    {
        auto var = std::dynamic_pointer_cast<AST::Var>(exp);

        auto it = idMap.find(var->getName());
        if (it != idMap.end())
        {
            return std::make_shared<AST::Var>(
                it->second.uniqueName,
                var->getDataType());
        }
        else
        {
            throw std::runtime_error("Undeclared variable: " + var->getName());
        }
    }
    case AST::NodeType::Cast:
    {
        auto cast = std::dynamic_pointer_cast<AST::Cast>(exp);

        auto resolvedType = resolveType(cast->getTargetType(), tagMap);
        return std::make_shared<AST::Cast>(
            resolvedType,
            resolveExp(cast->getExp(), idMap, tagMap),
            cast->getDataType());
    }
    case AST::NodeType::Unary:
    {
        auto unary = std::dynamic_pointer_cast<AST::Unary>(exp);
        return std::make_shared<AST::Unary>(
            unary->getOp(),
            resolveExp(unary->getExp(), idMap, tagMap),
            unary->getDataType());
    }
    case AST::NodeType::Binary:
    {
        auto binary = std::dynamic_pointer_cast<AST::Binary>(exp);
        return std::make_shared<AST::Binary>(
            binary->getOp(),
            resolveExp(binary->getExp1(), idMap, tagMap),
            resolveExp(binary->getExp2(), idMap, tagMap),
            binary->getDataType());
    }
    case AST::NodeType::SizeOf:
    {
        auto sizeOf = std::dynamic_pointer_cast<AST::SizeOf>(exp);
        return std::make_shared<AST::SizeOf>(
            resolveExp(sizeOf->getInnerExp(), idMap, tagMap),
            sizeOf->getDataType());
    }
    case AST::NodeType::SizeOfT:
    {
        auto sizeOfT = std::dynamic_pointer_cast<AST::SizeOfT>(exp);
        return std::make_shared<AST::SizeOfT>(
            std::make_shared<Types::DataType>(resolveType(*sizeOfT->getTypeName(), tagMap)),
            sizeOfT->getDataType());
    }
    case AST::NodeType::Constant:
    case AST::NodeType::String:
    {
        return exp;
    }
    case AST::NodeType::Conditional:
    {
        auto conditional = std::dynamic_pointer_cast<AST::Conditional>(exp);
        return std::make_shared<AST::Conditional>(
            resolveExp(conditional->getCondition(), idMap, tagMap),
            resolveExp(conditional->getThen(), idMap, tagMap),
            resolveExp(conditional->getElse(), idMap, tagMap),
            conditional->getDataType());
    }
    case AST::NodeType::FunctionCall:
    {
        auto fnCall = std::dynamic_pointer_cast<AST::FunctionCall>(exp);

        auto fnEntry = idMap.find(fnCall->getName());
        if (fnEntry != idMap.end())
        {
            auto newFnName = fnEntry->second.uniqueName;
            std::vector<std::shared_ptr<AST::Expression>> newArgs;
            newArgs.reserve(fnCall->getArgs().size());
            for (const auto &arg : fnCall->getArgs())
            {
                newArgs.push_back(resolveExp(arg, idMap, tagMap));
            }

            return std::make_shared<AST::FunctionCall>(
                newFnName,
                newArgs,
                fnCall->getDataType());
        }
        else
        {
            throw std::runtime_error("Undeclared function" + fnCall->getName());
        }
    }
    case AST::NodeType::Dereference:
    {
        auto dereference = std::dynamic_pointer_cast<AST::Dereference>(exp);
        return std::make_shared<AST::Dereference>(resolveExp(dereference->getInnerExp(), idMap, tagMap), dereference->getDataType());
    }
    case AST::NodeType::AddrOf:
    {
        auto addrOf = std::dynamic_pointer_cast<AST::AddrOf>(exp);
        return std::make_shared<AST::AddrOf>(resolveExp(addrOf->getInnerExp(), idMap, tagMap), addrOf->getDataType());
    }
    case AST::NodeType::Subscript:
    {
        auto subscript = std::dynamic_pointer_cast<AST::Subscript>(exp);
        return std::make_shared<AST::Subscript>(
            resolveExp(subscript->getExp1(), idMap, tagMap),
            resolveExp(subscript->getExp2(), idMap, tagMap),
            subscript->getDataType());
    }
    case AST::NodeType::Dot:
    {
        auto dot = std::dynamic_pointer_cast<AST::Dot>(exp);
        return std::make_shared<AST::Dot>(
            resolveExp(dot->getStructOrUnion(), idMap, tagMap),
            dot->getMember(),
            dot->getDataType());
    }
    case AST::NodeType::Arrow:
    {
        auto arrow = std::dynamic_pointer_cast<AST::Arrow>(exp);
        return std::make_shared<AST::Arrow>(
            resolveExp(arrow->getStructOrUnion(), idMap, tagMap),
            arrow->getMember(),
            arrow->getDataType());
    }
    default:
        throw std::runtime_error("Internal error: Unknown expression!");
    }
}

std::shared_ptr<AST::Statement>
IdentifierResolution::resolveStatement(const std::shared_ptr<AST::Statement> &stmt, IdMap &idMap, TagMap &tagMap)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Return:
    {
        auto returnStmt = std::dynamic_pointer_cast<AST::Return>(stmt);
        std::optional<std::shared_ptr<AST::Expression>> returnExp = returnStmt->getOptValue();
        if (returnExp.has_value())
            returnExp = std::make_optional(resolveExp(returnExp.value(), idMap, tagMap));

        return std::make_shared<AST::Return>(returnExp);
    }
    case AST::NodeType::ExpressionStmt:
        return std::make_shared<AST::ExpressionStmt>(resolveExp(std::dynamic_pointer_cast<AST::ExpressionStmt>(stmt)->getExp(), idMap, tagMap));
    case AST::NodeType::If:
    {
        auto ifStmt = std::dynamic_pointer_cast<AST::If>(stmt);

        return std::make_shared<AST::If>(
            resolveExp(ifStmt->getCondition(), idMap, tagMap),
            resolveStatement(ifStmt->getThenClause(), idMap, tagMap),
            ifStmt->getOptElseClause().has_value() ? std::make_optional(resolveStatement(ifStmt->getOptElseClause().value(), idMap, tagMap)) : std::nullopt);
    }
    case AST::NodeType::Compound:
    {
        // In a new compound block, create new variable & structure maps to enforce scope.
        auto newIdMap = copyIdentifierMap(idMap);
        auto newTagMap = copyTagMap(tagMap);
        return std::make_shared<AST::Compound>(resolveBlock(std::dynamic_pointer_cast<AST::Compound>(stmt)->getBlock(), newIdMap, newTagMap));
    }
    case AST::NodeType::While:
    {
        auto whileStmt = std::dynamic_pointer_cast<AST::While>(stmt);

        return std::make_shared<AST::While>(
            resolveExp(whileStmt->getCondition(), idMap, tagMap),
            resolveStatement(whileStmt->getBody(), idMap, tagMap),
            whileStmt->getId());
    }
    case AST::NodeType::DoWhile:
    {
        auto doWhileStmt = std::dynamic_pointer_cast<AST::DoWhile>(stmt);

        return std::make_shared<AST::DoWhile>(
            resolveStatement(doWhileStmt->getBody(), idMap, tagMap),
            resolveExp(doWhileStmt->getCondition(), idMap, tagMap),
            doWhileStmt->getId());
    }
    case AST::NodeType::For:
    {
        auto forStmt = std::dynamic_pointer_cast<AST::For>(stmt);

        // Create copy to preserve current scope for 'For'
        auto newIdMap = copyIdentifierMap(idMap);
        auto newTagMap = copyTagMap(tagMap);

        // Resolve initializer: returns an updated id_map along with the resolved initializer.
        auto resolvedInit = resolveForInit(forStmt->getInit(), newIdMap, newTagMap);

        return std::make_shared<AST::For>(
            resolvedInit,
            resolveOptionalExp(forStmt->getOptCondition(), newIdMap, newTagMap),
            resolveOptionalExp(forStmt->getOptPost(), newIdMap, newTagMap),
            resolveStatement(forStmt->getBody(), newIdMap, newTagMap),
            forStmt->getId());
    }
    case AST::NodeType::LabeledStatement:
    {
        auto labeledStmt = std::dynamic_pointer_cast<AST::LabeledStatement>(stmt);
        return std::make_shared<AST::LabeledStatement>(labeledStmt->getLabel(), resolveStatement(labeledStmt->getStatement(), idMap, tagMap));
    }
    case AST::NodeType::Switch:
    {
        auto switchStmt = std::dynamic_pointer_cast<AST::Switch>(stmt);

        return std::make_shared<AST::Switch>(
            resolveExp(switchStmt->getControl(), idMap, tagMap),
            resolveStatement(switchStmt->getBody(), idMap, tagMap),
            switchStmt->getOptCases(),
            switchStmt->getId());
    }
    case AST::NodeType::Case:
    {
        auto caseStmt = std::dynamic_pointer_cast<AST::Case>(stmt);

        return std::make_shared<AST::Case>(
            resolveExp(caseStmt->getValue(), idMap, tagMap),
            resolveStatement(caseStmt->getBody(), idMap, tagMap),
            caseStmt->getId());
    }
    case AST::NodeType::Default:
    {
        auto defaultStmt = std::dynamic_pointer_cast<AST::Default>(stmt);

        return std::make_shared<AST::Default>(
            resolveStatement(defaultStmt->getBody(), idMap, tagMap),
            defaultStmt->getId());
    }
    case AST::NodeType::Goto:
    case AST::NodeType::Null:
    case AST::NodeType::Break:
    case AST::NodeType::Continue:
        return stmt;
    default:
        throw std::runtime_error("Internal error: Unknown statement!");
    }
}

// Helper to resolve local variables; deals with validation and updating variable map
std::string
IdentifierResolution::resolveLocalVarHelper(const std::string &name, std::optional<AST::StorageClass> optStorageClass, IdMap &idMap)
{
    auto it = idMap.find(name);
    if (it != idMap.end() && it->second.fromCurrentScope)
    {
        auto entry = it->second;
        if (!(entry.hasLinkage && optStorageClass.has_value() && optStorageClass.value() == AST::StorageClass::Extern))
            throw std::runtime_error("Duplicate variable declaration: " + name); // Variable is present in the map and was defined in the current scope
    }

    if (optStorageClass.has_value() && optStorageClass.value() == AST::StorageClass::Extern)
    {
        idMap.insert_or_assign(name, IdMapEntry{
                                         .uniqueName = name,
                                         .fromCurrentScope = true,
                                         .hasLinkage = true,
                                     });

        return name;
    }

    auto uniqueName = UniqueIds::makeNamedTemporary(name);
    idMap.insert_or_assign(name, IdMapEntry{
                                     .uniqueName = uniqueName,
                                     .fromCurrentScope = true,
                                     .hasLinkage = false,
                                 });

    return uniqueName;
}

std::shared_ptr<AST::VariableDeclaration>
IdentifierResolution::resolveLocalVarDeclaration(const std::shared_ptr<AST::VariableDeclaration> &varDecl, IdMap &idMap, TagMap &tagMap)
{
    auto uniqueName = resolveLocalVarHelper(varDecl->getName(), varDecl->getOptStorageClass(), idMap);
    auto resolvedType = resolveType(varDecl->getVarType(), tagMap);
    std::optional<std::shared_ptr<AST::Initializer>> resolvedInit = varDecl->getOptInit().has_value()
                                                                        ? std::make_optional(resolveInitializer(varDecl->getOptInit().value(), idMap, tagMap))
                                                                        : std::nullopt;

    return std::make_shared<AST::VariableDeclaration>(uniqueName, resolvedInit, resolvedType, varDecl->getOptStorageClass());
}

std::shared_ptr<AST::BlockItem>
IdentifierResolution::resolveBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem, IdMap &idMap, TagMap &tagMap)
{
    switch (blockItem->getType())
    {
    case AST::NodeType::FunctionDeclaration:
    case AST::NodeType::VariableDeclaration:
    case AST::NodeType::TypeDeclaration:
        // resolving a declaration can change the structure or variable map
        return resolveLocalDeclaration(std::dynamic_pointer_cast<AST::Declaration>(blockItem), idMap, tagMap);
    default:
        // resolving a statement doesn't change the struct or variable map
        return resolveStatement(std::dynamic_pointer_cast<AST::Statement>(blockItem), idMap, tagMap);
    }
}

AST::Block
IdentifierResolution::resolveBlock(const AST::Block &block, IdMap &idMap, TagMap &tagMap)
{
    AST::Block resolvedBlock = {};

    for (auto &blockItem : block)
    {
        auto resolvedItem = resolveBlockItem(blockItem, idMap, tagMap);
        resolvedBlock.push_back(resolvedItem);
    }

    return resolvedBlock;
}

std::shared_ptr<AST::Declaration>
IdentifierResolution::resolveLocalDeclaration(const std::shared_ptr<AST::Declaration> decl, IdMap &idMap, TagMap &tagMap)
{
    if (auto varDecl = std::dynamic_pointer_cast<AST::VariableDeclaration>(decl))
    {
        return resolveLocalVarDeclaration(varDecl, idMap, tagMap);
    }
    if (auto fnDecl = std::dynamic_pointer_cast<AST::FunctionDeclaration>(decl))
    {
        if (fnDecl->getOptBody().has_value())
            throw std::runtime_error("Nested function definitions are not allowed!");

        if (fnDecl->getOptStorageClass().has_value() && fnDecl->getOptStorageClass().value() == AST::StorageClass::Static)
            throw std::runtime_error("Static keyword not allowd on local function declarations");

        return resolveFunDeclaration(fnDecl, idMap, tagMap);
    }
    if (auto typeDecl = std::dynamic_pointer_cast<AST::TypeDeclaration>(decl))
    {
        return resolveTagDeclaration(typeDecl, tagMap);
    }
    throw std::runtime_error("Internal error: Unknown declaration!");
}

std::vector<std::string>
IdentifierResolution::resolveParams(const std::vector<std::string> &params, IdMap &idMap)
{
    std::vector<std::string> resolvedParams;
    resolvedParams.reserve(params.size());

    for (auto &param : params)
    {
        resolvedParams.push_back(resolveLocalVarHelper(param, std::nullopt, idMap));
    }
    return resolvedParams;
}

std::shared_ptr<AST::TypeDeclaration>
IdentifierResolution::resolveTagDeclaration(const std::shared_ptr<AST::TypeDeclaration> &decl, TagMap &tagMap)
{
    auto it = tagMap.find(decl->getTag());
    std::string resolvedTag;

    if (it != tagMap.end() && it->second.tagFromCurrentScope)
    {
        // this refers to the same struct we've already declared, don't update the map
        resolvedTag = it->second.uniqueTag;
    }
    else
    {
        // this declare a new type, generate a tag and update the map
        resolvedTag = UniqueIds::makeNamedTemporary(decl->getTag());
        auto newEntry = TagEntry{
            .uniqueTag = resolvedTag,
            .tagFromCurrentScope = true};
        tagMap.insert_or_assign(decl->getTag(), newEntry);
    }

    // note that we need to use new tag map here in case member type is derived from this type
    std::vector<std::shared_ptr<AST::MemberDeclaration>> resolvedMembers{};
    for (auto member : decl->getMembers())
    {
        auto memberType = std::make_shared<Types::DataType>(resolveType(*member->getMemberType(), tagMap));
        auto resolvedMember = std::make_shared<AST::MemberDeclaration>(member->getMemberName(), memberType);
        resolvedMembers.push_back(resolvedMember);
    }

    return std::make_shared<AST::TypeDeclaration>(decl->getStructOrUnion(), resolvedTag, resolvedMembers);
}

std::shared_ptr<AST::VariableDeclaration>
IdentifierResolution::resolveGlobalScopeVariableDeclaration(const std::shared_ptr<AST::VariableDeclaration> &varDecl, IdMap &idMap, TagMap &tagMap)
{
    auto resolvedVarType = resolveType(varDecl->getVarType(), tagMap);
    idMap.insert_or_assign(varDecl->getName(), IdMapEntry{
                                                   .uniqueName = varDecl->getName(),
                                                   .fromCurrentScope = true,
                                                   .hasLinkage = true,
                                               });

    return std::make_shared<AST::VariableDeclaration>(varDecl->getName(), varDecl->getOptInit(), resolvedVarType, varDecl->getOptStorageClass());
}

std::shared_ptr<AST::FunctionDeclaration>
IdentifierResolution::resolveFunDeclaration(const std::shared_ptr<AST::FunctionDeclaration> &fnDecl, IdMap &idMap, TagMap &tagMap)
{
    auto entry = idMap.find(fnDecl->getName());

    if (entry != idMap.end() && entry->second.fromCurrentScope && !entry->second.hasLinkage)
    {
        throw std::runtime_error("Duplicate function declaration: " + fnDecl->getName());
    }

    auto resolvedType = resolveType(fnDecl->getFunType(), tagMap);
    auto newEntry = IdMapEntry{
        .uniqueName = fnDecl->getName(),
        .fromCurrentScope = true,
        .hasLinkage = true,
    };

    idMap.insert_or_assign(fnDecl->getName(), newEntry);
    auto innerMap = copyIdentifierMap(idMap);
    std::vector<std::string> resolvedParams = resolveParams(fnDecl->getParams(), innerMap);
    auto innerTagMap = copyTagMap(tagMap);
    std::optional<AST::Block> resolvedBody = std::nullopt;

    if (fnDecl->getOptBody().has_value())
    {
        resolvedBody = std::make_optional(resolveBlock(fnDecl->getOptBody().value(), innerMap, innerTagMap));
    }

    return std::make_shared<AST::FunctionDeclaration>(fnDecl->getName(), resolvedParams, resolvedBody, resolvedType, fnDecl->getOptStorageClass());
}

std::shared_ptr<AST::Declaration>
IdentifierResolution::resolveGlobalDeclaration(const std::shared_ptr<AST::Declaration> &decl, IdMap &idMap, TagMap &tagMap)
{
    if (auto fnDecl = std::dynamic_pointer_cast<AST::FunctionDeclaration>(decl))
    {
        return resolveFunDeclaration(fnDecl, idMap, tagMap);
    }
    else if (auto varDecl = std::dynamic_pointer_cast<AST::VariableDeclaration>(decl))
    {
        return resolveGlobalScopeVariableDeclaration(varDecl, idMap, tagMap);
    }
    else if (auto typeDecl = std::dynamic_pointer_cast<AST::TypeDeclaration>(decl))
    {
        return resolveTagDeclaration(typeDecl, tagMap);
    }
    else
    {
        throw std::runtime_error("Internal error: Unknown declaration!");
    }
}

std::shared_ptr<AST::Program>
IdentifierResolution::resolve(const std::shared_ptr<AST::Program> &prog)
{
    std::vector<std::shared_ptr<AST::Declaration>> resolvedDecls{};
    resolvedDecls.reserve(prog->getDeclarations().size());
    IdMap idMap{};
    TagMap strctMap{};

    for (const auto &decl : prog->getDeclarations())
    {
        resolvedDecls.push_back(resolveGlobalDeclaration(decl, idMap, strctMap));
    }

    return std::make_shared<AST::Program>(resolvedDecls);
}