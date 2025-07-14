#ifndef COLLECT_SWITCH_CASES_H
#define COLLECT_SWITCH_CASES_H

#include <optional>
#include <memory>
#include <string>
#include <map>

#include "AST.h"
#include "Types.h"

using OptSwitchCtx = std::optional<std::pair<Types::DataType, AST::CaseMap>>;

class CollectSwitchCases
{
public:
    CollectSwitchCases() = default;

    std::tuple<OptSwitchCtx, std::shared_ptr<AST::Statement>, std::string> analyzeCaseOrDefault(std::optional<std::shared_ptr<Constants::Const>> key, OptSwitchCtx optSwitchCtx, const std::string &lbl, const std::shared_ptr<AST::Statement> &innerStmt);
    std::pair<OptSwitchCtx, std::shared_ptr<AST::Statement>> analyzeStatement(const std::shared_ptr<AST::Statement> &stmt, OptSwitchCtx optSwitchCtx);
    std::pair<OptSwitchCtx, std::shared_ptr<AST::BlockItem>> analyzeBlockItem(const std::shared_ptr<AST::BlockItem> &blkItem, OptSwitchCtx optSwitchCtx);
    std::pair<OptSwitchCtx, AST::Block> analyzeBlock(const AST::Block &blk, OptSwitchCtx optSwitchCtx);
    std::shared_ptr<AST::FunctionDeclaration> analyzeFunctionDeclaration(const std::shared_ptr<AST::FunctionDeclaration> &funDef);
    std::shared_ptr<AST::Declaration> analyzeDeclaration(const std::shared_ptr<AST::Declaration> &decl);
    std::shared_ptr<AST::Program> analyzeSwitches(const std::shared_ptr<AST::Program> &prog);
};

#endif