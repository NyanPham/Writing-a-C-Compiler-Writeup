#ifndef AST_PRETTY_PRINT_H
#define AST_PRETTY_PRINT_H

#include <iostream>
#include <string>
#include <memory>
#include "./../AST.h"

class ASTPrettyPrint
{
public:
    ASTPrettyPrint() : indentLevel(0) {}

    void print(const AST::Program &program)
    {
        visit(program);
    }

private:
    int indentLevel;

    void increaseIndent() { indentLevel++; }
    void decreaseIndent() { indentLevel--; }
    std::string getIndent() const { return std::string(indentLevel * 4, ' '); }

    void visit(const AST::Node &node, bool indent = true)
    {
        switch (node.getType())
        {
        case AST::NodeType::Program:
            visitProgram(static_cast<const AST::Program &>(node));
            break;
        case AST::NodeType::FunctionDefinition:
            visitFunctionDefinition(static_cast<const AST::FunctionDefinition &>(node));
            break;
        case AST::NodeType::Declaration:
            visitDeclaration(static_cast<const AST::Declaration &>(node), indent);
            break;
        case AST::NodeType::Return:
            visitReturn(static_cast<const AST::Return &>(node), indent);
            break;
        case AST::NodeType::Constant:
            visitConstant(static_cast<const AST::Constant &>(node), indent);
            break;
        case AST::NodeType::Unary:
            visitUnary(static_cast<const AST::Unary &>(node), indent);
            break;
        case AST::NodeType::Binary:
            visitBinary(static_cast<const AST::Binary &>(node), indent);
            break;
        case AST::NodeType::Var:
            visitVar(static_cast<const AST::Var &>(node), indent);
            break;
        case AST::NodeType::Assignment:
            visitAssignment(static_cast<const AST::Assignment &>(node), indent);
            break;
        case AST::NodeType::CompoundAssignment:
            visitCompoundAssignment(static_cast<const AST::CompoundAssignment &>(node), indent);
            break;
        case AST::NodeType::PostfixIncr:
            visitPostfixIncr(static_cast<const AST::PostfixIncr &>(node), indent);
            break;
        case AST::NodeType::PostfixDecr:
            visitPostfixDecr(static_cast<const AST::PostfixDecr &>(node), indent);
            break;
        case AST::NodeType::Conditional:
            visitConditional(static_cast<const AST::Conditional &>(node), indent);
            break;
        case AST::NodeType::ExpressionStmt:
            visitExpressionStmt(static_cast<const AST::ExpressionStmt &>(node), indent);
            break;
        case AST::NodeType::If:
            visitIf(static_cast<const AST::If &>(node), indent);
            break;
        case AST::NodeType::Compound:
            visitCompound(static_cast<const AST::Compound &>(node), indent);
            break;
        case AST::NodeType::Break:
            visitBreak(static_cast<const AST::Break &>(node), indent);
            break;
        case AST::NodeType::Continue:
            visitContinue(static_cast<const AST::Continue &>(node), indent);
            break;
        case AST::NodeType::While:
            visitWhile(static_cast<const AST::While &>(node), indent);
            break;
        case AST::NodeType::DoWhile:
            visitDoWhile(static_cast<const AST::DoWhile &>(node), indent);
            break;
        case AST::NodeType::For:
            visitFor(static_cast<const AST::For &>(node), indent);
            break;
        case AST::NodeType::Null:
            visitNull(static_cast<const AST::Null &>(node), indent);
            break;
        case AST::NodeType::LabeledStatement:
            visitLabeledStatement(static_cast<const AST::LabeledStatement &>(node), indent);
            break;
        case AST::NodeType::Goto:
            visitGoto(static_cast<const AST::Goto &>(node), indent);
            break;
        case AST::NodeType::Switch:
            visitSwitch(static_cast<const AST::Switch &>(node), indent);
            break;
        case AST::NodeType::Case:
            visitCase(static_cast<const AST::Case &>(node), indent);
            break;
        case AST::NodeType::Default:
            visitDefault(static_cast<const AST::Default &>(node), indent);
            break;
        default:
            std::cerr << "Unknown node type" << std::endl;
            break;
        }
    }

    void visitProgram(const AST::Program &program)
    {
        std::cout << getIndent() << "Program(\n";
        increaseIndent();
        visit(*program.getFunctionDefinition());
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitFunctionDefinition(const AST::FunctionDefinition &funcDef)
    {
        std::cout << getIndent() << "Function(\n";
        increaseIndent();
        std::cout << getIndent() << "name=\"" << funcDef.getName() << "\",\n";
        std::cout << getIndent() << "body=\n";
        increaseIndent();
        for (const auto &item : funcDef.getBody())
        {
            visit(*item);
        }
        decreaseIndent();
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitReturn(const AST::Return &ret, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Return(\n";
        increaseIndent();
        visit(*ret.getValue());
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitConstant(const AST::Constant &constant, bool indent)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Constant(" << constant.getValue() << ")\n";
    }

    void visitUnary(const AST::Unary &unary, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Unary(\n";
        increaseIndent();
        std::cout << getIndent() << "op=";
        switch (unary.getOp())
        {
        case AST::UnaryOp::Complement:
            std::cout << "Complement";
            break;
        case AST::UnaryOp::Negate:
            std::cout << "Negate";
            break;
        case AST::UnaryOp::Not:
            std::cout << "Not";
            break;
        case AST::UnaryOp::Incr:
            std::cout << "Incr";
            break;
        case AST::UnaryOp::Decr:
            std::cout << "Decr";
            break;
        default:
            break;
        }
        std::cout << ",\n";
        std::cout << getIndent() << "exp=";
        visit(*unary.getExp(), false);
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitBinary(const AST::Binary &binary, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Binary(\n";
        increaseIndent();
        std::cout << getIndent() << "op=";
        switch (binary.getOp())
        {
        case AST::BinaryOp::Add:
            std::cout << "Add";
            break;
        case AST::BinaryOp::Subtract:
            std::cout << "Subtract";
            break;
        case AST::BinaryOp::Multiply:
            std::cout << "Multiply";
            break;
        case AST::BinaryOp::Divide:
            std::cout << "Divide";
            break;
        case AST::BinaryOp::Remainder:
            std::cout << "Remainder";
            break;
        case AST::BinaryOp::And:
            std::cout << "And";
            break;
        case AST::BinaryOp::Or:
            std::cout << "Or";
            break;
        case AST::BinaryOp::Equal:
            std::cout << "Equal";
            break;
        case AST::BinaryOp::NotEqual:
            std::cout << "NotEqual";
            break;
        case AST::BinaryOp::LessThan:
            std::cout << "LessThan";
            break;
        case AST::BinaryOp::LessOrEqual:
            std::cout << "LessOrEqual";
            break;
        case AST::BinaryOp::GreaterThan:
            std::cout << "GreaterThan";
            break;
        case AST::BinaryOp::GreaterOrEqual:
            std::cout << "GreaterOrEqual";
            break;
        case AST::BinaryOp::BitwiseXor:
            std::cout << "BitwiseXor";
            break;
        case AST::BinaryOp::BitwiseOr:
            std::cout << "BitwiseOr";
            break;
        case AST::BinaryOp::BitShiftLeft:
            std::cout << "BitShiftLeft";
            break;
        case AST::BinaryOp::BitShiftRight:
            std::cout << "BitShiftRight";
            break;
        default:
            break;
        }
        std::cout << ",\n";
        std::cout << getIndent() << "left=";
        visit(*binary.getExp1(), false);
        std::cout << getIndent() << "right=";
        visit(*binary.getExp2(), false);
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitVar(const AST::Var &var, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Var(" << var.getName() << ")\n";
    }

    void visitAssignment(const AST::Assignment &assignment, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Assignment(\n";
        increaseIndent();
        std::cout << getIndent() << "left=";
        visit(*assignment.getLeftExp(), false);
        std::cout << getIndent() << "right=";
        visit(*assignment.getRightExp(), false);
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitCompoundAssignment(const AST::CompoundAssignment &compoundAssignment, bool indent = true)
    {
        if (indent)
        {
            std::cout << getIndent();
        }
        std::cout << "CompoundAssignment(\n";
        increaseIndent();
        std::cout << getIndent() << "op=" << getOpString(compoundAssignment.getOp()) << ",\n";
        std::cout << getIndent() << "left=";
        visit(*compoundAssignment.getLeftExp(), false);
        std::cout << getIndent() << "right=";
        visit(*compoundAssignment.getRightExp(), false);
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    std::string getOpString(AST::BinaryOp op)
    {
        switch (op)
        {
        case AST::BinaryOp::Add:
            return "Add";
        case AST::BinaryOp::Subtract:
            return "Subtract";
        case AST::BinaryOp::Multiply:
            return "Multiply";
        case AST::BinaryOp::Divide:
            return "Divide";
        case AST::BinaryOp::Remainder:
            return "Remainder";
        case AST::BinaryOp::And:
            return "And";
        case AST::BinaryOp::Or:
            return "Or";
        case AST::BinaryOp::Equal:
            return "Equal";
        case AST::BinaryOp::NotEqual:
            return "NotEqual";
        case AST::BinaryOp::LessThan:
            return "LessThan";
        case AST::BinaryOp::LessOrEqual:
            return "LessOrEqual";
        case AST::BinaryOp::GreaterThan:
            return "GreaterThan";
        case AST::BinaryOp::GreaterOrEqual:
            return "GreaterOrEqual";
        case AST::BinaryOp::BitwiseAnd:
            return "BitwiseAnd";
        case AST::BinaryOp::BitwiseOr:
            return "BitwiseOr";
        case AST::BinaryOp::BitwiseXor:
            return "BitwiseXor";
        case AST::BinaryOp::BitShiftLeft:
            return "BitShiftLeft";
        case AST::BinaryOp::BitShiftRight:
            return "BitShiftRight";
        default:
            return "Unknown";
        }
    }

    void visitPostfixIncr(const AST::PostfixIncr &postfixIncr, bool indent = true)
    {
        if (indent)
        {
            std::cout << getIndent();
        }
        std::cout << "PostfixIncr(\n";
        increaseIndent();
        std::cout << getIndent() << "exp=";
        visit(*postfixIncr.getExp(), false);
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitPostfixDecr(const AST::PostfixDecr &postfixDecr, bool indent = true)
    {
        if (indent)
        {
            std::cout << getIndent();
        }
        std::cout << "PostfixDecr(\n";
        increaseIndent();
        std::cout << getIndent() << "exp=";
        visit(*postfixDecr.getExp(), false);
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitConditional(const AST::Conditional &conditional, bool indent = true)
    {
        if (indent)
        {
            std::cout << getIndent();
        }
        std::cout << "Conditional(\n";
        increaseIndent();
        std::cout << getIndent() << "condition=";
        visit(*conditional.getCondition(), false);
        std::cout << getIndent() << "then=";
        visit(*conditional.getThen(), false);
        std::cout << getIndent() << "else=";
        visit(*conditional.getElse(), false);
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitExpressionStmt(const AST::ExpressionStmt &exprStmt, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "ExpressionStmt(\n";
        increaseIndent();
        visit(*exprStmt.getExp());
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitIf(const AST::If &ifStmt, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "If(\n";
        increaseIndent();
        std::cout << getIndent() << "condition=";
        visit(*ifStmt.getCondition(), false);
        std::cout << getIndent() << "thenClause=";
        visit(*ifStmt.getThenClause(), false);

        if (ifStmt.getElseClause().has_value())
        {
            std::cout << getIndent() << "elseClause=";
            visit(**ifStmt.getElseClause(), false);
        }

        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitCompound(const AST::Compound &compound, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Compound(\n";
        increaseIndent();

        for (const auto &item : compound.getBlock())
        {
            visit(*item);
        }

        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitBreak(const AST::Break &brk, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Break(" << brk.getId() << ")\n";
    }
    void visitContinue(const AST::Continue &cont, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Continue(" << cont.getId() << ")\n";
    }
    void visitWhile(const AST::While &whileLoop, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "While(\n";
        increaseIndent();
        std::cout << getIndent() << "condition=";
        visit(*whileLoop.getCondition(), false);
        std::cout << getIndent() << "body=";
        visit(*whileLoop.getBody(), false);
        std::cout << getIndent() << "id=\"" << whileLoop.getId() << "\"\n";
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }
    void visitDoWhile(const AST::DoWhile &doLoop, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "DoWhile(\n";
        increaseIndent();
        std::cout << getIndent() << "body=";
        visit(*doLoop.getBody(), false);
        std::cout << getIndent() << "condition=";
        visit(*doLoop.getCondition(), false);
        std::cout << getIndent() << "id=\"" << doLoop.getId() << "\"\n";
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }
    void visitFor(const AST::For &forLoop, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "For(\n";
        increaseIndent();

        std::cout << getIndent() << "init=";
        auto &init = forLoop.getInit();

        if (init->getType() == AST::NodeType::InitDecl)
        {
            visit(*static_cast<AST::InitDecl &>(*init).getDecl(), false);
        }
        else
        {
            auto &expInit = static_cast<AST::InitExp &>(*init);

            if (expInit.hasExp())
            {
                visit(*expInit.getExp(), false);
            }
            else
            {
                std::cout << "None\n";
            }
        }

        std::cout << getIndent() << "condition=";
        if (forLoop.getCondition().has_value())
        {
            visit(**forLoop.getCondition(), false);
        }
        else
        {
            std::cout << "None\n";
        }

        std::cout << getIndent() << "post=";
        if (forLoop.getPost().has_value())
        {
            visit(**forLoop.getPost(), false);
        }
        else
        {
            std::cout << "None\n";
        }

        std::cout << getIndent() << "body=";
        visit(*forLoop.getBody(), false);
        std::cout << getIndent() << "id=\"" << forLoop.getId() << "\"\n";
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitNull(const AST::Null &null, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Null()\n";
    }

    void visitLabeledStatement(const AST::LabeledStatement &labeledStmt, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "LabeledStatement(\n";
        increaseIndent();
        std::cout << getIndent() << "label=" << labeledStmt.getLabel() << '\n';
        std::cout << getIndent() << "statement=";
        visit(*labeledStmt.getStatement(), false);
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitGoto(const AST::Goto &gotoStmt, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Goto(" << gotoStmt.getLabel() << ")\n";
    }

    void visitSwitch(const AST::Switch &switchStmt, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Switch(\n";
        increaseIndent();
        std::cout << getIndent() << "control=";
        visit(*switchStmt.getControl(), false);
        std::cout << getIndent() << "body=";
        visit(*switchStmt.getBody(), false);

        if (switchStmt.getCases().has_value())
        {
            std::cout << getIndent() << "cases=\n";
            increaseIndent();
            for (const auto &[key, value] : switchStmt.getCases().value())
            {
                if (key.has_value())
                {
                    std::cout << getIndent() << "Case(" << key.value() << ": " << value << ")\n";
                }
                else
                {
                    std::cout << getIndent() << "Default(" << value << ")\n";
                }
            }
            decreaseIndent();
        }
        else
        {
            std::cout << getIndent() << "cases=None\n";
        }

        std::cout << getIndent() << "id=\"" << switchStmt.getId() << "\"\n";
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitCase(const AST::Case &caseStmt, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Case(\n";
        increaseIndent();
        std::cout << getIndent() << "value=";
        visit(*caseStmt.getValue(), false);
        std::cout << getIndent() << "body=";
        visit(*caseStmt.getBody(), false);
        std::cout << getIndent() << "id=\"" << caseStmt.getId() << "\"\n";
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitDefault(const AST::Default &defaultStmt, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Case(\n";
        increaseIndent();
        std::cout << getIndent() << "body=";
        visit(*defaultStmt.getBody(), false);
        std::cout << getIndent() << "id=\"" << defaultStmt.getId() << "\"\n";
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitDeclaration(const AST::Declaration &decl, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Declaration(\n";
        increaseIndent();
        std::cout << getIndent() << "name=\"" << decl.getName() << "\"";
        if (decl.getInit())
        {
            std::cout << ",\n"
                      << getIndent() << "init=";
            visit(**decl.getInit(), false);
        }
        else
        {
            std::cout << "\n";
        }
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }
};

#endif // AST_PRETTY_PRINT_H