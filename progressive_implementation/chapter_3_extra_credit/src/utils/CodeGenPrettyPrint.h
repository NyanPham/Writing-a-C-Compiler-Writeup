#ifndef CODE_GEN_PRETTY_PRINT_H
#define CODE_GEN_PRETTY_PRINT_H

#include <iostream>
#include <string>
#include <memory>
#include "./../Assembly.h"

class CodeGenPrettyPrint
{
public:
    CodeGenPrettyPrint() : indentLevel(0) {}

    void print(const Assembly::Program &program)
    {
        visitProgram(program);
    }

private:
    int indentLevel;

    void increaseIndent() { indentLevel++; }
    void decreaseIndent() { indentLevel--; }
    std::string getIndent() const { return std::string(indentLevel * 4, ' '); }

    void visit(const Assembly::Node &node, bool indent = true)
    {
        switch (node.getType())
        {
        case Assembly::NodeType::Program:
            visitProgram(static_cast<const Assembly::Program &>(node));
            break;
        case Assembly::NodeType::Function:
            visitFunction(static_cast<const Assembly::Function &>(node));
            break;
        case Assembly::NodeType::Ret:
            visitRet(static_cast<const Assembly::Ret &>(node));
            break;
        case Assembly::NodeType::Mov:
            visitMov(static_cast<const Assembly::Mov &>(node), indent);
            break;
        case Assembly::NodeType::Unary:
            visitUnary(static_cast<const Assembly::Unary &>(node), indent);
            break;
        case Assembly::NodeType::Binary:
            visitBinary(static_cast<const Assembly::Binary &>(node), indent);
            break;
        case Assembly::NodeType::Idiv:
            visitIdiv(static_cast<const Assembly::Idiv &>(node), indent);
            break;
        case Assembly::NodeType::Cdq:
            visitCdq(static_cast<const Assembly::Cdq &>(node));
            break;
        case Assembly::NodeType::AllocateStack:
            visitAllocateStack(static_cast<const Assembly::AllocateStack &>(node), indent);
            break;
        case Assembly::NodeType::Imm:
            visitImm(static_cast<const Assembly::Imm &>(node), indent);
            break;
        case Assembly::NodeType::Reg:
            visitReg(static_cast<const Assembly::Reg &>(node), indent);
            break;
        case Assembly::NodeType::Pseudo:
            visitPseudo(static_cast<const Assembly::Pseudo &>(node), indent);
            break;
        case Assembly::NodeType::Stack:
            visitStack(static_cast<const Assembly::Stack &>(node), indent);
            break;
        default:
            std::cerr << "Unknown node type" << std::endl;
            break;
        }
    }

    void visitProgram(const Assembly::Program &program)
    {
        std::cout << getIndent() << "Program(\n";
        increaseIndent();
        visitFunction(*program.getFunction());
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitFunction(const Assembly::Function &func)
    {
        std::cout << getIndent() << "Function(\n";
        increaseIndent();
        std::cout << getIndent() << "name=\"" << func.getName() << "\",\n";
        std::cout << getIndent() << "instructions=\n";
        increaseIndent();
        for (const auto &instr : func.getInstructions())
        {
            visit(*instr);
        }
        decreaseIndent();
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitRet(const Assembly::Ret &ret)
    {
        std::cout << getIndent() << "Ret()\n";
    }

    void visitMov(const Assembly::Mov &mov, bool indent = true)
    {
        std::cout << getIndent() << "Mov(\n";
        increaseIndent();
        printMember("src", *mov.getSrc());
        printMember("dst", *mov.getDst());
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitUnary(const Assembly::Unary &unary, bool indent = true)
    {
        std::cout << getIndent() << "Unary(\n";
        increaseIndent();
        std::cout << getIndent() << "op=" << (unary.getOp() == Assembly::UnaryOp::Not ? "Not" : "Neg") << ",\n";
        std::cout << getIndent() << "operand=";
        visit(*unary.getOperand(), false);
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitBinary(const Assembly::Binary &binary, bool indent = true)
    {
        std::cout << getIndent() << "Binary(\n";
        increaseIndent();
        std::cout << getIndent() << "op=";
        switch (binary.getOp())
        {
        case Assembly::BinaryOp::Add:
            std::cout << "Add";
            break;
        case Assembly::BinaryOp::Sub:
            std::cout << "Sub";
            break;
        case Assembly::BinaryOp::Mult:
            std::cout << "Mult";
            break;
        case Assembly::BinaryOp::And:
            std::cout << "And";
            break;
        case Assembly::BinaryOp::Or:
            std::cout << "Or";
            break;
        case Assembly::BinaryOp::Xor:
            std::cout << "Xor";
            break;
        case Assembly::BinaryOp::Sal:
            std::cout << "Sal";
            break;
        case Assembly::BinaryOp::Sar:
            std::cout << "Sar";
            break;
        }
        std::cout << ",\n";
        std::cout << getIndent() << "src=";
        visit(*binary.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*binary.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitIdiv(const Assembly::Idiv &idiv, bool indent = true)
    {
        std::cout << getIndent() << "Idiv(\n";
        increaseIndent();
        std::cout << getIndent() << "operand=";
        visit(*idiv.getOperand(), false);
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitCdq(const Assembly::Cdq &cdq)
    {
        std::cout << getIndent() << "Cdq()\n";
    }

    void visitAllocateStack(const Assembly::AllocateStack &alloc, bool indent = true)
    {
        std::cout << getIndent() << "AllocateStack(" << alloc.getOffset() << ")\n";
    }

    void visitImm(const Assembly::Imm &imm, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Imm(" << imm.getValue() << ")\n";
    }

    void visitReg(const Assembly::Reg &reg, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Reg(" << (reg.getName() == Assembly::RegName::AX ? "AX" : "R10") << ")\n";
    }

    void visitPseudo(const Assembly::Pseudo &pseudo, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Pseudo(" << pseudo.getName() << ")\n";
    }

    void visitStack(const Assembly::Stack &stack, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Stack(" << stack.getOffset() << ")\n";
    }

    template <typename T>
    void printMember(const std::string &name, const T &value)
    {
        std::cout << getIndent() << name << "=";
        visit(value, false);
    }
};

#endif // CODE_GEN_PRETTY_PRINT_H