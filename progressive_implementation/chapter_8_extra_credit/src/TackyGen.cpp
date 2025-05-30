#include <vector>
#include <algorithm>
#include <iostream>

#include "TackyGen.h"
#include "Tacky.h"
#include "AST.h"
#include "UniqueIds.h"

std::string TackyGen::breakLabel(const std::string &id)
{
    return "break." + id;
}

std::string TackyGen::continueLabel(const std::string &id)
{
    return "continue." + id;
}

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
    case AST::UnaryOp::Incr:
    case AST::UnaryOp::Decr:
        throw std::runtime_error("Internal error: Should handle ++/-- operator separately!");
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

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForDoLoop(const std::shared_ptr<AST::DoWhile> &doLoop)
{
    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

    auto startLabel = UniqueIds::makeLabel("do_loop_start");
    auto contLabel = continueLabel(doLoop->getId());
    auto brkLabel = breakLabel(doLoop->getId());
    auto bodyInsts = emitTackyForStatement(doLoop->getBody());
    auto [evalCond, c] = emitTackyForExp(doLoop->getCondition());

    insts.push_back(std::make_shared<TACKY::Label>(startLabel));
    insts.insert(insts.end(), bodyInsts.begin(), bodyInsts.end());
    insts.push_back(std::make_shared<TACKY::Label>(contLabel));
    insts.insert(insts.end(), evalCond.begin(), evalCond.end());
    insts.push_back(std::make_shared<TACKY::JumpIfNotZero>(c, startLabel));
    insts.push_back(std::make_shared<TACKY::Label>(brkLabel));

    return insts;
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForWhileLoop(const std::shared_ptr<AST::While> &whileLoop)
{
    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

    auto contLabel = continueLabel(whileLoop->getId());
    auto brkLabel = breakLabel(whileLoop->getId());
    auto [evalCond, c] = emitTackyForExp(whileLoop->getCondition());
    auto bodyInsts = emitTackyForStatement(whileLoop->getBody());

    insts.push_back(std::make_shared<TACKY::Label>(contLabel));
    insts.insert(insts.end(), evalCond.begin(), evalCond.end());
    insts.push_back(std::make_shared<TACKY::JumpIfZero>(c, brkLabel));
    insts.insert(insts.end(), bodyInsts.begin(), bodyInsts.end());
    insts.push_back(std::make_shared<TACKY::Jump>(contLabel));
    insts.push_back(std::make_shared<TACKY::Label>(brkLabel));

    return insts;
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForForLoop(const std::shared_ptr<AST::For> &forLoop)
{
    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

    auto startLabel = UniqueIds::makeLabel("for_start");
    auto contLabel = continueLabel(forLoop->getId());
    auto brkLabel = breakLabel(forLoop->getId());
    auto bodyInsts = emitTackyForStatement(forLoop->getBody());

    auto forInitInsts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    if (auto initDecl = std::dynamic_pointer_cast<AST::InitDecl>(forLoop->getInit()))
    {
        auto innerInsts = emitTackyForDeclaration(initDecl->getDecl());
        forInitInsts.insert(forInitInsts.end(), innerInsts.begin(), innerInsts.end());
    }
    else if (auto initExp = std::dynamic_pointer_cast<AST::InitExp>(forLoop->getInit()))
    {
        if (initExp->hasExp())
        {
            auto [innerInsts, _] = emitTackyForExp(initExp->getExp());
            forInitInsts.insert(forInitInsts.end(), innerInsts.begin(), innerInsts.end());
        }
    }

    auto testCondInsts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    if (forLoop->hasCondition())
    {
        auto [innerInsts, c] = emitTackyForExp(forLoop->getCondition().value());
        testCondInsts.insert(testCondInsts.end(), innerInsts.begin(), innerInsts.end());
        testCondInsts.push_back(std::make_shared<TACKY::JumpIfZero>(c, brkLabel));
    }

    auto postInsts = std::vector<std::shared_ptr<TACKY::Instruction>>{};
    if (forLoop->hasPost())
    {
        auto [innerInsts, _] = emitTackyForExp(forLoop->getPost().value());
        postInsts.insert(postInsts.end(), innerInsts.begin(), innerInsts.end());
    }

    insts.insert(insts.end(), forInitInsts.begin(), forInitInsts.end());
    insts.push_back(std::make_shared<TACKY::Label>(startLabel));
    insts.insert(insts.end(), testCondInsts.begin(), testCondInsts.end());
    insts.insert(insts.end(), bodyInsts.begin(), bodyInsts.end());
    insts.push_back(std::make_shared<TACKY::Label>(contLabel));
    insts.insert(insts.end(), postInsts.begin(), postInsts.end());
    insts.push_back(std::make_shared<TACKY::Jump>(startLabel));
    insts.push_back(std::make_shared<TACKY::Label>(brkLabel));

    return insts;
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitConditionalExp(const std::shared_ptr<AST::Conditional> &conditional)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> insts{};

    auto [evalCond, v] = emitTackyForExp(conditional->getCondition());
    auto [evalV1, v1] = emitTackyForExp(conditional->getThen());
    auto [evalV2, v2] = emitTackyForExp(conditional->getElse());

    auto e2Label = UniqueIds::makeLabel("conditional_else");
    auto endLabel = UniqueIds::makeLabel("conditional_end");
    auto dstName = UniqueIds::makeTemporary();
    auto dst = std::make_shared<TACKY::Var>(dstName);

    insts.insert(insts.end(), evalCond.begin(), evalCond.end());
    insts.push_back(std::make_shared<TACKY::JumpIfZero>(v, e2Label));
    insts.insert(insts.end(), evalV1.begin(), evalV1.end());
    insts.push_back(std::make_shared<TACKY::Copy>(v1, dst));
    insts.push_back(std::make_shared<TACKY::Jump>(endLabel));
    insts.push_back(std::make_shared<TACKY::Label>(e2Label));
    insts.insert(insts.end(), evalV2.begin(), evalV2.end());
    insts.push_back(std::make_shared<TACKY::Copy>(v2, dst));
    insts.push_back(std::make_shared<TACKY::Label>(endLabel));

    return {
        insts,
        dst,
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitPostfix(const AST::BinaryOp &op, const std::shared_ptr<AST::Var> var)
{
    auto dstName = UniqueIds::makeTemporary();
    auto dst = std::make_shared<TACKY::Var>(dstName);

    auto tackyVar = std::make_shared<TACKY::Var>(var->getName());

    std::vector<std::shared_ptr<TACKY::Instruction>> insts{
        std::make_shared<TACKY::Copy>(tackyVar, dst),
        std::make_shared<TACKY::Binary>(convertBinop(op), tackyVar, std::make_shared<TACKY::Constant>(1), tackyVar),
    };

    return {
        insts,
        dst,
    };
}

std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>>
TackyGen::emitCompoundAssignment(const AST::BinaryOp &op, const std::shared_ptr<AST::Var> var, const std::shared_ptr<AST::Expression> rhs)
{
    auto [evalRhs, rhsResult] = emitTackyForExp(rhs);
    auto dst = std::make_shared<TACKY::Var>(var->getName());
    auto tackyOp = convertBinop(op);

    std::vector<std::shared_ptr<TACKY::Instruction>> insts{};

    insts.insert(insts.end(), evalRhs.begin(), evalRhs.end());
    insts.push_back(std::make_shared<TACKY::Binary>(tackyOp, dst, rhsResult, dst));

    return {
        insts,
        dst,
    };
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
TackyGen::emitUnaryExp(const std::shared_ptr<AST::Unary> &unary)
{
    auto [innerEval, src] = emitTackyForExp(unary->getExp());

    auto op = convertUnop(unary->getOp());
    auto dstName = UniqueIds::makeTemporary();
    auto dst = std::make_shared<TACKY::Var>(dstName);

    innerEval.push_back(std::make_shared<TACKY::Unary>(op, src, dst));

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
    case AST::NodeType::Var:
    {
        return {
            {},
            std::make_shared<TACKY::Var>(std::dynamic_pointer_cast<AST::Var>(exp)->getName()),
        };
    }
    case AST::NodeType::Unary:
    {
        auto unary = std::dynamic_pointer_cast<AST::Unary>(exp);

        switch (unary->getOp())
        {
        case AST::UnaryOp::Incr:
        {
            if (unary->getExp()->getType() != AST::NodeType::Var)
            {
                throw std::runtime_error("Bad lvalue!");
            }

            return emitCompoundAssignment(AST::BinaryOp::Add, std::dynamic_pointer_cast<AST::Var>(unary->getExp()), std::make_shared<AST::Constant>(1));
        }

        case AST::UnaryOp::Decr:
        {
            if (unary->getExp()->getType() != AST::NodeType::Var)
            {
                throw std::runtime_error("Bad lvalue!");
            }

            return emitCompoundAssignment(AST::BinaryOp::Subtract, std::dynamic_pointer_cast<AST::Var>(unary->getExp()), std::make_shared<AST::Constant>(1));
        }
        default:
            return emitUnaryExp(unary);
        }
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
    case AST::NodeType::Assignment:
    {
        auto assignment = std::dynamic_pointer_cast<AST::Assignment>(exp);

        if (assignment->getLeftExp()->getType() != AST::NodeType::Var)
        {
            throw std::runtime_error("Bad lvalue!");
        }

        auto [rhsInsts, rhsResult] = emitTackyForExp(assignment->getRightExp());
        auto lhsVar = std::make_shared<TACKY::Var>(std::dynamic_pointer_cast<AST::Var>(assignment->getLeftExp())->getName());
        rhsInsts.push_back(
            std::make_shared<TACKY::Copy>(rhsResult, lhsVar));

        return {
            rhsInsts,
            rhsResult,
        };
    }

    case AST::NodeType::CompoundAssignment:
    {
        auto compoundAssignment = std::dynamic_pointer_cast<AST::CompoundAssignment>(exp);

        if (compoundAssignment->getLeftExp()->getType() != AST::NodeType::Var)
        {
            throw std::runtime_error("Bad lvalue!");
        }

        return emitCompoundAssignment(compoundAssignment->getOp(), std::dynamic_pointer_cast<AST::Var>(compoundAssignment->getLeftExp()), compoundAssignment->getRightExp());
    }
    case AST::NodeType::PostfixIncr:
    {
        auto postfixIncr = std::dynamic_pointer_cast<AST::PostfixIncr>(exp);

        if (postfixIncr->getExp()->getType() != AST::NodeType::Var)
        {
            throw std::runtime_error("Bad lvalue!");
        }

        return emitPostfix(AST::BinaryOp::Add, std::dynamic_pointer_cast<AST::Var>(postfixIncr->getExp()));
    }
    case AST::NodeType::PostfixDecr:
    {
        auto postfixDecr = std::dynamic_pointer_cast<AST::PostfixDecr>(exp);

        if (postfixDecr->getExp()->getType() != AST::NodeType::Var)
        {
            throw std::runtime_error("Bad lvalue!");
        }

        return emitPostfix(AST::BinaryOp::Subtract, std::dynamic_pointer_cast<AST::Var>(postfixDecr->getExp()));
    }
    case AST::NodeType::Conditional:
    {
        return emitConditionalExp(std::dynamic_pointer_cast<AST::Conditional>(exp));
    }
    default:
        throw std::invalid_argument("Internal error: Invalid expression");
    }
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForIfStatement(const std::shared_ptr<AST::If> &ifStmt)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> insts{};

    if (!ifStmt->getElseClause().has_value())
    {
        auto endLabel = UniqueIds::makeLabel("if_end");

        auto [evalCond, c] = emitTackyForExp(ifStmt->getCondition());
        auto evalThen = emitTackyForStatement(ifStmt->getThenClause());

        insts.insert(insts.end(), evalCond.begin(), evalCond.end());
        insts.push_back(std::make_shared<TACKY::JumpIfZero>(c, endLabel));
        insts.insert(insts.end(), evalThen.begin(), evalThen.end());
        insts.push_back(std::make_shared<TACKY::Label>(endLabel));
    }
    else
    {
        auto elseLabel = UniqueIds::makeLabel("if_else");
        auto endLabel = UniqueIds::makeLabel("if_end");

        auto [evalCond, c] = emitTackyForExp(ifStmt->getCondition());
        auto evalThen = emitTackyForStatement(ifStmt->getThenClause());
        auto evalElse = emitTackyForStatement(ifStmt->getElseClause().value());

        insts.insert(insts.end(), evalCond.begin(), evalCond.end());
        insts.push_back(std::make_shared<TACKY::JumpIfZero>(c, elseLabel));
        insts.insert(insts.end(), evalThen.begin(), evalThen.end());
        insts.push_back(std::make_shared<TACKY::Jump>(endLabel));
        insts.push_back(std::make_shared<TACKY::Label>(elseLabel));
        insts.insert(insts.end(), evalElse.begin(), evalElse.end());
        insts.push_back(std::make_shared<TACKY::Label>(endLabel));
    }

    return insts;
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForSwitch(const std::shared_ptr<AST::Switch> &switchStmt)
{
    auto brkLabel = breakLabel(switchStmt->getId());
    auto [evalControl, c] = emitTackyForExp(switchStmt->getControl());
    auto cmpResult = std::make_shared<TACKY::Var>(UniqueIds::makeTemporary());

    if (!switchStmt->getCases().has_value())
    {
        throw std::runtime_error("Switch case map is not defined!");
    }

    auto cases = switchStmt->getCases().value();

    std::vector<std::shared_ptr<TACKY::Instruction>> jumpToCases{};

    std::optional<std::string> defaultCaseId = std::nullopt;
    for (const auto &[key, id] : cases)
    {
        if (key.has_value())
        {
            jumpToCases.push_back(
                std::make_shared<TACKY::Binary>(
                    TACKY::BinaryOp::Equal,
                    std::make_shared<TACKY::Constant>(key.value()),
                    c,
                    cmpResult));

            jumpToCases.push_back(
                std::make_shared<TACKY::JumpIfNotZero>(cmpResult, id));
        }
        else
        {
            // A default case, we'll treat it later below
            defaultCaseId = std::make_optional(id);
        }
    }

    std::shared_ptr<TACKY::Instruction> defaultTacky;

    if (defaultCaseId.has_value())
        defaultTacky = std::make_shared<TACKY::Jump>(defaultCaseId.value());

    auto bodyTacky = emitTackyForStatement(switchStmt->getBody());

    std::vector<std::shared_ptr<TACKY::Instruction>> insts{};

    insts.insert(insts.end(), evalControl.begin(), evalControl.end());
    insts.insert(insts.end(), jumpToCases.begin(), jumpToCases.end());
    if (defaultTacky)
        insts.push_back(defaultTacky);
    insts.push_back(std::make_shared<TACKY::Jump>(brkLabel));
    insts.insert(insts.end(), bodyTacky.begin(), bodyTacky.end());
    insts.push_back(std::make_shared<TACKY::Label>(brkLabel));

    return insts;
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForStatement(const std::shared_ptr<AST::Statement> &stmt)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Return:
    {
        auto [insts, v] = emitTackyForExp(std::dynamic_pointer_cast<AST::Return>(stmt)->getValue());
        insts.push_back(std::make_shared<TACKY::Return>(v));

        return insts;
    }
    case AST::NodeType::ExpressionStmt:
    {
        auto [insts, v] = emitTackyForExp(std::dynamic_pointer_cast<AST::ExpressionStmt>(stmt)->getExp()); // Discard the evaluated v destination, we only care the side effect

        return insts;
    }
    case AST::NodeType::If:
    {
        return emitTackyForIfStatement(std::dynamic_pointer_cast<AST::If>(stmt));
    }
    case AST::NodeType::Compound:
    {
        auto compoundStmt = std::dynamic_pointer_cast<AST::Compound>(stmt);
        auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

        for (auto &blockItem : compoundStmt->getBlock())
        {
            auto innerInsts = emitTackyForBlockItem(blockItem);
            insts.insert(insts.end(), innerInsts.begin(), innerInsts.end());
        }

        return insts;
    }
    case AST::NodeType::LabeledStatement:
    {
        auto labeledStmt = std::dynamic_pointer_cast<AST::LabeledStatement>(stmt);

        auto insts = emitTackyForStatement(labeledStmt->getStatement());
        insts.insert(insts.begin(), std::make_shared<TACKY::Label>(labeledStmt->getLabel()));

        return insts;
    }
    case AST::NodeType::Goto:
    {
        return {
            std::make_shared<TACKY::Jump>(std::dynamic_pointer_cast<AST::Goto>(stmt)->getLabel()),
        };
    }
    case AST::NodeType::Break:
    {
        return {
            std::make_shared<TACKY::Jump>(breakLabel(std::dynamic_pointer_cast<AST::Break>(stmt)->getId())),
        };
    }
    case AST::NodeType::Continue:
    {
        return {
            std::make_shared<TACKY::Jump>(continueLabel(std::dynamic_pointer_cast<AST::Continue>(stmt)->getId())),
        };
    }
    case AST::NodeType::While:
    {
        return emitTackyForWhileLoop(std::dynamic_pointer_cast<AST::While>(stmt));
    }
    case AST::NodeType::DoWhile:
    {
        return emitTackyForDoLoop(std::dynamic_pointer_cast<AST::DoWhile>(stmt));
    }
    case AST::NodeType::For:
    {
        return emitTackyForForLoop(std::dynamic_pointer_cast<AST::For>(stmt));
    }
    case AST::NodeType::Switch:
    {
        return emitTackyForSwitch(std::dynamic_pointer_cast<AST::Switch>(stmt));
    }
    case AST::NodeType::Case:
    {
        auto caseStmt = std::dynamic_pointer_cast<AST::Case>(stmt);
        auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

        insts.push_back(std::make_shared<TACKY::Label>(caseStmt->getId()));
        auto innerInsts = emitTackyForStatement(caseStmt->getBody());
        insts.insert(insts.end(), innerInsts.begin(), innerInsts.end());

        return insts;
    }
    case AST::NodeType::Default:
    {
        auto defaultStmt = std::dynamic_pointer_cast<AST::Default>(stmt);
        auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

        insts.push_back(std::make_shared<TACKY::Label>(defaultStmt->getId()));
        auto innerInsts = emitTackyForStatement(defaultStmt->getBody());
        insts.insert(insts.end(), innerInsts.begin(), innerInsts.end());

        return insts;
    }
    case AST::NodeType::Null:
    {
        return {};
    }
    default:
        throw std::invalid_argument("Internal error: Invalid statement");
    }
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForDeclaration(const std::shared_ptr<AST::Declaration> &decl)
{

    if (decl->getInit().has_value())
    {
        auto [evalAssignment, v] = emitTackyForExp(
            std::make_shared<AST::Assignment>(
                std::make_shared<AST::Var>(decl->getName()),
                decl->getInit().value()));

        return evalAssignment;
    }

    return {};
}

std::vector<std::shared_ptr<TACKY::Instruction>>
TackyGen::emitTackyForBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem)
{
    switch (blockItem->getType())
    {
    case AST::NodeType::Declaration:
    {
        return emitTackyForDeclaration(std::dynamic_pointer_cast<AST::Declaration>(blockItem));
    }
    default:
        return emitTackyForStatement(std::dynamic_pointer_cast<AST::Statement>(blockItem));
    }
}

std::shared_ptr<TACKY::Function>
TackyGen::emitTackyForFunction(const std::shared_ptr<AST::FunctionDefinition> &funDef)
{
    auto insts = std::vector<std::shared_ptr<TACKY::Instruction>>{};

    for (auto &blockItem : funDef->getBody())
    {
        auto innerInsts = emitTackyForBlockItem(blockItem);
        insts.insert(insts.end(), innerInsts.begin(), innerInsts.end());
    }

    auto extraReturn = std::make_shared<TACKY::Return>(std::make_shared<TACKY::Constant>(0));
    insts.push_back(extraReturn);

    return std::make_shared<TACKY::Function>(funDef->getName(), insts);
}

std::shared_ptr<TACKY::Program>
TackyGen::gen(const std::shared_ptr<AST::Program> &prog)
{
    return std::make_shared<TACKY::Program>(emitTackyForFunction(prog->getFunctionDefinition()));
}
