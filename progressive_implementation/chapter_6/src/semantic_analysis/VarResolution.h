#ifndef VAR_RESOLUTION_H
#define VAR_RESOLUTION_H

#include <optional>
#include <memory>
#include <string>
#include <map>

#include "AST.h"

using VarMap = std::map<std::string, std::string>;

class VarResolution
{
public:
    VarResolution() = default;

    std::shared_ptr<AST::Expression> resolveExp(const std::shared_ptr<AST::Expression> &exp, VarMap &varMap);
    std::shared_ptr<AST::Statement> resolveStatement(const std::shared_ptr<AST::Statement> &stmt, VarMap &varMap);
    std::shared_ptr<AST::Declaration> resolveDeclaration(const std::shared_ptr<AST::Declaration> &decl, VarMap &varMap);
    std::shared_ptr<AST::BlockItem> resolveBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem, VarMap &varMap);
    std::shared_ptr<AST::FunctionDefinition> resolveFunctionDef(const std::shared_ptr<AST::FunctionDefinition> &funDef);
    std::shared_ptr<AST::Program> resolve(const std::shared_ptr<AST::Program> &prog);
};

#endif