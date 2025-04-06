#ifndef IDENTIFIER_RESOLUTION_H
#define IDENTIFIER_RESOLUTION_H

#include <optional>
#include <memory>
#include <string>
#include <map>

#include "AST.h"

struct IdMapEntry
{
    std::string uniqueName;
    bool fromCurrentScope;
    bool hasLinkage;
};

using IdMap = std::map<std::string, IdMapEntry>;

class IdentifierResolution
{
public:
    IdentifierResolution() = default;

    IdMap copyIdentifierMap(const IdMap &idMap);

    std::shared_ptr<AST::ForInit> resolveForInit(const std::shared_ptr<AST::ForInit> &forInit, IdMap &idMap);
    std::optional<std::shared_ptr<AST::Expression>> resolveOptionalExp(const std::optional<std::shared_ptr<AST::Expression>> &optExp, IdMap &idMap);
    std::shared_ptr<AST::Expression> resolveExp(const std::shared_ptr<AST::Expression> &exp, IdMap &idMap);
    std::shared_ptr<AST::Statement> resolveStatement(const std::shared_ptr<AST::Statement> &stmt, IdMap &idMap);
    std::string resolveLocalVarHelper(const std::string &name, IdMap &idMap);
    std::shared_ptr<AST::VariableDeclaration> resolveLocalVarDeclaration(std::shared_ptr<AST::VariableDeclaration> decl, IdMap &idMap);
    std::shared_ptr<AST::BlockItem> resolveBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem, IdMap &idMap);
    AST::Block resolveBlock(const AST::Block &block, IdMap &idMap);
    std::shared_ptr<AST::Declaration> resolveLocalDeclaration(const std::shared_ptr<AST::Declaration> decl, IdMap &idMap);
    std::vector<std::string> resolveParams(const std::vector<std::string> &params, IdMap &idMap);
    std::shared_ptr<AST::FunctionDeclaration> resolveFunDeclaration(const std::shared_ptr<AST::FunctionDeclaration> decl, IdMap &idMap);
    std::shared_ptr<AST::Program> resolve(const std::shared_ptr<AST::Program> &prog);
};

#endif