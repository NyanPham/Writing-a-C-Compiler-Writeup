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

struct StructEntry
{
    std::string uniqueTag;
    bool structFromCurrentScope;
};

using StructMap = std::map<std::string, StructEntry>;

class IdentifierResolution
{
public:
    IdentifierResolution() = default;

    IdMap copyIdentifierMap(const IdMap &idMap);
    StructMap copyStructMap(const StructMap &structMap);

    Types::DataType resolveType(const Types::DataType &type, StructMap &structMap);
    std::shared_ptr<AST::Initializer> resolveInitializer(const std::shared_ptr<AST::Initializer> &init, IdMap &idMap, StructMap &structMap);
    std::shared_ptr<AST::ForInit> resolveForInit(const std::shared_ptr<AST::ForInit> &forInit, IdMap &idMap, StructMap &structMap);
    std::optional<std::shared_ptr<AST::Expression>> resolveOptionalExp(const std::optional<std::shared_ptr<AST::Expression>> &optExp, IdMap &idMap, StructMap &structMap);
    std::shared_ptr<AST::Expression> resolveExp(const std::shared_ptr<AST::Expression> &exp, IdMap &idMap, StructMap &structMap);
    std::shared_ptr<AST::Statement> resolveStatement(const std::shared_ptr<AST::Statement> &stmt, IdMap &idMap, StructMap &structMap);
    std::string resolveLocalVarHelper(const std::string &name, std::optional<AST::StorageClass> storageClass, IdMap &idMap);
    std::shared_ptr<AST::VariableDeclaration> resolveLocalVarDeclaration(const std::shared_ptr<AST::VariableDeclaration> &decl, IdMap &idMap, StructMap &structMap);
    std::shared_ptr<AST::BlockItem> resolveBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem, IdMap &idMap, StructMap &structMap);
    AST::Block resolveBlock(const AST::Block &block, IdMap &idMap, StructMap &structMap);
    std::shared_ptr<AST::Declaration> resolveLocalDeclaration(const std::shared_ptr<AST::Declaration> decl, IdMap &idMap, StructMap &structMap);
    std::vector<std::string> resolveParams(const std::vector<std::string> &params, IdMap &idMap);
    std::shared_ptr<AST::StructDeclaration> resolveStructDeclaration(const std::shared_ptr<AST::StructDeclaration> &decl, StructMap &structMap);
    std::shared_ptr<AST::VariableDeclaration> resolveGlobalScopeVariableDeclaration(const std::shared_ptr<AST::VariableDeclaration> &varDecl, IdMap &idMap, StructMap &structMap);
    std::shared_ptr<AST::FunctionDeclaration> resolveFunDeclaration(const std::shared_ptr<AST::FunctionDeclaration> &decl, IdMap &idMap, StructMap &structMap);
    std::shared_ptr<AST::Declaration> resolveGlobalDeclaration(const std::shared_ptr<AST::Declaration> &decl, IdMap &idMap, StructMap &structMap);
    std::shared_ptr<AST::Program> resolve(const std::shared_ptr<AST::Program> &prog);
};

#endif