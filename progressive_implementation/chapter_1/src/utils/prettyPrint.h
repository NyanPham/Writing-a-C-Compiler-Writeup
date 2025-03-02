#ifndef PRETTY_PRINT_H
#define PRETTY_PRINT_H

#include <iostream>
#include <string>
#include <memory>
#include "./../AST.h"
#include "./../Assembly.h"

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
            visitConstant(static_cast<const AST::Constant &>(node));
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
        case Assembly::NodeType::Imm:
            visitImm(static_cast<const Assembly::Imm &>(node), indent);
            break;
        case Assembly::NodeType::Register:
            visitRegister(static_cast<const Assembly::Register &>(node), indent);
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
        visit(*funcDef.getBody(), false); // Pass false to avoid extra indentation
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

    void visitConstant(const AST::Constant &constant)
    {
        std::cout << getIndent() << "Constant(" << constant.getValue() << ")\n";
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

    void visitImm(const Assembly::Imm &imm, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Imm(" << imm.getValue() << ")\n";
    }

    void visitRegister(const Assembly::Register &reg, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Register()\n";
    }

    template <typename T>
    void printMember(const std::string &name, const T &value)
    {
        std::cout << getIndent() << name << "=";
        visit(value, false); // Pass false to avoid extra indentation
    }
};

#endif // PRETTY_PRINT_H