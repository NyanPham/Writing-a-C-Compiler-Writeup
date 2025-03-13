#include <vector>
#include "TackyGen.h"
#include "Tacky.h"
#include "AST.h"
#include "UniqueIds.h"

TACKY::UnaryOp TackyGen::convertUnop(AST::UnaryOp op)
{
    switch (op)
    {
    case AST::UnaryOp::Complement:
        return TACKY::UnaryOp::Complement;
    case AST::UnaryOp::Negate:
        return TACKY::UnaryOp::Negate;
    case AST::UnaryOp::Not:
        return TACKY::UnaryOp::Not;
    default:
        throw std::invalid_argument("Internal error: Invalid operator");
    }
}

TACKY::BinaryOp TackyGen::convertBinop(AST::BinaryOp op)
{
    switch (op)
    {
    case AST::BinaryOp::Add:
        return TACKY::BinaryOp::Add;
    case AST::BinaryOp::Subtract:
        return TACKY::BinaryOp::Subtract;
    case AST::BinaryOp::Multiply:
        return TACKY::BinaryOp::Multiply;
    case AST::BinaryOp::Divide:
        return TACKY::BinaryOp::Divide;
    case AST::BinaryOp::Remainder:
        return TACKY::BinaryOp::Remainder;
    case AST::BinaryOp::And:
        return TACKY::BinaryOp::And;
    case AST::BinaryOp::Or:
        return TACKY::BinaryOp::Or;
    case AST::BinaryOp::Equal:
        return TACKY::BinaryOp::Equal;
    case AST::BinaryOp::NotEqual:
        return TACKY::BinaryOp::NotEqual;
    case AST::BinaryOp::LessThan:
        return TACKY::BinaryOp::LessThan;
    case AST::BinaryOp::LessOrEqual:
        return TACKY::BinaryOp::LessOrEqual;
    case AST::BinaryOp::GreaterThan:
        return TACKY::BinaryOp::GreaterThan;
    case AST::BinaryOp::GreaterOrEqual:
        return TACKY::BinaryOp::GreaterOrEqual;
    case AST::BinaryOp::BitwiseAnd:
        return TACKY::BinaryOp::BitwiseAnd;
    case AST::BinaryOp::BitwiseOr:
        return TACKY::BinaryOp::BitwiseOr;
    case AST::BinaryOp::BitwiseXor:
        return TACKY::BinaryOp::BitwiseXor;
    case AST::BinaryOp::BitShiftLeft:
        return TACKY::BinaryOp::BitShiftLeft;
    case AST::BinaryOp::BitShiftRight:
        return TACKY::BinaryOp::BitShiftRight;
    default:
        throw std::runtime_error("Internal Error: Invalid Binary operator!");
    }
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitAndExp(const std::shared_ptr<AST::Binary> &binary)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> innerEval{};

    auto [innerEval1, v1] = emitTackyForExp(binary->getExp1());
    auto [innerEval2, v2] = emitTackyForExp(binary->getExp2());

    auto falseLabel = UniqueIds::makeLabel("and_false");
    auto endLabel = UniqueIds::makeLabel("and_end");

    auto dstName = UniqueIds::makeTemporary();
    auto dst = std::make_shared<TACKY::Var>(dstName);

    innerEval.insert(innerEval.end(), innerEval1.begin(), innerEval1.end());
    innerEval.push_back(std::make_shared<TACKY::JumpIfZero>(v1, falseLabel));
    innerEval.insert(innerEval.end(), innerEval2.begin(), innerEval2.end());
    innerEval.push_back(std::make_shared<TACKY::JumpIfZero>(v2, falseLabel));
    innerEval.push_back(std::make_shared<TACKY::Copy>(std::make_shared<TACKY::Constant>(1), dst));
    innerEval.push_back(std::make_shared<TACKY::Jump>(endLabel));
    innerEval.push_back(std::make_shared<TACKY::Label>(falseLabel));
    innerEval.push_back(std::make_shared<TACKY::Copy>(std::make_shared<TACKY::Constant>(0), dst));
    innerEval.push_back(std::make_shared<TACKY::Label>(endLabel));

    return {
        innerEval,
        dst,
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitOrExp(const std::shared_ptr<AST::Binary> &binary)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> innerEval{};

    auto [innerEval1, v1] = emitTackyForExp(binary->getExp1());
    auto [innerEval2, v2] = emitTackyForExp(binary->getExp2());

    auto trueLabel = UniqueIds::makeLabel("or_true");
    auto endLabel = UniqueIds::makeLabel("or_end");

    auto dstName = UniqueIds::makeTemporary();
    auto dst = std::make_shared<TACKY::Var>(dstName);

    innerEval.insert(innerEval.end(), innerEval1.begin(), innerEval1.end());
    innerEval.push_back(std::make_shared<TACKY::JumpIfZero>(v1, trueLabel));
    innerEval.insert(innerEval.end(), innerEval2.begin(), innerEval2.end());
    innerEval.push_back(std::make_shared<TACKY::JumpIfZero>(v2, trueLabel));
    innerEval.push_back(std::make_shared<TACKY::Copy>(std::make_shared<TACKY::Constant>(0), dst));
    innerEval.push_back(std::make_shared<TACKY::Jump>(endLabel));
    innerEval.push_back(std::make_shared<TACKY::Label>(trueLabel));
    innerEval.push_back(std::make_shared<TACKY::Copy>(std::make_shared<TACKY::Constant>(1), dst));
    innerEval.push_back(std::make_shared<TACKY::Label>(endLabel));

    return {
        innerEval,
        dst,
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitBinaryExp(const std::shared_ptr<AST::Binary> &binary)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> innerEval{};

    auto [innerEval1, src1] = emitTackyForExp(binary->getExp1());
    auto [innerEval2, src2] = emitTackyForExp(binary->getExp2());

    innerEval.insert(innerEval.end(), innerEval1.begin(), innerEval1.end());
    innerEval.insert(innerEval.end(), innerEval2.begin(), innerEval2.end());

    auto dstName = UniqueIds::makeTemporary();
    auto dst = std::make_shared<TACKY::Var>(dstName);
    auto op = convertBinop(binary->getOp());

    innerEval.push_back(std::make_shared<TACKY::Binary>(op, src1, src2, dst));

    return {
        innerEval,
        dst,
    };
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

        auto op = convertUnop(unary->getOp());
        auto dstName = UniqueIds::makeTemporary();
        auto dst = std::make_shared<TACKY::Var>(dstName);

        innerEval.push_back(std::make_shared<TACKY::Unary>(op, src, dst));

        return {
            innerEval,
            dst};
    }
    case AST::NodeType::Binary:
    {
        auto binary = std::dynamic_pointer_cast<AST::Binary>(exp);
        switch (binary->getOp())
        {
        case AST::BinaryOp::And:
            return emitAndExp(binary);
        case AST::BinaryOp::Or:
            return emitOrExp(binary);
        default:
            return emitBinaryExp(binary);
        }
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
