#ifndef TACKY_PRETTY_PRINT_H
#define TACKY_PRETTY_PRINT_H

#include <iostream>
#include <string>
#include "./../Tacky.h"

class TackyPrettyPrint
{
public:
    TackyPrettyPrint() : indentLevel(0) {}

    void print(const TACKY::Program &program)
    {
        visitProgram(program);
    }

private:
    int indentLevel;

    void increaseIndent() { indentLevel++; }
    void decreaseIndent() { indentLevel--; }
    std::string getIndent() const { return std::string(indentLevel * 4, ' '); }

    void visit(const TACKY::Node &node, bool indent = true)
    {
        switch (node.getType())
        {
        case TACKY::NodeType::Program:
            visitProgram(static_cast<const TACKY::Program &>(node));
            break;
        case TACKY::NodeType::Function:
            visitFunction(static_cast<const TACKY::Function &>(node));
            break;
        case TACKY::NodeType::Return:
            visitReturn(static_cast<const TACKY::Return &>(node), indent);
            break;
        case TACKY::NodeType::Unary:
            visitUnary(static_cast<const TACKY::Unary &>(node), indent);
            break;
        case TACKY::NodeType::Binary:
            visitBinary(static_cast<const TACKY::Binary &>(node), indent);
            break;
        case TACKY::NodeType::Copy:
            visitCopy(static_cast<const TACKY::Copy &>(node), indent);
            break;
        case TACKY::NodeType::Jump:
            visitJump(static_cast<const TACKY::Jump &>(node), indent);
            break;
        case TACKY::NodeType::JumpIfZero:
            visitJumpIfZero(static_cast<const TACKY::JumpIfZero &>(node), indent);
            break;
        case TACKY::NodeType::JumpIfNotZero:
            visitJumpIfNotZero(static_cast<const TACKY::JumpIfNotZero &>(node), indent);
            break;
        case TACKY::NodeType::Label:
            visitLabel(static_cast<const TACKY::Label &>(node), indent);
            break;
        case TACKY::NodeType::FunCall:
            visitFunCall(static_cast<const TACKY::FunCall &>(node), indent);
            break;
        case TACKY::NodeType::Constant:
            visitConstant(static_cast<const TACKY::Constant &>(node), indent);
            break;
        case TACKY::NodeType::Var:
            visitVar(static_cast<const TACKY::Var &>(node), indent);
            break;
        default:
            std::cerr << "Unknown node type" << std::endl;
            break;
        }
    }

    void visitProgram(const TACKY::Program &program)
    {
        std::cout << getIndent() << "Program(\n";
        increaseIndent();

        for (const auto &fnDef : program.getFunctions())
        {
            visitFunction(*fnDef);
        }

        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitFunction(const TACKY::Function &func)
    {
        std::cout << getIndent() << "Function(\n";
        increaseIndent();
        std::cout << getIndent() << "name=\"" << func.getName() << "\",\n";
        std::cout << getIndent() << "params=[\n";

        for (const auto &param : func.getParams())
            std::cout << getIndent() << "\t\"" << param << "\",\n";

        std::cout << getIndent() << "],\n";

        std::cout << getIndent() << "instructions=\n";
        increaseIndent();
        for (const auto &instr : func.getInstructions())
        {
            visit(*instr);
        }
        decreaseIndent();
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitReturn(const TACKY::Return &ret, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Return(\n";
        increaseIndent();
        visit(*ret.getValue());
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitUnary(const TACKY::Unary &unary, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Unary(\n";
        increaseIndent();
        std::cout << getIndent() << "op=" << (unary.getOp() == TACKY::UnaryOp::Complement ? "Complement" : "Negate") << ",\n";
        std::cout << getIndent() << "src=";
        visit(*unary.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*unary.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitBinary(const TACKY::Binary &binary, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Binary(\n";
        increaseIndent();
        std::cout << getIndent() << "op=";
        switch (binary.getOp())
        {
        case TACKY::BinaryOp::Add:
            std::cout << "Add";
            break;
        case TACKY::BinaryOp::Subtract:
            std::cout << "Subtract";
            break;
        case TACKY::BinaryOp::Multiply:
            std::cout << "Multiply";
            break;
        case TACKY::BinaryOp::Divide:
            std::cout << "Divide";
            break;
        case TACKY::BinaryOp::Remainder:
            std::cout << "Remainder";
            break;
        case TACKY::BinaryOp::And:
            std::cout << "And";
            break;
        case TACKY::BinaryOp::Or:
            std::cout << "Or";
            break;
        case TACKY::BinaryOp::Equal:
            std::cout << "Equal";
            break;
        case TACKY::BinaryOp::NotEqual:
            std::cout << "NotEqual";
            break;
        case TACKY::BinaryOp::LessThan:
            std::cout << "LessThan";
            break;
        case TACKY::BinaryOp::LessOrEqual:
            std::cout << "LessOrEqual";
            break;
        case TACKY::BinaryOp::GreaterThan:
            std::cout << "GreaterThan";
            break;
        case TACKY::BinaryOp::GreaterOrEqual:
            std::cout << "GreaterOrEqual";
            break;
        case TACKY::BinaryOp::BitwiseAnd:
            std::cout << "BitwiseAnd";
            break;
        case TACKY::BinaryOp::BitwiseOr:
            std::cout << "BitwiseOr";
            break;
        case TACKY::BinaryOp::BitwiseXor:
            std::cout << "BitwiseXor";
            break;
        case TACKY::BinaryOp::BitShiftLeft:
            std::cout << "BitShiftLeft";
            break;
        case TACKY::BinaryOp::BitShiftRight:
            std::cout << "BitShiftRight";
            break;
        }
        std::cout << ",\n";
        std::cout << getIndent() << "src1=";
        visit(*binary.getSrc1(), false);
        std::cout << getIndent() << "src2=";
        visit(*binary.getSrc2(), false);
        std::cout << getIndent() << "dst=";
        visit(*binary.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitCopy(const TACKY::Copy &copy, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Copy(\n";
        increaseIndent();
        std::cout << getIndent() << "src=";
        visit(*copy.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*copy.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitJump(const TACKY::Jump &jump, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Jump(target=" << jump.getTarget() << ")\n";
    }

    void visitJumpIfZero(const TACKY::JumpIfZero &jumpIfZero, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "JumpIfZero(\n";
        increaseIndent();
        std::cout << getIndent() << "cond=";
        visit(*jumpIfZero.getCond(), false);
        std::cout << getIndent() << "target=" << jumpIfZero.getTarget() << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitJumpIfNotZero(const TACKY::JumpIfNotZero &jumpIfNotZero, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "JumpIfNotZero(\n";
        increaseIndent();
        std::cout << getIndent() << "cond=";
        visit(*jumpIfNotZero.getCond(), false);
        std::cout << getIndent() << "target=" << jumpIfNotZero.getTarget() << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitLabel(const TACKY::Label &label, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Label(name=" << label.getName() << ")\n";
    }

    void visitFunCall(const TACKY::FunCall &fnCall, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "FunCall(\n";
        increaseIndent();
        std::cout << getIndent() << "name=" << fnCall.getFnName() << "\n";
        std::cout << getIndent() << "args=[\n";
        increaseIndent();
        for (const auto &arg : fnCall.getArgs())
        {
            visit(*arg, true);
        }
        decreaseIndent();
        std::cout << getIndent() << "],\n";

        std::cout << getIndent() << "dst=";
        visit(*fnCall.getDst(), false);

        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitConstant(const TACKY::Constant &constant, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Constant(" << constant.getValue() << ")\n";
    }

    void visitVar(const TACKY::Var &var, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Var(" << var.getName() << ")\n";
    }
};

#endif