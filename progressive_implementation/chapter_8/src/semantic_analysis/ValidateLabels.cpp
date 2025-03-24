#include <optional>
#include <memory>
#include <string>
#include <set>

#include "AST.h"
#include "ValidateLabels.h"

void ValidateLabels::collectLabelsFromStatement(
    std::set<std::string> &definedLabels,
    std::set<std::string> &usedLabels,
    const std::shared_ptr<AST::Statement> &stmt)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Goto:
    {
        usedLabels.insert(std::dynamic_pointer_cast<AST::Goto>(stmt)->getLabel());
        break;
    }
    case AST::NodeType::LabeledStatement:
    {
        auto labeledStmt = std::dynamic_pointer_cast<AST::LabeledStatement>(stmt);

        if (definedLabels.find(labeledStmt->getLabel()) != definedLabels.end())
        {
            throw std::runtime_error("Duplicate label: " + labeledStmt->getLabel());
        }

        definedLabels.insert(labeledStmt->getLabel());
        collectLabelsFromStatement(definedLabels, usedLabels, labeledStmt->getStatement());
        break;
    }
    case AST::NodeType::If:
    {
        auto ifStmt = std::dynamic_pointer_cast<AST::If>(stmt);
        collectLabelsFromStatement(definedLabels, usedLabels, ifStmt->getThenClause());

        if (ifStmt->getElseClause().has_value())
        {
            collectLabelsFromStatement(definedLabels, usedLabels, ifStmt->getElseClause().value());
        }
        break;
    }
    case AST::NodeType::Compound:
    {
        auto compoundStmt = std::dynamic_pointer_cast<AST::Compound>(stmt);
        for (auto &blockItem : compoundStmt->getBlock())
        {
            collectLabelsFromBlockItem(definedLabels, usedLabels, blockItem);
        }
        break;
    }
    case AST::NodeType::While:
    {
        auto whileStmt = std::dynamic_pointer_cast<AST::While>(stmt);
        collectLabelsFromStatement(definedLabels, usedLabels, whileStmt->getBody());
        break;
    }
    case AST::NodeType::DoWhile:
    {
        auto doWhileStmt = std::dynamic_pointer_cast<AST::DoWhile>(stmt);
        collectLabelsFromStatement(definedLabels, usedLabels, doWhileStmt->getBody());
        break;
    }
    case AST::NodeType::For:
    {
        auto forStmt = std::dynamic_pointer_cast<AST::For>(stmt);
        collectLabelsFromStatement(definedLabels, usedLabels, forStmt->getBody());
        break;
    }
    default:
        break;
    }
}

void ValidateLabels::collectLabelsFromBlockItem(
    std::set<std::string> &definedLabels,
    std::set<std::string> &usedLabels,
    const std::shared_ptr<AST::BlockItem> &blockItem)
{
    if (blockItem->getType() == AST::NodeType::Declaration)
    {
        return;
    }
    else
    {
        collectLabelsFromStatement(definedLabels, usedLabels, std::dynamic_pointer_cast<AST::Statement>(blockItem));
    }
}

void ValidateLabels::validateLabelsInFun(const std::shared_ptr<AST::FunctionDefinition> &funDef)
{
    std::set<std::string> definedLabels{};
    std::set<std::string> usedLabels{};

    for (auto &blockItem : funDef->getBody())
    {
        collectLabelsFromBlockItem(definedLabels, usedLabels, blockItem);
    }

    std::set<std::string> undefinedLabels{};
    for (auto &usedLabel : usedLabels)
    {
        if (definedLabels.find(usedLabel) == definedLabels.end())
        {
            undefinedLabels.insert(usedLabel);
        }
    }

    if (!undefinedLabels.empty())
    {
        std::string errMsg = "Found labels that are used but not defined: ";

        for (auto &label : undefinedLabels)
        {
            errMsg += label + ", ";
        }

        throw std::runtime_error(errMsg);
    }
}

void ValidateLabels::validateLabels(const std::shared_ptr<AST::Program> &prog)
{
    validateLabelsInFun(prog->getFunctionDefinition());
}
