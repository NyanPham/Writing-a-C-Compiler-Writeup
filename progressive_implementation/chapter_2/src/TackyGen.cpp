#include <vector>
#include "TackyGen.h"
#include "Tacky.h"
#include "AST.h"
#include "UniqueIds.h"

TACKY::UnaryOp TackyGen::convertOp(AST::UnaryOp op)
{
    switch (op)
    {
    case AST::UnaryOp::Complement:
        return TACKY::UnaryOp::Complement;
    case AST::UnaryOp::Negate:
        return TACKY::UnaryOp::Negate;
    default:
        throw std::invalid_argument("Internal error: Invalid operator");
    }
}

std::pair<
    std::vector<std::shared_ptr<TACKY::Instruction>>,
    std::shared_ptr<TACKY::Val>>
TackyGen::emitTackyForExp(const std::shared_ptr<AST::Expression> &exp)
{
    switch (exp->getType())
    {
    case AST::NodeType::Constant:
    {
        return {
            {},
            std::make_shared<TACKY::Constant>(std::dynamic_pointer_cast<AST::Constant>(exp)->getValue())};
    }
    case AST::NodeType::Unary:
    {
        auto unary = std::dynamic_pointer_cast<AST::Unary>(exp);
        auto [innerEval, src] = emitTackyForExp(unary->getExp());

        auto op = convertOp(unary->getOp());
        auto dstName = UniqueIds::makeTemporary();
        auto dst = std::make_shared<TACKY::Var>(dstName);

        innerEval.push_back(std::make_shared<TACKY::Unary>(op, src, dst));

        return {
            innerEval,
            dst};
    }
    default:
        throw std::invalid_argument("Internal error: Invalid expression");
    }
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForStatement(const std::shared_ptr<AST::Statement> &stmt)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Return:
    {
        auto [instructions, v] = emitTackyForExp(std::dynamic_pointer_cast<AST::Return>(stmt)->getValue());
        instructions.push_back(std::make_shared<TACKY::Return>(v));

        return instructions;
    }
    default:
        throw std::invalid_argument("Internal error: Invalid statement");
    }
}

std::shared_ptr<TACKY::Function>
TackyGen::emitTackyForFunction(const std::shared_ptr<AST::FunctionDefinition> &funDef)
{
    auto instructions = emitTackyForStatement(funDef->getBody());
    return std::make_shared<TACKY::Function>(funDef->getName(), instructions);
}

std::shared_ptr<TACKY::Program>
TackyGen::gen(const std::shared_ptr<AST::Program> &prog)
{
    return std::make_shared<TACKY::Program>(emitTackyForFunction(prog->getFunctionDefinition()));
}
