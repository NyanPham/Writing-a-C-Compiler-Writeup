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
        case AST::NodeType::ExpressionStmt:
            visitExpressionStmt(static_cast<const AST::ExpressionStmt &>(node), indent);
            break;
        case AST::NodeType::Null:
            visitNull(static_cast<const AST::Null &>(node), indent);
            break;
        case AST::NodeType::Declaration:
            visitDeclaration(static_cast<const AST::Declaration &>(node), indent);
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

    void visitNull(const AST::Null &null, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Null()\n";
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
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }
};

#endif // AST_PRETTY_PRINT_H