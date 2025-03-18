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

    TACKY::UnaryOp convertUnop(AST::UnaryOp op);
    TACKY::BinaryOp convertBinop(AST::BinaryOp op);

    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitPostfix(const AST::BinaryOp &op, const std::shared_ptr<AST::Var> var);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitCompoundAssignment(const AST::BinaryOp &op, const std::shared_ptr<AST::Var> var, const std::shared_ptr<AST::Expression> rhs);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitAndExp(const std::shared_ptr<AST::Binary> &binary);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitOrExp(const std::shared_ptr<AST::Binary> &binary);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitUnaryExp(const std::shared_ptr<AST::Unary> &unary);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitBinaryExp(const std::shared_ptr<AST::Binary> &binary);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitTackyForExp(const std::shared_ptr<AST::Expression> &exp);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForStatement(const std::shared_ptr<AST::Statement> &stmt);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem);
    std::shared_ptr<TACKY::Function> emitTackyForFunction(const std::shared_ptr<AST::FunctionDefinition> &funDef);
    std::shared_ptr<TACKY::Program> gen(const std::shared_ptr<AST::Program> &prog);
};

#endif