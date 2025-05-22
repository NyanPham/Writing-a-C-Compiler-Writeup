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

std::shared_ptr<AST::ForInit> IdentifierResolution::resolveForInit(const std::shared_ptr<AST::ForInit> &forInit, IdMap &idMap)
{
    switch (forInit->getType())
    {
    case AST::NodeType::InitDecl:
    {
        auto initDecl = std::dynamic_pointer_cast<AST::InitDecl>(forInit);
        return std::make_shared<AST::InitDecl>(resolveLocalVarDeclaration(initDecl->getDecl(), idMap));
    }
    case AST::NodeType::InitExp:
    {
        auto initExp = std::dynamic_pointer_cast<AST::InitExp>(forInit);
        return std::make_shared<AST::InitExp>(resolveOptionalExp(initExp->getOptExp(), idMap));
    }
    default:
        throw std::runtime_error("Internal Error: Unknown ForInit type!");
    }
}

std::optional<std::shared_ptr<AST::Expression>> IdentifierResolution::resolveOptionalExp(const std::optional<std::shared_ptr<AST::Expression>> &optExp, IdMap &idMap)
{
    if (optExp.has_value())
    {
        return std::make_optional(resolveExp(optExp.value(), idMap));
    }
    else
    {
        return std::nullopt;
    }
}

std::shared_ptr<AST::Expression>
IdentifierResolution::resolveExp(const std::shared_ptr<AST::Expression> &exp, IdMap &idMap)
{
    switch (exp->getType())
    {
    case AST::NodeType::Assignment:
    {
        auto assignment = std::dynamic_pointer_cast<AST::Assignment>(exp);

        if (assignment->getLeftExp()->getType() != AST::NodeType::Var)
        {
            throw std::runtime_error("Invalid lvalue!");
        }

        return std::make_shared<AST::Assignment>(
            resolveExp(assignment->getLeftExp(), idMap),
            resolveExp(assignment->getRightExp(), idMap),
            assignment->getDataType());
    }
    case AST::NodeType::CompoundAssignment:
    {
        auto compoundAssignment = std::dynamic_pointer_cast<AST::CompoundAssignment>(exp);

        if (compoundAssignment->getLeftExp()->getType() != AST::NodeType::Var)
        {
            throw std::runtime_error("Invalid lvalue!");
        }

        return std::make_shared<AST::CompoundAssignment>(
            compoundAssignment->getOp(),
            resolveExp(compoundAssignment->getLeftExp(), idMap),
            resolveExp(compoundAssignment->getRightExp(), idMap),
            compoundAssignment->getDataType());
    }
    case AST::NodeType::PostfixIncr:
    {
        auto postfixIncr = std::dynamic_pointer_cast<AST::PostfixIncr>(exp);

        if (postfixIncr->getExp()->getType() != AST::NodeType::Var)
        {
            throw std::runtime_error("Invalid lvalue!");
        }

        return std::make_shared<AST::PostfixIncr>(
            resolveExp(postfixIncr->getExp(), idMap),
            postfixIncr->getDataType());
    }
    case AST::NodeType::PostfixDecr:
    {
        auto postfixDecr = std::dynamic_pointer_cast<AST::PostfixDecr>(exp);

        if (postfixDecr->getExp()->getType() != AST::NodeType::Var)
        {
            throw std::runtime_error("Invalid lvalue!");
        }

        return std::make_shared<AST::PostfixDecr>(
            resolveExp(postfixDecr->getExp(), idMap),
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

        return std::make_shared<AST::Cast>(
            cast->getTargetType(),
            resolveExp(cast->getExp(), idMap),
            cast->getDataType());
    }
    case AST::NodeType::Unary:
    {
        auto unary = std::dynamic_pointer_cast<AST::Unary>(exp);

        if ((unary->getOp() == AST::UnaryOp::Incr || unary->getOp() == AST::UnaryOp::Decr) && unary->getExp()->getType() != AST::NodeType::Var)
        {
            throw std::runtime_error("Operand of ++/-- must be an lvalue!");
        }

        return std::make_shared<AST::Unary>(
            unary->getOp(),
            resolveExp(unary->getExp(), idMap),
            unary->getDataType());
    }
    case AST::NodeType::Binary:
    {
        auto binary = std::dynamic_pointer_cast<AST::Binary>(exp);
        return std::make_shared<AST::Binary>(
            binary->getOp(),
            resolveExp(binary->getExp1(), idMap),
            resolveExp(binary->getExp2(), idMap),
            binary->getDataType());
    }
    case AST::NodeType::Constant:
    {
        return exp;
    }
    case AST::NodeType::Conditional:
    {
        auto conditional = std::dynamic_pointer_cast<AST::Conditional>(exp);
        return std::make_shared<AST::Conditional>(
            resolveExp(conditional->getCondition(), idMap),
            resolveExp(conditional->getThen(), idMap),
            resolveExp(conditional->getElse(), idMap),
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
                newArgs.push_back(resolveExp(arg, idMap));
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
    default:
        throw std::runtime_error("Internal error: Unknown expression!");
    }
}

std::shared_ptr<AST::Statement>
IdentifierResolution::resolveStatement(const std::shared_ptr<AST::Statement> &stmt, IdMap &idMap)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Return:
        return std::make_shared<AST::Return>(resolveExp(std::dynamic_pointer_cast<AST::Return>(stmt)->getValue(), idMap));
    case AST::NodeType::ExpressionStmt:
        return std::make_shared<AST::ExpressionStmt>(resolveExp(std::dynamic_pointer_cast<AST::ExpressionStmt>(stmt)->getExp(), idMap));
    case AST::NodeType::If:
    {
        auto ifStmt = std::dynamic_pointer_cast<AST::If>(stmt);

        return std::make_shared<AST::If>(
            resolveExp(ifStmt->getCondition(), idMap),
            resolveStatement(ifStmt->getThenClause(), idMap),
            ifStmt->getOptElseClause().has_value() ? std::make_optional(resolveStatement(ifStmt->getOptElseClause().value(), idMap)) : std::nullopt);
    }
    case AST::NodeType::Compound:
    {
        auto newIdMap = copyIdentifierMap(idMap);
        return std::make_shared<AST::Compound>(resolveBlock(std::dynamic_pointer_cast<AST::Compound>(stmt)->getBlock(), newIdMap));
    }
    case AST::NodeType::While:
    {
        auto whileStmt = std::dynamic_pointer_cast<AST::While>(stmt);

        return std::make_shared<AST::While>(
            resolveExp(whileStmt->getCondition(), idMap),
            resolveStatement(whileStmt->getBody(), idMap),
            whileStmt->getId());
    }
    case AST::NodeType::DoWhile:
    {
        auto doWhileStmt = std::dynamic_pointer_cast<AST::DoWhile>(stmt);

        return std::make_shared<AST::DoWhile>(
            resolveStatement(doWhileStmt->getBody(), idMap),
            resolveExp(doWhileStmt->getCondition(), idMap),
            doWhileStmt->getId());
    }
    case AST::NodeType::For:
    {
        auto forStmt = std::dynamic_pointer_cast<AST::For>(stmt);
        auto newIdMap = copyIdentifierMap(idMap);
        auto resolvedInit = resolveForInit(forStmt->getInit(), newIdMap);

        return std::make_shared<AST::For>(
            resolvedInit,
            resolveOptionalExp(forStmt->getOptCondition(), newIdMap),
            resolveOptionalExp(forStmt->getOptPost(), newIdMap),
            resolveStatement(forStmt->getBody(), newIdMap),
            forStmt->getId());
    }
    case AST::NodeType::LabeledStatement:
    {
        auto labeledStmt = std::dynamic_pointer_cast<AST::LabeledStatement>(stmt);
        return std::make_shared<AST::LabeledStatement>(labeledStmt->getLabel(), resolveStatement(labeledStmt->getStatement(), idMap));
    }
    case AST::NodeType::Switch:
    {
        auto switchStmt = std::dynamic_pointer_cast<AST::Switch>(stmt);

        return std::make_shared<AST::Switch>(
            resolveExp(switchStmt->getControl(), idMap),
            resolveStatement(switchStmt->getBody(), idMap),
            switchStmt->getOptCases(),
            switchStmt->getId());
    }
    case AST::NodeType::Case:
    {
        auto caseStmt = std::dynamic_pointer_cast<AST::Case>(stmt);

        return std::make_shared<AST::Case>(
            resolveExp(caseStmt->getValue(), idMap),
            resolveStatement(caseStmt->getBody(), idMap),
            caseStmt->getId());
    }
    case AST::NodeType::Default:
    {
        auto defaultStmt = std::dynamic_pointer_cast<AST::Default>(stmt);

        return std::make_shared<AST::Default>(
            resolveStatement(defaultStmt->getBody(), idMap),
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
IdentifierResolution::resolveLocalVarDeclaration(const std::shared_ptr<AST::VariableDeclaration> &varDecl, IdMap &idMap)
{
    auto uniqueName = resolveLocalVarHelper(varDecl->getName(), varDecl->getOptStorageClass(), idMap);
    std::optional<std::shared_ptr<AST::Expression>> resolvedInit = resolveOptionalExp(varDecl->getOptInit(), idMap);

    return std::make_shared<AST::VariableDeclaration>(uniqueName, resolvedInit, varDecl->getVarType(), varDecl->getOptStorageClass());
}

std::shared_ptr<AST::BlockItem>
IdentifierResolution::resolveBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem, IdMap &idMap)
{
    switch (blockItem->getType())
    {
    case AST::NodeType::FunctionDeclaration:
    case AST::NodeType::VariableDeclaration:
        return resolveLocalDeclaration(std::dynamic_pointer_cast<AST::Declaration>(blockItem), idMap);
    default:
        return resolveStatement(std::dynamic_pointer_cast<AST::Statement>(blockItem), idMap);
    }
}

AST::Block
IdentifierResolution::resolveBlock(const AST::Block &block, IdMap &idMap)
{
    AST::Block resolvedBlock = {};

    for (auto &blockItem : block)
    {
        auto resolvedItem = resolveBlockItem(blockItem, idMap);
        resolvedBlock.push_back(resolvedItem);
    }

    return resolvedBlock;
}

std::shared_ptr<AST::Declaration>
IdentifierResolution::resolveLocalDeclaration(const std::shared_ptr<AST::Declaration> decl, IdMap &idMap)
{
    if (auto varDecl = std::dynamic_pointer_cast<AST::VariableDeclaration>(decl))
    {
        return resolveLocalVarDeclaration(varDecl, idMap);
    }
    if (auto fnDecl = std::dynamic_pointer_cast<AST::FunctionDeclaration>(decl))
    {
        if (fnDecl->getOptBody().has_value())
            throw std::runtime_error("Nested function definitions are not allowed!");

        if (fnDecl->getOptStorageClass().has_value() && fnDecl->getOptStorageClass().value() == AST::StorageClass::Static)
            throw std::runtime_error("Static keyword not allowd on local function declarations");

        return resolveFunDeclaration(fnDecl, idMap);
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

std::shared_ptr<AST::VariableDeclaration>
IdentifierResolution::resolveGlobalScopeVariableDeclaration(const std::shared_ptr<AST::VariableDeclaration> &varDecl, IdMap &idMap)
{
    idMap.insert_or_assign(varDecl->getName(), IdMapEntry{
                                                   .uniqueName = varDecl->getName(),
                                                   .fromCurrentScope = true,
                                                   .hasLinkage = true,
                                               });

    return varDecl;
}

std::shared_ptr<AST::FunctionDeclaration>
IdentifierResolution::resolveFunDeclaration(const std::shared_ptr<AST::FunctionDeclaration> &fnDecl, IdMap &idMap)
{
    auto entry = idMap.find(fnDecl->getName());

    if (entry != idMap.end() && entry->second.fromCurrentScope && !entry->second.hasLinkage)
    {
        throw std::runtime_error("Duplicate function declaration: " + fnDecl->getName());
    }

    auto newEntry = IdMapEntry{
        .uniqueName = fnDecl->getName(),
        .fromCurrentScope = true,
        .hasLinkage = true,
    };

    idMap.insert_or_assign(fnDecl->getName(), newEntry);
    auto innerMap = copyIdentifierMap(idMap);
    std::vector<std::string> resolvedParams = resolveParams(fnDecl->getParams(), innerMap);
    std::optional<AST::Block> resolvedBody = std::nullopt;

    if (fnDecl->getOptBody().has_value())
    {
        resolvedBody = std::make_optional(resolveBlock(fnDecl->getOptBody().value(), innerMap));
    }

    return std::make_shared<AST::FunctionDeclaration>(fnDecl->getName(), resolvedParams, resolvedBody, fnDecl->getFunType(), fnDecl->getOptStorageClass());
}

std::shared_ptr<AST::Declaration>
IdentifierResolution::resolveGlobalDeclaration(const std::shared_ptr<AST::Declaration> &decl, IdMap &idMap)
{
    if (auto fnDecl = std::dynamic_pointer_cast<AST::FunctionDeclaration>(decl))
    {
        return resolveFunDeclaration(fnDecl, idMap);
    }
    else if (auto varDecl = std::dynamic_pointer_cast<AST::VariableDeclaration>(decl))
    {
        return resolveGlobalScopeVariableDeclaration(varDecl, idMap);
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
    IdMap idMap;

    for (const auto &decl : prog->getDeclarations())
    {
        resolvedDecls.push_back(resolveGlobalDeclaration(decl, idMap));
    }

    return std::make_shared<AST::Program>(resolvedDecls);
}