#ifndef LOOP_LABELING_H
#define LOOP_LABELING_H

#include <optional>
#include <memory>
#include <string>
#include <map>

#include "AST.h"

class LoopLabeling
{
public:
    LoopLabeling() = default;

    std::shared_ptr<AST::Statement> labelStatement(const std::shared_ptr<AST::Statement> &stmt, std::optional<std::string> currLabel);
    std::shared_ptr<AST::BlockItem> labelBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem, std::optional<std::string> currLabel);
    AST::Block labelBlock(const AST::Block &block, std::optional<std::string> currLabel);
    std::shared_ptr<AST::FunctionDefinition> labelFunctionDef(const std::shared_ptr<AST::FunctionDefinition> &funDef);
    std::shared_ptr<AST::Program> labelLoops(const std::shared_ptr<AST::Program> &prog);
};

#endif