#include <optional>
#include <memory>
#include <string>
#include <set>
#include <functional>

#include "AST.h"
#include "ValidateLabels.h"

std::shared_ptr<AST::Statement>
ValidateLabels::collectLabelsFromStatement(
    std::set<std::string> &definedLabels,
    std::set<std::string> &usedLabels,
    const std::shared_ptr<AST::Statement> &stmt,
    std::function<std::string(const std::string &)> transformLabel)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Goto:
    {
        auto gotoStmt = std::dynamic_pointer_cast<AST::Goto>(stmt);

        usedLabels.insert(gotoStmt->getLabel());
        return std::make_shared<AST::Goto>(transformLabel(gotoStmt->getLabel()));
    }
    case AST::NodeType::LabeledStatement:
    {
        auto labeledStmt = std::dynamic_pointer_cast<AST::LabeledStatement>(stmt);

        if (definedLabels.find(labeledStmt->getLabel()) != definedLabels.end())
        {
            throw std::runtime_error("Duplicate label: " + labeledStmt->getLabel());
        }

        definedLabels.insert(labeledStmt->getLabel());
        auto newStmt = collectLabelsFromStatement(definedLabels, usedLabels, labeledStmt->getStatement(), transformLabel);
        return std::make_shared<AST::LabeledStatement>(transformLabel(labeledStmt->getLabel()), newStmt);
    }
    case AST::NodeType::If:
    {
        auto ifStmt = std::dynamic_pointer_cast<AST::If>(stmt);
        auto newThenClause = collectLabelsFromStatement(definedLabels, usedLabels, ifStmt->getThenClause(), transformLabel);
        auto newElseClause = (std::optional<std::shared_ptr<AST::Statement>>)std::nullopt;

        if (ifStmt->getOptElseClause().has_value())
        {
            newElseClause = std::make_optional(collectLabelsFromStatement(definedLabels, usedLabels, ifStmt->getOptElseClause().value(), transformLabel));
        }

        return std::make_shared<AST::If>(ifStmt->getCondition(), newThenClause, newElseClause);
    }
    case AST::NodeType::Compound:
    {
        auto compoundStmt = std::dynamic_pointer_cast<AST::Compound>(stmt);
        AST::Block newBlk;
        newBlk.reserve(compoundStmt->getBlock().size());

        for (auto &blockItem : compoundStmt->getBlock())
        {
            auto newBlkItem = collectLabelsFromBlockItem(definedLabels, usedLabels, blockItem, transformLabel);
            newBlk.push_back(newBlkItem);
        }

        return std::make_shared<AST::Compound>(newBlk);
    }
    case AST::NodeType::While:
    {
        auto whileStmt = std::dynamic_pointer_cast<AST::While>(stmt);
        auto newBody = collectLabelsFromStatement(definedLabels, usedLabels, whileStmt->getBody(), transformLabel);
        return std::make_shared<AST::While>(whileStmt->getCondition(), newBody, whileStmt->getId());
    }
    case AST::NodeType::DoWhile:
    {
        auto doWhileStmt = std::dynamic_pointer_cast<AST::DoWhile>(stmt);
        auto newBody = collectLabelsFromStatement(definedLabels, usedLabels, doWhileStmt->getBody(), transformLabel);
        return std::make_shared<AST::DoWhile>(newBody, doWhileStmt->getCondition(), doWhileStmt->getId());
    }
    case AST::NodeType::For:
    {
        auto forStmt = std::dynamic_pointer_cast<AST::For>(stmt);
        auto newBody = collectLabelsFromStatement(definedLabels, usedLabels, forStmt->getBody(), transformLabel);
        return std::make_shared<AST::For>(forStmt->getInit(), forStmt->getOptCondition(), forStmt->getOptPost(), newBody, forStmt->getId());
    }
    case AST::NodeType::Switch:
    {
        auto switchStmt = std::dynamic_pointer_cast<AST::Switch>(stmt);
        auto newBody = collectLabelsFromStatement(definedLabels, usedLabels, switchStmt->getBody(), transformLabel);
        return std::make_shared<AST::Switch>(switchStmt->getControl(), newBody, switchStmt->getOptCases(), switchStmt->getId());
    }
    case AST::NodeType::Case:
    {
        auto caseStmt = std::dynamic_pointer_cast<AST::Case>(stmt);
        auto newBody = collectLabelsFromStatement(definedLabels, usedLabels, caseStmt->getBody(), transformLabel);
        return std::make_shared<AST::Case>(caseStmt->getValue(), newBody, caseStmt->getId());
    }
    case AST::NodeType::Default:
    {
        auto defaultStmt = std::dynamic_pointer_cast<AST::Default>(stmt);
        auto newBody = collectLabelsFromStatement(definedLabels, usedLabels, defaultStmt->getBody(), transformLabel);
        return std::make_shared<AST::Default>(newBody, defaultStmt->getId());
    }
    default:
        return stmt;
    }
}

std::shared_ptr<AST::BlockItem>
ValidateLabels::collectLabelsFromBlockItem(
    std::set<std::string> &definedLabels,
    std::set<std::string> &usedLabels,
    const std::shared_ptr<AST::BlockItem> &blockItem,
    std::function<std::string(const std::string &)> transformLabel)
{
    if (blockItem->getType() == AST::NodeType::VariableDeclaration || blockItem->getType() == AST::NodeType::FunctionDeclaration)
        return blockItem;
    else
        return collectLabelsFromStatement(definedLabels, usedLabels, std::dynamic_pointer_cast<AST::Statement>(blockItem), transformLabel);
}

std::shared_ptr<AST::FunctionDeclaration>
ValidateLabels::validateLabelsInFun(const std::shared_ptr<AST::FunctionDeclaration> &fnDecl)
{
    std::set<std::string> definedLabels{};
    std::set<std::string> usedLabels{};

    std::function<std::string(const std::string &)> transformLabel =
        [&fnDecl](const std::string &label)
    { return fnDecl->getName() + "." + label; };

    if (fnDecl->getOptBody().has_value())
    {
        std::vector<std::shared_ptr<AST::BlockItem>> renamedBlock;
        renamedBlock.reserve(fnDecl->getOptBody().value().size());

        for (auto &blockItem : fnDecl->getOptBody().value())
        {
            auto newItem = collectLabelsFromBlockItem(definedLabels, usedLabels, blockItem, transformLabel);
            renamedBlock.push_back(newItem);
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
        return std::make_shared<AST::FunctionDeclaration>(
            fnDecl->getName(),
            fnDecl->getParams(),
            std::make_optional(renamedBlock),
            fnDecl->getFunType(),
            fnDecl->getOptStorageClass());
    }
    else
    {
        return fnDecl;
    }
}

std::shared_ptr<AST::Declaration>
ValidateLabels::validateLabelsInDecl(const std::shared_ptr<AST::Declaration> &decl)
{
    if (auto fnDecl = std::dynamic_pointer_cast<AST::FunctionDeclaration>(decl))
        return validateLabelsInFun(fnDecl);
    else
        return decl;
}

std::shared_ptr<AST::Program>
ValidateLabels::validateLabels(const std::shared_ptr<AST::Program> &prog)
{
    std::vector<std::shared_ptr<AST::Declaration>> validatedDecls;
    validatedDecls.reserve(prog->getDeclarations().size());

    for (const auto &decl : prog->getDeclarations())
    {
        auto newDecl = validateLabelsInDecl(decl);
        validatedDecls.push_back(newDecl);
    }

    return std::make_shared<AST::Program>(validatedDecls);
}
