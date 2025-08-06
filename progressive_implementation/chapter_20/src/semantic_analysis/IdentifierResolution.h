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

/* Struct or Union tag */
struct TagEntry
{
    std::string uniqueTag;
    bool tagFromCurrentScope;
};

using TagMap = std::map<std::string, TagEntry>;

class IdentifierResolution
{
public:
    IdentifierResolution() = default;

    IdMap copyIdentifierMap(const IdMap &idMap);
    TagMap copyTagMap(const TagMap &tagMap);

    Types::DataType resolveType(const Types::DataType &type, TagMap &tagMap);
    std::shared_ptr<AST::Initializer> resolveInitializer(const std::shared_ptr<AST::Initializer> &init, IdMap &idMap, TagMap &tagMap);
    std::shared_ptr<AST::ForInit> resolveForInit(const std::shared_ptr<AST::ForInit> &forInit, IdMap &idMap, TagMap &tagMap);
    std::optional<std::shared_ptr<AST::Expression>> resolveOptionalExp(const std::optional<std::shared_ptr<AST::Expression>> &optExp, IdMap &idMap, TagMap &tagMap);
    std::shared_ptr<AST::Expression> resolveExp(const std::shared_ptr<AST::Expression> &exp, IdMap &idMap, TagMap &tagMap);
    std::shared_ptr<AST::Statement> resolveStatement(const std::shared_ptr<AST::Statement> &stmt, IdMap &idMap, TagMap &tagMap);
    std::string resolveLocalVarHelper(const std::string &name, std::optional<AST::StorageClass> storageClass, IdMap &idMap);
    std::shared_ptr<AST::VariableDeclaration> resolveLocalVarDeclaration(const std::shared_ptr<AST::VariableDeclaration> &decl, IdMap &idMap, TagMap &tagMap);
    std::shared_ptr<AST::BlockItem> resolveBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem, IdMap &idMap, TagMap &tagMap);
    AST::Block resolveBlock(const AST::Block &block, IdMap &idMap, TagMap &tagMap);
    std::shared_ptr<AST::Declaration> resolveLocalDeclaration(const std::shared_ptr<AST::Declaration> decl, IdMap &idMap, TagMap &tagMap);
    std::vector<std::string> resolveParams(const std::vector<std::string> &params, IdMap &idMap);
    std::shared_ptr<AST::TypeDeclaration> resolveTagDeclaration(const std::shared_ptr<AST::TypeDeclaration> &decl, TagMap &tagMap);
    std::shared_ptr<AST::VariableDeclaration> resolveGlobalScopeVariableDeclaration(const std::shared_ptr<AST::VariableDeclaration> &varDecl, IdMap &idMap, TagMap &tagMap);
    std::shared_ptr<AST::FunctionDeclaration> resolveFunDeclaration(const std::shared_ptr<AST::FunctionDeclaration> &decl, IdMap &idMap, TagMap &tagMap);
    std::shared_ptr<AST::Declaration> resolveGlobalDeclaration(const std::shared_ptr<AST::Declaration> &decl, IdMap &idMap, TagMap &tagMap);
    std::shared_ptr<AST::Program> resolve(const std::shared_ptr<AST::Program> &prog);
};

#endif