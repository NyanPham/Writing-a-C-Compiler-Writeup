#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "AST.h"
#include "Types.h"
#include "Symbols.h"

class TypeChecker
{
public:
    TypeChecker() = default;

    std::shared_ptr<AST::Expression> typeCheckExp(const std::shared_ptr<AST::Expression> &exp);
    AST::Block typeCheckBlock(const AST::Block &blk);
    std::shared_ptr<AST::BlockItem> typeCheckBlockItem(const std::shared_ptr<AST::BlockItem> &blkItm);
    std::shared_ptr<AST::Statement> typeCheckStatement(const std::shared_ptr<AST::Statement> &stmt);
    std::shared_ptr<AST::Declaration> typeCheckDeclaration(const std::shared_ptr<AST::Declaration> &decl);
    std::shared_ptr<AST::VariableDeclaration> typeCheckVarDecl(const std::shared_ptr<AST::VariableDeclaration> &varDecl);
    std::shared_ptr<AST::FunctionDeclaration> typeCheckFunDecl(const std::shared_ptr<AST::FunctionDeclaration> &funDecl);
    std::shared_ptr<AST::Program> typeCheck(const std::shared_ptr<AST::Program> &prog);

    Symbols::SymbolTable &getSymbolTable() { return symbolTable; }

private:
    Symbols::SymbolTable symbolTable{};
};

#endif