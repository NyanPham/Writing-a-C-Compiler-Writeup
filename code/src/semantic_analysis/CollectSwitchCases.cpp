#include <optional>
#include <memory>
#include <string>
#include <map>

#include "AST.h"
#include "CollectSwitchCases.h"
#include "UniqueIds.h"

std::tuple<std::optional<AST::CaseMap>, std::shared_ptr<AST::Statement>, std::string>
CollectSwitchCases::analyzeCaseOrDefault(std::optional<int> key, std::optional<AST::CaseMap> optCaseMap, const std::string &lbl, const std::shared_ptr<AST::Statement> &innerStmt)
{
    // Make sure we're in a switch statement
    if (!optCaseMap.has_value())
    {
        throw std::runtime_error("Found case statement outside of switch");
    }

    auto caseMap = optCaseMap.value();

    // Check for duplicates
    if (caseMap.find(key) != caseMap.end())
    {
        if (key.has_value())
        {
            throw std::runtime_error("Duplicate case in switch statement: " + std::to_string(key.value()));
        }
        else
        {
            throw std::runtime_error("Duplicate default in switch statement");
        }
    }

    // Generate new ID - lbl should be "case" or "default"
    auto caseId = UniqueIds::makeLabel(lbl);

    AST::CaseMap newCaseMap{};
    for (const auto &entry : caseMap)
    {
        newCaseMap[entry.first] = entry.second;
    }

    newCaseMap[key] = caseId;

    // Analyze inner statement
    auto [finalMap, newInnerStatement] = analyzeStatement(innerStmt, std::make_optional(newCaseMap));

    return {
        finalMap,
        newInnerStatement,
        caseId,
    };
}

std::pair<std::optional<AST::CaseMap>, std::shared_ptr<AST::Statement>>
CollectSwitchCases::analyzeStatement(const std::shared_ptr<AST::Statement> &stmt, std::optional<AST::CaseMap> optCaseMap)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Default:
    {
        auto defaultStmt = std::dynamic_pointer_cast<AST::Default>(stmt);
        auto [newMap, newStmt, defaultId] = analyzeCaseOrDefault(std::nullopt, optCaseMap, "default", defaultStmt->getBody());
        return {
            newMap,
            std::make_shared<AST::Default>(newStmt, defaultId),
        };
    }
    case AST::NodeType::Case:
    {
        auto caseStmt = std::dynamic_pointer_cast<AST::Case>(stmt);

        // Get integer value of this case
        if (caseStmt->getValue()->getType() != AST::NodeType::Constant)
            throw std::runtime_error("Non-constant value in case statement");

        auto key = std::dynamic_pointer_cast<AST::Constant>(caseStmt->getValue())->getValue();
        auto [newMap, newStmt, caseId] = analyzeCaseOrDefault(std::make_optional(key), optCaseMap, "case", caseStmt->getBody());
        return {
            newMap,
            std::make_shared<AST::Case>(caseStmt->getValue(), newStmt, caseId),
        };
    }
    case AST::NodeType::Switch:
    {
        auto switchStmt = std::dynamic_pointer_cast<AST::Switch>(stmt);

        // Use fresh map when traversing the switch body
        auto [newMap, newBody] = analyzeStatement(switchStmt->getBody(), std::make_optional<AST::CaseMap>());

        // Annotate the switch with new case map
        // Do not pass the new case map to the caller
        return {
            optCaseMap,
            std::make_shared<AST::Switch>(
                switchStmt->getControl(),
                newBody,
                newMap,
                switchStmt->getId()),
        };
    }
    // Just pass case map through to substatements
    case AST::NodeType::If:
    {
        auto ifStmt = std::dynamic_pointer_cast<AST::If>(stmt);
        auto [caseMap1, newThenClause] = analyzeStatement(ifStmt->getThenClause(), optCaseMap);

        std::optional<AST::CaseMap> caseMap2 = caseMap1;
        std::optional<std::shared_ptr<AST::Statement>> newElseClause = ifStmt->getElseClause();

        if (newElseClause.has_value())
        {
            auto [caseMap, newStmt] = analyzeStatement(newElseClause.value(), caseMap1);
            caseMap2 = caseMap;
            newElseClause = newStmt;
        }

        return {
            caseMap2,
            std::make_shared<AST::If>(ifStmt->getCondition(), newThenClause, newElseClause),
        };
    }
    case AST::NodeType::Compound:
    {
        auto [newCaseMap, newBlock] = analyzeBlock(std::dynamic_pointer_cast<AST::Compound>(stmt)->getBlock(), optCaseMap);
        return {
            newCaseMap,
            std::make_shared<AST::Compound>(newBlock),
        };
    }
    case AST::NodeType::While:
    {
        auto whileStmt = std::dynamic_pointer_cast<AST::While>(stmt);
        auto [newCaseMap, newBody] = analyzeStatement(whileStmt->getBody(), optCaseMap);

        return {
            newCaseMap,
            std::make_shared<AST::While>(whileStmt->getCondition(), newBody, whileStmt->getId()),
        };
    }
    case AST::NodeType::DoWhile:
    {
        auto doWhileStmt = std::dynamic_pointer_cast<AST::DoWhile>(stmt);
        auto [newCaseMap, newBody] = analyzeStatement(doWhileStmt->getBody(), optCaseMap);

        return {
            newCaseMap,
            std::make_shared<AST::DoWhile>(newBody, doWhileStmt->getCondition(), doWhileStmt->getId()),
        };
    }
    case AST::NodeType::For:
    {
        auto forStmt = std::dynamic_pointer_cast<AST::For>(stmt);
        auto [newCaseMap, newBody] = analyzeStatement(forStmt->getBody(), optCaseMap);

        return {
            newCaseMap,
            std::make_shared<AST::For>(forStmt->getInit(), forStmt->getCondition(), forStmt->getPost(), newBody, forStmt->getId()),
        };
    }
    case AST::NodeType::LabeledStatement:
    {
        auto labeledStmt = std::dynamic_pointer_cast<AST::LabeledStatement>(stmt);
        auto [newCaseMap, newStmt] = analyzeStatement(labeledStmt->getStatement(), optCaseMap);

        return {
            newCaseMap,
            std::make_shared<AST::LabeledStatement>(labeledStmt->getLabel(), newStmt),
        };
    }
    case AST::NodeType::Return:
    case AST::NodeType::Null:
    case AST::NodeType::ExpressionStmt:
    case AST::NodeType::Break:
    case AST::NodeType::Continue:
    case AST::NodeType::Goto:
        return {
            optCaseMap,
            stmt,
        };
    default:
        throw std::runtime_error("Internal error: unknown statement type");
    }
}

std::pair<std::optional<AST::CaseMap>, std::shared_ptr<AST::BlockItem>>
CollectSwitchCases::analyzeBlockItem(const std::shared_ptr<AST::BlockItem> &blkItem, std::optional<AST::CaseMap> optCaseMap)
{
    switch (blkItem->getType())
    {
    case AST::NodeType::Declaration:
        return {
            optCaseMap,
            blkItem,
        };
    default:
    {
        auto [newCaseMap, newStmt] = analyzeStatement(std::dynamic_pointer_cast<AST::Statement>(blkItem), optCaseMap);

        return {
            newCaseMap,
            newStmt,
        };
    }
    }
}

std::pair<std::optional<AST::CaseMap>, AST::Block>
CollectSwitchCases::analyzeBlock(const AST::Block &blk, std::optional<AST::CaseMap> optCaseMap)
{
    auto newCaseMap = optCaseMap;
    AST::Block newBlock;

    for (const auto &item : blk)
    {
        auto [updatedCaseMap, newBlockItem] = analyzeBlockItem(item, newCaseMap);
        newCaseMap = updatedCaseMap;
        newBlock.push_back(newBlockItem);
    }

    return {
        newCaseMap,
        newBlock,
    };
}

std::shared_ptr<AST::FunctionDefinition>
CollectSwitchCases::analyzeFunctionDef(const std::shared_ptr<AST::FunctionDefinition> &funDef)
{
    auto [_, blk] = analyzeBlock(funDef->getBody(), std::nullopt);

    return std::make_shared<AST::FunctionDefinition>(funDef->getName(), blk);
}

std::shared_ptr<AST::Program>
CollectSwitchCases::analyzeSwitches(const std::shared_ptr<AST::Program> &prog)
{
    return std::make_shared<AST::Program>(analyzeFunctionDef(prog->getFunctionDefinition()));
}
