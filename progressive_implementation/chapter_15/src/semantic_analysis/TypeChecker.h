#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "AST.h"
#include "Types.h"
#include "Symbols.h"

class TypeChecker
{
public:
    TypeChecker() = default;

    std::shared_ptr<AST::Initializer> typeCheckInit(const Types::DataType &targetType, const std::shared_ptr<AST::Initializer> &init);
    std::shared_ptr<AST::Initializer> makeZeroInit(const Types::DataType &type);
    Symbols::InitialValue toStaticInit(const Types::DataType &varType, const std::shared_ptr<AST::Initializer> &e);
    std::vector<std::shared_ptr<Initializers::StaticInit>> staticInitHelper(const std::shared_ptr<Types::DataType> &varType, const std::shared_ptr<AST::Initializer> &init);
    std::shared_ptr<AST::Expression> typeCheckAndConvert(const std::shared_ptr<AST::Expression> &exp);
    std::shared_ptr<AST::Subscript> typeCheckSubscript(const std::shared_ptr<AST::Subscript> &subscript);
    std::shared_ptr<AST::Cast> typeCheckCast(const std::shared_ptr<AST::Cast> &cast);
    std::shared_ptr<AST::Unary> typeCheckNot(const std::shared_ptr<AST::Unary> &notUnary);
    std::shared_ptr<AST::Unary> typeCheckComplement(const std::shared_ptr<AST::Unary> &complementUnary);
    std::shared_ptr<AST::Unary> typeCheckNegate(const std::shared_ptr<AST::Unary> &negateUnary);
    std::shared_ptr<AST::Unary> typeCheckIncrDecr(const std::shared_ptr<AST::Unary> &incrDecrUnary);
    std::shared_ptr<AST::Binary> typeCheckLogical(const std::shared_ptr<AST::Binary> &logical);
    std::shared_ptr<AST::Binary> typeCheckAddition(const std::shared_ptr<AST::Binary> &addition);
    std::shared_ptr<AST::Binary> typeCheckSubtraction(const std::shared_ptr<AST::Binary> &subtraction);
    std::shared_ptr<AST::Binary> typeCheckMultiplicative(const std::shared_ptr<AST::Binary> &multiplicative);
    std::shared_ptr<AST::Binary> typeCheckEquality(const std::shared_ptr<AST::Binary> &equality);
    std::shared_ptr<AST::Binary> typeCheckComparison(const std::shared_ptr<AST::Binary> &comparison);
    std::shared_ptr<AST::Binary> typeCheckBitwise(const std::shared_ptr<AST::Binary> &bitwise);
    std::shared_ptr<AST::Binary> typeCheckBitShift(const std::shared_ptr<AST::Binary> &bitShift);
    std::shared_ptr<AST::Dereference> typeCheckDereference(const std::shared_ptr<AST::Dereference> &dereference);
    std::shared_ptr<AST::AddrOf> typeCheckAddrOf(const std::shared_ptr<AST::AddrOf> &addrOf);
    std::shared_ptr<AST::Var> typeCheckVar(const std::shared_ptr<AST::Var> &var);
    std::shared_ptr<AST::Constant> typeCheckConstant(const std::shared_ptr<AST::Constant> &c);
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