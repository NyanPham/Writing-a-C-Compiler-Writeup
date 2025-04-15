#include <optional>
#include <memory>
#include <string>
#include <map>

#include "AST.h"
#include "LoopLabeling.h"
#include "UniqueIds.h"

std::shared_ptr<AST::Statement>
LoopLabeling::labelStatement(const std::shared_ptr<AST::Statement> &stmt, std::optional<std::string> currBreakId, std::optional<std::string> currContinueId)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Break:
    {
        if (!currBreakId.has_value())
        {
            throw std::runtime_error("Break outside of loop!");
        }

        return std::make_shared<AST::Break>(currBreakId.value());
    }
    case AST::NodeType::Continue:
    {
        if (!currContinueId.has_value())
        {
            throw std::runtime_error("Continue outside of loop!");
        }

        return std::make_shared<AST::Continue>(currContinueId.value());
    }
    case AST::NodeType::While:
    {
        auto whileStmt = std::dynamic_pointer_cast<AST::While>(stmt);
        auto newId = UniqueIds::makeLabel("while");

        return std::make_shared<AST::While>(whileStmt->getCondition(), labelStatement(whileStmt->getBody(), newId, newId), newId);
    }
    case AST::NodeType::DoWhile:
    {
        auto doWhileStmt = std::dynamic_pointer_cast<AST::DoWhile>(stmt);
        auto newId = UniqueIds::makeLabel("do_while");

        return std::make_shared<AST::DoWhile>(
            labelStatement(doWhileStmt->getBody(), newId, newId),
            doWhileStmt->getCondition(),
            newId);
    }
    case AST::NodeType::For:
    {
        auto forStmt = std::dynamic_pointer_cast<AST::For>(stmt);
        auto newId = UniqueIds::makeLabel("for");

        return std::make_shared<AST::For>(
            forStmt->getInit(),
            forStmt->getOptCondition(),
            forStmt->getOptPost(),
            labelStatement(forStmt->getBody(), newId, newId),
            newId);
    }
    case AST::NodeType::Compound:
    {
        auto compoundStmt = std::dynamic_pointer_cast<AST::Compound>(stmt);
        return std::make_shared<AST::Compound>(labelBlock(compoundStmt->getBlock(), currBreakId, currContinueId));
    }
    case AST::NodeType::If:
    {
        auto ifStmt = std::dynamic_pointer_cast<AST::If>(stmt);
        return std::make_shared<AST::If>(
            ifStmt->getCondition(),
            labelStatement(ifStmt->getThenClause(), currBreakId, currContinueId),
            ifStmt->getOptElseClause().has_value() ? std::make_optional(labelStatement(ifStmt->getOptElseClause().value(), currBreakId, currContinueId)) : std::nullopt);
    }
    case AST::NodeType::LabeledStatement:
    {
        auto labeledStmt = std::dynamic_pointer_cast<AST::LabeledStatement>(stmt);
        return std::make_shared<AST::LabeledStatement>(
            labeledStmt->getLabel(),
            labelStatement(labeledStmt->getStatement(), currBreakId, currContinueId));
    }
    case AST::NodeType::Switch:
    {
        auto switchStmt = std::dynamic_pointer_cast<AST::Switch>(stmt);
        auto newBreakId = UniqueIds::makeLabel("switch");

        return std::make_shared<AST::Switch>(
            switchStmt->getControl(),
            labelStatement(switchStmt->getBody(), newBreakId, currContinueId),
            switchStmt->getOptCases(),
            newBreakId);
    }
    case AST::NodeType::Case:
    {
        auto caseStmt = std::dynamic_pointer_cast<AST::Case>(stmt);

        return std::make_shared<AST::Case>(
            caseStmt->getValue(),
            labelStatement(caseStmt->getBody(), currBreakId, currContinueId),
            caseStmt->getId());
    }
    case AST::NodeType::Default:
    {
        auto defaultStmt = std::dynamic_pointer_cast<AST::Default>(stmt);

        return std::make_shared<AST::Default>(
            labelStatement(defaultStmt->getBody(), currBreakId, currContinueId),
            defaultStmt->getId());
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
LoopLabeling::labelBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem, std::optional<std::string> currBreakId, std::optional<std::string> currContinueId)
{
    switch (blockItem->getType())
    {
    case AST::NodeType::FunctionDeclaration:
    case AST::NodeType::VariableDeclaration:
        return blockItem;
    default:
        return labelStatement(std::dynamic_pointer_cast<AST::Statement>(blockItem), currBreakId, currContinueId);
    }
}

AST::Block
LoopLabeling::labelBlock(const AST::Block &block, std::optional<std::string> currBreakId, std::optional<std::string> currContinueId)
{
    auto labeledBlock = AST::Block{};

    for (const auto &blockItem : block)
    {
        auto labeledBlockitem = labelBlockItem(blockItem, currBreakId, currContinueId);
        labeledBlock.push_back(labeledBlockitem);
    }

    return labeledBlock;
}

std::shared_ptr<AST::Declaration>
LoopLabeling::labelDeclaration(const std::shared_ptr<AST::Declaration> &decl)
{
    if (auto fnDecl = std::dynamic_pointer_cast<AST::FunctionDeclaration>(decl))
    {
        if (!fnDecl->getOptBody().has_value())
            return fnDecl;

        return std::make_shared<AST::FunctionDeclaration>(fnDecl->getName(), fnDecl->getParams(), labelBlock(fnDecl->getOptBody().value(), std::nullopt, std::nullopt), fnDecl->getOptStorageClass());
    }
    else
    {
        return decl;
    }
}

std::shared_ptr<AST::Program>
LoopLabeling::labelLoops(const std::shared_ptr<AST::Program> &prog)
{
    std::vector<std::shared_ptr<AST::Declaration>> labeledDecls;
    labeledDecls.reserve(prog->getDeclarations().size());

    for (const auto &decl : prog->getDeclarations())
    {
        labeledDecls.push_back(labelDeclaration(decl));
    }

    return std::make_shared<AST::Program>(labeledDecls);
}
