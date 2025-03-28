#ifndef COLLECT_SWITCH_CASES_H
#define COLLECT_SWITCH_CASES_H

#include <optional>
#include <memory>
#include <string>
#include <map>

#include "AST.h"

class CollectSwitchCases
{
public:
    CollectSwitchCases() = default;

    std::tuple<std::optional<AST::CaseMap>, std::shared_ptr<AST::Statement>, std::string> analyzeCaseOrDefault(std::optional<int> key, std::optional<AST::CaseMap> optCaseMap, const std::string &lbl, const std::shared_ptr<AST::Statement> &innerStmt);
    std::pair<std::optional<AST::CaseMap>, std::shared_ptr<AST::Statement>> analyzeStatement(const std::shared_ptr<AST::Statement> &stmt, std::optional<AST::CaseMap> optCaseMap);
    std::pair<std::optional<AST::CaseMap>, std::shared_ptr<AST::BlockItem>> analyzeBlockItem(const std::shared_ptr<AST::BlockItem> &blkItem, std::optional<AST::CaseMap> optCaseMap);
    std::pair<std::optional<AST::CaseMap>, AST::Block> analyzeBlock(const AST::Block &blk, std::optional<AST::CaseMap> optCaseMap);
    std::shared_ptr<AST::FunctionDefinition> analyzeFunctionDef(const std::shared_ptr<AST::FunctionDefinition> &funDef);
    std::shared_ptr<AST::Program> analyzeSwitches(const std::shared_ptr<AST::Program> &prog);
};

#endif