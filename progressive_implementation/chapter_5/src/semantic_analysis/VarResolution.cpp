#include <optional>
#include <memory>
#include <string>
#include <map>

#include "AST.h"
#include "VarResolution.h"
#include "UniqueIds.h"

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
    case AST::NodeType::Var:
    {
        auto var = std::dynamic_pointer_cast<AST::Var>(exp);

        auto it = varMap.find(var->getName());
        if (it != varMap.end())
        {
            return std::make_shared<AST::Var>(it->second);
        }
        else
        {
            throw std::runtime_error("Undeclared variable: " + var->getName());
        }
    }
    case AST::NodeType::Unary:
    {
        auto unary = std::dynamic_pointer_cast<AST::Unary>(exp);
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
    default:
        throw std::runtime_error("Internal error: Unknown expression!");
    }
}

std::shared_ptr<AST::Statement> VarResolution::resolveStatement(const std::shared_ptr<AST::Statement> &stmt, VarMap &varMap)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Return:
        return std::make_shared<AST::Return>(resolveExp(std::dynamic_pointer_cast<AST::Return>(stmt)->getValue(), varMap));
    case AST::NodeType::ExpressionStmt:
        return std::make_shared<AST::ExpressionStmt>(resolveExp(std::dynamic_pointer_cast<AST::ExpressionStmt>(stmt)->getExp(), varMap));
    case AST::NodeType::Null:
        return stmt;
    default:
        throw std::runtime_error("Internal error: Unknown statement!");
    }
}

std::shared_ptr<AST::Declaration> VarResolution::resolveDeclaration(const std::shared_ptr<AST::Declaration> &decl, VarMap &varMap)
{
    std::optional<std::shared_ptr<AST::Expression>> init = std::nullopt;
    auto it = varMap.find(decl->getName());

    if (it != varMap.end())
    {
        throw std::runtime_error("Duplicate variable declaration: " + decl->getName());
    }

    auto uniqueName{UniqueIds::makeNamedTemporary(decl->getName())};
    varMap.insert({decl->getName(), uniqueName});

    if (decl->getInit().has_value())
    {
        init = std::make_optional(resolveExp(decl->getInit().value(), varMap));
    }

    return std::make_shared<AST::Declaration>(uniqueName, init);
}

std::shared_ptr<AST::BlockItem> VarResolution::resolveBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem, VarMap &varMap)
{
    switch (blockItem->getType())
    {
    case AST::NodeType::Declaration:
        return resolveDeclaration(std::dynamic_pointer_cast<AST::Declaration>(blockItem), varMap);
    default:
        return resolveStatement(std::dynamic_pointer_cast<AST::Statement>(blockItem), varMap);
    }
}

std::shared_ptr<AST::FunctionDefinition> VarResolution::resolveFunctionDef(const std::shared_ptr<AST::FunctionDefinition> &funDef)
{
    VarMap varMap = {};
    std::vector<std::shared_ptr<AST::BlockItem>> resolvedBody = {};

    for (auto &blockItem : funDef->getBody())
    {
        auto resolvedBlockItem = resolveBlockItem(blockItem, varMap);
        resolvedBody.push_back(resolvedBlockItem);
    }

    return std::make_shared<AST::FunctionDefinition>(funDef->getName(), resolvedBody);
}

std::shared_ptr<AST::Program> VarResolution::resolve(const std::shared_ptr<AST::Program> &prog)
{
    return std::make_shared<AST::Program>(resolveFunctionDef(prog->getFunctionDefinition()));
}