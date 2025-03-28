#ifndef TACKY_GEN_H
#define TACKY_GEN_H

#include <vector>
#include <utility>
#include <memory>
#include "Tacky.h"
#include "AST.h"

class TackyGen
{
public:
    TackyGen() = default;

    std::string breakLabel(const std::string &id);
    std::string continueLabel(const std::string &id);

    TACKY::UnaryOp convertUnop(AST::UnaryOp op);
    TACKY::BinaryOp convertBinop(AST::BinaryOp op);

    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForDoLoop(const std::shared_ptr<AST::DoWhile> &doLoop);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForWhileLoop(const std::shared_ptr<AST::While> &whileLoop);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForForLoop(const std::shared_ptr<AST::For> &forLoop);

    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForSwitch(const std::shared_ptr<AST::Switch> &switchStmt);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitConditionalExp(const std::shared_ptr<AST::Conditional> &conditional);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitPostfix(const AST::BinaryOp &op, const std::shared_ptr<AST::Var> var);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitCompoundAssignment(const AST::BinaryOp &op, const std::shared_ptr<AST::Var> var, const std::shared_ptr<AST::Expression> rhs);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitAndExp(const std::shared_ptr<AST::Binary> &binary);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitOrExp(const std::shared_ptr<AST::Binary> &binary);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitUnaryExp(const std::shared_ptr<AST::Unary> &unary);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitBinaryExp(const std::shared_ptr<AST::Binary> &binary);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitTackyForExp(const std::shared_ptr<AST::Expression> &exp);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForIfStatement(const std::shared_ptr<AST::If> &ifStmt);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForStatement(const std::shared_ptr<AST::Statement> &stmt);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForDeclaration(const std::shared_ptr<AST::Declaration> &decl);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem);
    std::shared_ptr<TACKY::Function> emitTackyForFunction(const std::shared_ptr<AST::FunctionDefinition> &funDef);
    std::shared_ptr<TACKY::Program> gen(const std::shared_ptr<AST::Program> &prog);
};

#endif