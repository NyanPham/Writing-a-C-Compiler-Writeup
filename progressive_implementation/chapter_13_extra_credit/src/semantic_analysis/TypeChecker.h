#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "AST.h"
#include "Types.h"
#include "Symbols.h"

class TypeChecker
{
public:
    TypeChecker() = default;

    std::shared_ptr<AST::Var> typeCheckVar(const std::shared_ptr<AST::Var> &var);
    std::shared_ptr<AST::Constant> typeCheckConstant(const std::shared_ptr<AST::Constant> &c);
    std::shared_ptr<AST::Unary> typeCheckUnary(const std::shared_ptr<AST::Unary> &unary);
    std::shared_ptr<AST::Binary> typeCheckBinary(const std::shared_ptr<AST::Binary> &binary);
    std::shared_ptr<AST::Assignment> typeCheckAssignment(const std::shared_ptr<AST::Assignment> &assignment);
    std::shared_ptr<AST::CompoundAssignment> typeCheckCompoundAssignment(const std::shared_ptr<AST::CompoundAssignment> &compoundAssignment);
    std::shared_ptr<AST::PostfixDecr> typeCheckPostfixDecr(const std::shared_ptr<AST::PostfixDecr> &postfixDecr);
    std::shared_ptr<AST::PostfixIncr> typeCheckPostfixIncr(const std::shared_ptr<AST::PostfixIncr> &postfixIncr);
    std::shared_ptr<AST::Conditional> typeCheckConditional(const std::shared_ptr<AST::Conditional> &conditional);
    std::shared_ptr<AST::FunctionCall> typeCheckFunctionCall(const std::shared_ptr<AST::FunctionCall> &funCall);

    std::shared_ptr<AST::Expression> typeCheckExp(const std::shared_ptr<AST::Expression> &exp);
    AST::Block typeCheckBlock(const AST::Block &blk, const Types::DataType &retType);
    std::shared_ptr<AST::BlockItem> typeCheckBlockItem(const std::shared_ptr<AST::BlockItem> &blkItm, const Types::DataType &retType);
    std::shared_ptr<AST::Statement> typeCheckStatement(const std::shared_ptr<AST::Statement> &stmt, const Types::DataType &retType);
    std::shared_ptr<AST::VariableDeclaration> typeCheckLocalVarDecl(const std::shared_ptr<AST::VariableDeclaration> &varDecl);
    std::shared_ptr<AST::Declaration> typeCheckLocalDecl(const std::shared_ptr<AST::Declaration> &decl);
    std::shared_ptr<AST::VariableDeclaration> typeCheckFileScopeVarDecl(const std::shared_ptr<AST::VariableDeclaration> &varDecl);
    std::shared_ptr<AST::FunctionDeclaration> typeCheckFunDecl(const std::shared_ptr<AST::FunctionDeclaration> &funDecl);
    std::shared_ptr<AST::Declaration> typeCheckGlobalDecl(const std::shared_ptr<AST::Declaration> &decl);
    std::shared_ptr<AST::Program> typeCheck(const std::shared_ptr<AST::Program> &prog);

    Symbols::SymbolTable &getSymbolTable() { return _symbolTable; }

private:
    Symbols::SymbolTable _symbolTable{};
};

#endif