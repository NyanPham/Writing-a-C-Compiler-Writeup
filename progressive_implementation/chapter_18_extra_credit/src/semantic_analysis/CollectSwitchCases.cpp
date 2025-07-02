#include <optional>
#include <memory>
#include <string>
#include <map>
#include <cstdint>

#include "AST.h"
#include "CollectSwitchCases.h"
#include "UniqueIds.h"
#include "Const.h"
#include "ConstConvert.h"

std::tuple<OptSwitchCtx, std::shared_ptr<AST::Statement>, std::string>
CollectSwitchCases::analyzeCaseOrDefault(std::optional<std::shared_ptr<Constants::Const>> key, OptSwitchCtx optSwitchCtx, const std::string &lbl, const std::shared_ptr<AST::Statement> &innerStmt)
{
    // Make sure we're in a switch statement
    if (!optSwitchCtx.has_value())
        throw std::runtime_error("Found case statement outside of switch");

    auto [switchType, caseMap] = optSwitchCtx.value();

    auto convertedKey = key.has_value() ? std::make_optional(ConstConvert::convert(switchType, key.value())) : std::nullopt;

    // Check for duplicates
    if (caseMap.find(convertedKey) != caseMap.end())
    {
        if (convertedKey.has_value())
        {
            throw std::runtime_error("Duplicate case in switch statement");
        }
        else
        {
            throw std::runtime_error("Duplicate default in switch statement");
        }
    }

    // Generate new ID - lbl should be "case" or "default"
    auto caseId = UniqueIds::makeLabel(lbl);
    caseMap.insert_or_assign(convertedKey, caseId);

    OptSwitchCtx updatedCtx = std::make_optional(std::make_pair(switchType, caseMap));

    // Analyze inner statement
    auto [finalCtx, newInnerStatement] = analyzeStatement(innerStmt, updatedCtx);

    return {
        finalCtx,
        newInnerStatement,
        caseId,
    };
}

std::pair<OptSwitchCtx, std::shared_ptr<AST::Statement>>
CollectSwitchCases::analyzeStatement(const std::shared_ptr<AST::Statement> &stmt, OptSwitchCtx optSwitchCtx)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Default:
    {
        auto defaultStmt = std::dynamic_pointer_cast<AST::Default>(stmt);
        auto [newCtx, newStmt, defaultId] = analyzeCaseOrDefault(std::nullopt, optSwitchCtx, "default", defaultStmt->getBody());
        return {
            newCtx,
            std::make_shared<AST::Default>(newStmt, defaultId),
        };
    }
    case AST::NodeType::Case:
    {
        auto caseStmt = std::dynamic_pointer_cast<AST::Case>(stmt);

        // Get integer value of this case
        if (caseStmt->getValue()->getType() != AST::NodeType::Constant)
            throw std::runtime_error("Non-constant value in case statement");

        auto constNode = std::dynamic_pointer_cast<AST::Constant>(caseStmt->getValue())->getConst();

        // if (auto constInt = Constants::getConstInt(*constNode))
        //     key = constInt->val;
        // else if (auto constLong = Constants::getConstLong(*constNode))
        //     key = constLong->val;
        // else
        //     throw std::runtime_error("Internal error: Bad Constant type");

        auto [newCtx, newStmt, caseId] = analyzeCaseOrDefault(std::make_optional(constNode), optSwitchCtx, "case", caseStmt->getBody());
        return {
            newCtx,
            std::make_shared<AST::Case>(caseStmt->getValue(), newStmt, caseId),
        };
    }
    case AST::NodeType::Switch:
    {
        auto switchStmt = std::dynamic_pointer_cast<AST::Switch>(stmt);

        // Use fresh map when traversing the switch body
        auto switchType = switchStmt->getControl()->getDataType();
        if (!switchType.has_value())
            throw std::runtime_error("Internal error: Ensure the pass TypeChecking is executed before CollectSwitchCases!");

        auto [newCtx, newBody] = analyzeStatement(switchStmt->getBody(), OptSwitchCtx(std::make_pair(switchType.value(), AST::CaseMap{})));

        // Annotate the switch with new case map
        // Do not pass the new case map to the caller
        return {
            optSwitchCtx,
            std::make_shared<AST::Switch>(
                switchStmt->getControl(),
                newBody,
                std::make_optional(newCtx->second),
                switchStmt->getId()),
        };
    }
    // Just pass case map through to substatements
    case AST::NodeType::If:
    {
        auto ifStmt = std::dynamic_pointer_cast<AST::If>(stmt);
        auto [optSwichCtx1, newThenClause] = analyzeStatement(ifStmt->getThenClause(), optSwitchCtx);

        OptSwitchCtx optSwitchCtx2 = optSwichCtx1;
        std::optional<std::shared_ptr<AST::Statement>> newElseClause = ifStmt->getOptElseClause();

        if (newElseClause.has_value())
        {
            auto [newCtx, newStmt] = analyzeStatement(newElseClause.value(), optSwitchCtx2);
            optSwitchCtx2 = newCtx;
            newElseClause = newStmt;
        }

        return {
            optSwitchCtx2,
            std::make_shared<AST::If>(ifStmt->getCondition(), newThenClause, newElseClause),
        };
    }
    case AST::NodeType::Compound:
    {
        auto [newCtx, newBlock] = analyzeBlock(std::dynamic_pointer_cast<AST::Compound>(stmt)->getBlock(), optSwitchCtx);
        return {
            newCtx,
            std::make_shared<AST::Compound>(newBlock),
        };
    }
    case AST::NodeType::While:
    {
        auto whileStmt = std::dynamic_pointer_cast<AST::While>(stmt);
        auto [newCaseMap, newBody] = analyzeStatement(whileStmt->getBody(), optSwitchCtx);

        return {
            newCaseMap,
            std::make_shared<AST::While>(whileStmt->getCondition(), newBody, whileStmt->getId()),
        };
    }
    case AST::NodeType::DoWhile:
    {
        auto doWhileStmt = std::dynamic_pointer_cast<AST::DoWhile>(stmt);
        auto [newCtx, newBody] = analyzeStatement(doWhileStmt->getBody(), optSwitchCtx);

        return {
            newCtx,
            std::make_shared<AST::DoWhile>(newBody, doWhileStmt->getCondition(), doWhileStmt->getId()),
        };
    }
    case AST::NodeType::For:
    {
        auto forStmt = std::dynamic_pointer_cast<AST::For>(stmt);
        auto [newCtx, newBody] = analyzeStatement(forStmt->getBody(), optSwitchCtx);

        return {
            newCtx,
            std::make_shared<AST::For>(forStmt->getInit(), forStmt->getOptCondition(), forStmt->getOptPost(), newBody, forStmt->getId()),
        };
    }
    case AST::NodeType::LabeledStatement:
    {
        auto labeledStmt = std::dynamic_pointer_cast<AST::LabeledStatement>(stmt);
        auto [newCtx, newStmt] = analyzeStatement(labeledStmt->getStatement(), optSwitchCtx);

        return {
            newCtx,
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
            optSwitchCtx,
            stmt,
        };
    default:
        throw std::runtime_error("Internal error: unknown statement type");
    }
}

std::pair<OptSwitchCtx, std::shared_ptr<AST::BlockItem>>
CollectSwitchCases::analyzeBlockItem(const std::shared_ptr<AST::BlockItem> &blkItem, OptSwitchCtx optSwitchCtx)
{
    switch (blkItem->getType())
    {
    case AST::NodeType::FunctionDeclaration:
    case AST::NodeType::VariableDeclaration:
    case AST::NodeType::TypeDeclaration:
        return {
            optSwitchCtx,
            blkItem,
        };
    default:
    {
        auto [newCtx, newStmt] = analyzeStatement(std::dynamic_pointer_cast<AST::Statement>(blkItem), optSwitchCtx);

        return {
            newCtx,
            newStmt,
        };
    }
    }
}

std::pair<OptSwitchCtx, AST::Block>
CollectSwitchCases::analyzeBlock(const AST::Block &blk, OptSwitchCtx optSwitchCtx)
{
    auto newOptSwitchCtx = optSwitchCtx;
    AST::Block newBlock;

    for (const auto &item : blk)
    {
        auto [updatedCtx, newBlockItem] = analyzeBlockItem(item, newOptSwitchCtx);
        newOptSwitchCtx = updatedCtx;
        newBlock.push_back(newBlockItem);
    }

    return {
        newOptSwitchCtx,
        newBlock,
    };
}

std::shared_ptr<AST::FunctionDeclaration>
CollectSwitchCases::analyzeFunctionDeclaration(const std::shared_ptr<AST::FunctionDeclaration> &fnDecl)
{
    if (fnDecl->getOptBody().has_value())
    {
        auto [_, blk] = analyzeBlock(fnDecl->getOptBody().value(), std::nullopt);
        return std::make_shared<AST::FunctionDeclaration>(fnDecl->getName(), fnDecl->getParams(), blk, fnDecl->getFunType(), fnDecl->getOptStorageClass());
    }
    else
    {
        return fnDecl;
    }
}

std::shared_ptr<AST::Declaration>
CollectSwitchCases::analyzeDeclaration(const std::shared_ptr<AST::Declaration> &decl)
{
    if (auto fnDecl = std::dynamic_pointer_cast<AST::FunctionDeclaration>(decl))
        return analyzeFunctionDeclaration(fnDecl);
    else
        return decl;
}

std::shared_ptr<AST::Program>
CollectSwitchCases::analyzeSwitches(const std::shared_ptr<AST::Program> &prog)
{
    std::vector<std::shared_ptr<AST::Declaration>> analyzedDecls;
    analyzedDecls.reserve(prog->getDeclarations().size());

    for (const auto &decl : prog->getDeclarations())
    {
        analyzedDecls.push_back(analyzeDeclaration(decl));
    }

    return std::make_shared<AST::Program>(analyzedDecls);
}
