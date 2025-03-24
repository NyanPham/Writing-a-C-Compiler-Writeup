#ifndef VAR_RESOLUTION_H
#define VAR_RESOLUTION_H

#include <optional>
#include <memory>
#include <string>
#include <map>

#include "AST.h"

struct VarMapEntry
{
    std::string uniqueName;
    bool fromCurrentBlock;
};

using VarMap = std::map<std::string, VarMapEntry>;

class VarResolution
{
public:
    VarResolution() = default;

    VarMap copyVariableMap(const VarMap &varMap);

    std::shared_ptr<AST::ForInit> resolveForInit(const std::shared_ptr<AST::ForInit> &forInit, VarMap &varMap);
    std::optional<std::shared_ptr<AST::Expression>> resolveOptionalExp(const std::optional<std::shared_ptr<AST::Expression>> &optExp, VarMap &varMap);
    std::shared_ptr<AST::Expression> resolveExp(const std::shared_ptr<AST::Expression> &exp, VarMap &varMap);
    std::shared_ptr<AST::Statement> resolveStatement(const std::shared_ptr<AST::Statement> &stmt, VarMap &varMap);
    std::shared_ptr<AST::Declaration> resolveDeclaration(const std::shared_ptr<AST::Declaration> &decl, VarMap &varMap);
    std::shared_ptr<AST::BlockItem> resolveBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem, VarMap &varMap);
    AST::Block resolveBlock(const AST::Block &block, VarMap &varMap);
    std::shared_ptr<AST::FunctionDefinition> resolveFunctionDef(const std::shared_ptr<AST::FunctionDefinition> &funDef);
    std::shared_ptr<AST::Program> resolve(const std::shared_ptr<AST::Program> &prog);
};

#endif