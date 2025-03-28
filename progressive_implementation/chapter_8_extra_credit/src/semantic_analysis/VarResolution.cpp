#include <optional>
#include <memory>
#include <string>
#include <map>

#include "AST.h"
#include "VarResolution.h"
#include "UniqueIds.h"

VarMap
VarResolution::copyVariableMap(const VarMap &varMap)
{
    VarMap newVarMap = {};

    for (const auto &entry : varMap)
    {
        newVarMap[entry.first] = {
            entry.second.uniqueName,
            false,
        };
    }

    return newVarMap;
}

std::shared_ptr<AST::ForInit> VarResolution::resolveForInit(const std::shared_ptr<AST::ForInit> &forInit, VarMap &varMap)
{
    switch (forInit->getType())
    {
    case AST::NodeType::InitDecl:
    {
        auto initDecl = std::dynamic_pointer_cast<AST::InitDecl>(forInit);
        return std::make_shared<AST::InitDecl>(resolveDeclaration(initDecl->getDecl(), varMap));
    }
    case AST::NodeType::InitExp:
    {
        auto initExp = std::dynamic_pointer_cast<AST::InitExp>(forInit);
        return std::make_shared<AST::InitExp>(resolveOptionalExp(initExp->getOptExp(), varMap));
    }
    default:
        throw std::runtime_error("Internal Error: Unknown ForInit type!");
    }
}

std::optional<std::shared_ptr<AST::Expression>> VarResolution::resolveOptionalExp(const std::optional<std::shared_ptr<AST::Expression>> &optExp, VarMap &varMap)
{
    if (optExp.has_value())
    {
        return std::make_optional(resolveExp(optExp.value(), varMap));
    }
    else
    {
        return std::nullopt;
    }
}

std::shared_ptr<AST::Expression> VarResolution::resolveExp(const std::shared_ptr<AST::Expression> &exp, VarMap &varMap)
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

        return std::make_shared<AST::Assignment>(resolveExp(assignment->getLeftExp(), varMap), resolveExp(assignment->getRightExp(), varMap));
    }
    case AST::NodeType::CompoundAssignment:
    {
        auto compoundAssignment = std::dynamic_pointer_cast<AST::CompoundAssignment>(exp);

        if (compoundAssignment->getLeftExp()->getType() != AST::NodeType::Var)
        {
            throw std::runtime_error("Invalid lvalue!");
        }

        return std::make_shared<AST::CompoundAssignment>(compoundAssignment->getOp(), resolveExp(compoundAssignment->getLeftExp(), varMap), resolveExp(compoundAssignment->getRightExp(), varMap));
    }
    case AST::NodeType::PostfixIncr:
    {
        auto postfixIncr = std::dynamic_pointer_cast<AST::PostfixIncr>(exp);

        if (postfixIncr->getExp()->getType() != AST::NodeType::Var)
        {
            throw std::runtime_error("Invalid lvalue!");
        }

        return std::make_shared<AST::PostfixIncr>(resolveExp(postfixIncr->getExp(), varMap));
    }
    case AST::NodeType::PostfixDecr:
    {
        auto postfixDecr = std::dynamic_pointer_cast<AST::PostfixDecr>(exp);

        if (postfixDecr->getExp()->getType() != AST::NodeType::Var)
        {
            throw std::runtime_error("Invalid lvalue!");
        }

        return std::make_shared<AST::PostfixDecr>(resolveExp(postfixDecr->getExp(), varMap));
    }
    case AST::NodeType::Var:
    {
        auto var = std::dynamic_pointer_cast<AST::Var>(exp);

        auto it = varMap.find(var->getName());
        if (it != varMap.end())
        {
            return std::make_shared<AST::Var>(it->second.uniqueName);
        }
        else
        {
            throw std::runtime_error("Undeclared variable: " + var->getName());
        }
    }
    case AST::NodeType::Unary:
    {
        auto unary = std::dynamic_pointer_cast<AST::Unary>(exp);

        if ((unary->getOp() == AST::UnaryOp::Incr || unary->getOp() == AST::UnaryOp::Decr) && unary->getExp()->getType() != AST::NodeType::Var)
        {
            throw std::runtime_error("Operand of ++/-- must be an lvalue!");
        }

        return std::make_shared<AST::Unary>(unary->getOp(), resolveExp(unary->getExp(), varMap));
    }
    case AST::NodeType::Binary:
    {
        auto binary = std::dynamic_pointer_cast<AST::Binary>(exp);
        return std::make_shared<AST::Binary>(binary->getOp(), resolveExp(binary->getExp1(), varMap), resolveExp(binary->getExp2(), varMap));
    }
    case AST::NodeType::Constant:
    {
        return exp;
    }
    case AST::NodeType::Conditional:
    {
        auto conditional = std::dynamic_pointer_cast<AST::Conditional>(exp);
        return std::make_shared<AST::Conditional>(resolveExp(conditional->getCondition(), varMap), resolveExp(conditional->getThen(), varMap), resolveExp(conditional->getElse(), varMap));
    }
    default:
        throw std::runtime_error("Internal error: Unknown expression!");
    }
}

std::shared_ptr<AST::Statement>
VarResolution::resolveStatement(const std::shared_ptr<AST::Statement> &stmt, VarMap &varMap)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Return:
        return std::make_shared<AST::Return>(resolveExp(std::dynamic_pointer_cast<AST::Return>(stmt)->getValue(), varMap));
    case AST::NodeType::ExpressionStmt:
        return std::make_shared<AST::ExpressionStmt>(resolveExp(std::dynamic_pointer_cast<AST::ExpressionStmt>(stmt)->getExp(), varMap));
    case AST::NodeType::If:
    {
        auto ifStmt = std::dynamic_pointer_cast<AST::If>(stmt);

        return std::make_shared<AST::If>(
            resolveExp(ifStmt->getCondition(), varMap),
            resolveStatement(ifStmt->getThenClause(), varMap),
            ifStmt->getElseClause().has_value() ? std::make_optional(resolveStatement(ifStmt->getElseClause().value(), varMap)) : std::nullopt);
    }
    case AST::NodeType::Compound:
    {
        auto newVarMap = copyVariableMap(varMap);
        return std::make_shared<AST::Compound>(resolveBlock(std::dynamic_pointer_cast<AST::Compound>(stmt)->getBlock(), newVarMap));
    }
    case AST::NodeType::While:
    {
        auto whileStmt = std::dynamic_pointer_cast<AST::While>(stmt);

        return std::make_shared<AST::While>(
            resolveExp(whileStmt->getCondition(), varMap),
            resolveStatement(whileStmt->getBody(), varMap),
            whileStmt->getId());
    }
    case AST::NodeType::DoWhile:
    {
        auto doWhileStmt = std::dynamic_pointer_cast<AST::DoWhile>(stmt);

        return std::make_shared<AST::DoWhile>(
            resolveStatement(doWhileStmt->getBody(), varMap),
            resolveExp(doWhileStmt->getCondition(), varMap),
            doWhileStmt->getId());
    }
    case AST::NodeType::For:
    {
        auto forStmt = std::dynamic_pointer_cast<AST::For>(stmt);
        auto newVarMap = copyVariableMap(varMap);
        auto resolvedInit = resolveForInit(forStmt->getInit(), newVarMap);

        return std::make_shared<AST::For>(
            resolvedInit,
            resolveOptionalExp(forStmt->getCondition(), newVarMap),
            resolveOptionalExp(forStmt->getPost(), newVarMap),
            resolveStatement(forStmt->getBody(), newVarMap),
            forStmt->getId());
    }
    case AST::NodeType::LabeledStatement:
    {
        auto labeledStmt = std::dynamic_pointer_cast<AST::LabeledStatement>(stmt);
        return std::make_shared<AST::LabeledStatement>(labeledStmt->getLabel(), resolveStatement(labeledStmt->getStatement(), varMap));
    }
    case AST::NodeType::Switch:
    {
        auto switchStmt = std::dynamic_pointer_cast<AST::Switch>(stmt);

        return std::make_shared<AST::Switch>(
            resolveExp(switchStmt->getControl(), varMap),
            resolveStatement(switchStmt->getBody(), varMap),
            switchStmt->getCases(),
            switchStmt->getId());
    }
    case AST::NodeType::Case:
    {
        auto caseStmt = std::dynamic_pointer_cast<AST::Case>(stmt);

        return std::make_shared<AST::Case>(
            resolveExp(caseStmt->getValue(), varMap),
            resolveStatement(caseStmt->getBody(), varMap),
            caseStmt->getId());
    }
    case AST::NodeType::Default:
    {
        auto defaultStmt = std::dynamic_pointer_cast<AST::Default>(stmt);

        return std::make_shared<AST::Default>(
            resolveStatement(defaultStmt->getBody(), varMap),
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

std::shared_ptr<AST::Declaration>
VarResolution::resolveDeclaration(const std::shared_ptr<AST::Declaration> &decl, VarMap &varMap)
{
    std::optional<std::shared_ptr<AST::Expression>> init = std::nullopt;
    auto it = varMap.find(decl->getName());

    if (it != varMap.end() && it->second.fromCurrentBlock)
    {
        throw std::runtime_error("Duplicate variable declaration: " + decl->getName());
    }

    auto uniqueName{UniqueIds::makeNamedTemporary(decl->getName())};

    if (it == varMap.end())
    {
        varMap.insert({decl->getName(), {uniqueName, true}});
    }
    else
    {
        it->second.uniqueName = uniqueName;
        it->second.fromCurrentBlock = true;
    }

    if (decl->getInit().has_value())
    {
        init = std::make_optional(resolveExp(decl->getInit().value(), varMap));
    }

    return std::make_shared<AST::Declaration>(uniqueName, init);
}

std::shared_ptr<AST::BlockItem>
VarResolution::resolveBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem, VarMap &varMap)
{
    switch (blockItem->getType())
    {
    case AST::NodeType::Declaration:
        return resolveDeclaration(std::dynamic_pointer_cast<AST::Declaration>(blockItem), varMap);
    default:
        return resolveStatement(std::dynamic_pointer_cast<AST::Statement>(blockItem), varMap);
    }
}

AST::Block
VarResolution::resolveBlock(const AST::Block &block, VarMap &varMap)
{
    AST::Block resolvedBlock = {};

    for (auto &blockItem : block)
    {
        auto resolvedItem = resolveBlockItem(blockItem, varMap);
        resolvedBlock.push_back(resolvedItem);
    }

    return resolvedBlock;
}

std::shared_ptr<AST::FunctionDefinition>
VarResolution::resolveFunctionDef(const std::shared_ptr<AST::FunctionDefinition> &funDef)
{
    VarMap varMap = {};
    auto resolvedBody = resolveBlock(funDef->getBody(), varMap);

    return std::make_shared<AST::FunctionDefinition>(funDef->getName(), resolvedBody);
}

std::shared_ptr<AST::Program>
VarResolution::resolve(const std::shared_ptr<AST::Program> &prog)
{
    return std::make_shared<AST::Program>(resolveFunctionDef(prog->getFunctionDefinition()));
}