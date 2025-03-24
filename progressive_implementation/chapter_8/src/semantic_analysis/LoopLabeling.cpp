#include <optional>
#include <memory>
#include <string>
#include <map>

#include "AST.h"
#include "LoopLabeling.h"
#include "UniqueIds.h"

std::shared_ptr<AST::Statement>
LoopLabeling::labelStatement(const std::shared_ptr<AST::Statement> &stmt, std::optional<std::string> currLabel)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Break:
    {
        if (!currLabel.has_value())
        {
            throw std::runtime_error("Break outside of loop!");
        }

        return std::make_shared<AST::Break>(currLabel.value());
    }
    case AST::NodeType::Continue:
    {
        if (!currLabel.has_value())
        {
            throw std::runtime_error("Continue outside of loop!");
        }

        return std::make_shared<AST::Continue>(currLabel.value());
    }
    case AST::NodeType::While:
    {
        auto whileStmt = std::dynamic_pointer_cast<AST::While>(stmt);
        auto newId = UniqueIds::makeLabel("while");

        return std::make_shared<AST::While>(whileStmt->getCondition(), labelStatement(whileStmt->getBody(), newId), newId);
    }
    case AST::NodeType::DoWhile:
    {
        auto doWhileStmt = std::dynamic_pointer_cast<AST::DoWhile>(stmt);
        auto newId = UniqueIds::makeLabel("do_while");

        return std::make_shared<AST::DoWhile>(
            labelStatement(doWhileStmt->getBody(), newId),
            doWhileStmt->getCondition(),
            newId);
    }
    case AST::NodeType::For:
    {
        auto forStmt = std::dynamic_pointer_cast<AST::For>(stmt);
        auto newId = UniqueIds::makeLabel("for");

        return std::make_shared<AST::For>(
            forStmt->getInit(),
            forStmt->getCondition(),
            forStmt->getPost(),
            labelStatement(forStmt->getBody(), newId),
            newId);
    }
    case AST::NodeType::Compound:
    {
        auto compoundStmt = std::dynamic_pointer_cast<AST::Compound>(stmt);
        return std::make_shared<AST::Compound>(labelBlock(compoundStmt->getBlock(), currLabel));
    }
    case AST::NodeType::If:
    {
        auto ifStmt = std::dynamic_pointer_cast<AST::If>(stmt);
        return std::make_shared<AST::If>(
            ifStmt->getCondition(),
            labelStatement(ifStmt->getThenClause(), currLabel),
            ifStmt->getElseClause().has_value() ? std::make_optional(labelStatement(ifStmt->getElseClause().value(), currLabel)) : std::nullopt);
    }
    case AST::NodeType::LabeledStatement:
    {
        auto labeledStmt = std::dynamic_pointer_cast<AST::LabeledStatement>(stmt);
        return std::make_shared<AST::LabeledStatement>(
            labeledStmt->getLabel(),
            labelStatement(labeledStmt->getStatement(), currLabel));
    }
    case AST::NodeType::Null:
    case AST::NodeType::Return:
    case AST::NodeType::ExpressionStmt:
    case AST::NodeType::Goto:
        return stmt;
    default:
        throw std::runtime_error("Internal Error: Unknown statement type!");
    }
}

std::shared_ptr<AST::BlockItem>
LoopLabeling::labelBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem, std::optional<std::string> currLabel)
{
    switch (blockItem->getType())
    {
    case AST::NodeType::Declaration:
        return blockItem;
    default:
        return labelStatement(std::dynamic_pointer_cast<AST::Statement>(blockItem), currLabel);
    }
}

AST::Block
LoopLabeling::labelBlock(const AST::Block &block, std::optional<std::string> currLabel)
{
    auto labeledBlock = AST::Block{};

    for (const auto &blockItem : block)
    {
        auto labeledBlockitem = labelBlockItem(blockItem, currLabel);
        labeledBlock.push_back(labeledBlockitem);
    }

    return labeledBlock;
}

std::shared_ptr<AST::FunctionDefinition>
LoopLabeling::labelFunctionDef(const std::shared_ptr<AST::FunctionDefinition> &funDef)
{
    return std::make_shared<AST::FunctionDefinition>(funDef->getName(), labelBlock(funDef->getBody(), std::nullopt));
}

std::shared_ptr<AST::Program>
LoopLabeling::labelLoops(const std::shared_ptr<AST::Program> &prog)
{
    return std::make_shared<AST::Program>(labelFunctionDef(prog->getFunctionDefinition()));
}
