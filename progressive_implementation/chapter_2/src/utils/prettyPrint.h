#ifndef PRETTY_PRINT_H
#define PRETTY_PRINT_H

#include <iostream>
#include <string>
#include <memory>
#include "./../AST.h"
#include "./../Assembly.h"
#include "./../Tacky.h"

class PrettyPrint
{
public:
    PrettyPrint() : indentLevel(0) {}

    void print(const AST::Program &program)
    {
        visit(program);
    }

    void print(const Assembly::Program &program)
    {
        visit(program);
    }

    void print(const TACKY::Program &program)
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
        default:
            std::cerr << "Unknown node type" << std::endl;
            break;
        }
    }

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
        std::cout << getIndent() << "body=";
        visit(*funcDef.getBody(), false);
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
        std::cout << getIndent() << "op=" << (unary.getOp() == AST::UnaryOp::Complement ? "Complement" : "Negate") << ",\n";
        std::cout << getIndent() << "exp=";
        visit(*unary.getExp(), false);
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitProgram(const Assembly::Program &program)
    {
        std::cout << getIndent() << "Program(\n";
        increaseIndent();
        visit(*program.getFunction());
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

    void visitProgram(const TACKY::Program &program)
    {
        std::cout << getIndent() << "Program(\n";
        increaseIndent();
        visit(*program.getFunction());
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitFunction(const TACKY::Function &func)
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

    void visitReturn(const TACKY::Return &ret, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Return(\n";
        increaseIndent();
        visit(*ret.getValue());
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitConstant(const TACKY::Constant &constant, bool indent)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Constant(" << constant.getValue() << ")\n";
    }

    void visitUnary(const TACKY::Unary &unary, bool indent = true)
    {
        std::cout << getIndent() << "Unary(\n";
        increaseIndent();
        std::cout << getIndent() << "op=" << (unary.getOp() == TACKY::UnaryOp::Complement ? "Complement" : "Negate") << ",\n";
        std::cout << getIndent() << "src=";
        visit(*unary.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*unary.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitVar(const TACKY::Var &var, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Var(" << var.getName() << ")\n";
    }

    template <typename T>
    void printMember(const std::string &name, const T &value)
    {
        std::cout << getIndent() << name << "=";
        visit(value, false);
    }
};

#endif // PRETTY_PRINT_H