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
        case Assembly::NodeType::StaticVariable:
            visitStaticVariable(static_cast<const Assembly::StaticVariable &>(node));
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
        case Assembly::NodeType::Cmp:
            visitCmp(static_cast<const Assembly::Cmp &>(node), indent);
            break;
        case Assembly::NodeType::Idiv:
            visitIdiv(static_cast<const Assembly::Idiv &>(node), indent);
            break;
        case Assembly::NodeType::Cdq:
            visitCdq(static_cast<const Assembly::Cdq &>(node));
            break;
        case Assembly::NodeType::Jmp:
            visitJmp(static_cast<const Assembly::Jmp &>(node), indent);
            break;
        case Assembly::NodeType::JmpCC:
            visitJmpCC(static_cast<const Assembly::JmpCC &>(node), indent);
            break;
        case Assembly::NodeType::SetCC:
            visitSetCC(static_cast<const Assembly::SetCC &>(node), indent);
            break;
        case Assembly::NodeType::Label:
            visitLabel(static_cast<const Assembly::Label &>(node), indent);
            break;
        case Assembly::NodeType::AllocateStack:
            visitAllocateStack(static_cast<const Assembly::AllocateStack &>(node), indent);
            break;
        case Assembly::NodeType::DeallocateStack:
            visitDeallocateStack(static_cast<const Assembly::DeallocateStack &>(node), indent);
            break;
        case Assembly::NodeType::Push:
            visitPush(static_cast<const Assembly::Push &>(node), indent);
            break;
        case Assembly::NodeType::Call:
            visitCall(static_cast<const Assembly::Call &>(node), indent);
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
        case Assembly::NodeType::Data:
            visitData(static_cast<const Assembly::Data &>(node), indent);
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

        for (const auto &tl : program.getTopLevels())
        {
            if (auto fn = std::dynamic_pointer_cast<Assembly::Function>(tl))
            {
                visitFunction(*fn);
            }
            else if (auto var = std::dynamic_pointer_cast<Assembly::StaticVariable>(tl))
            {
                visitStaticVariable(*var);
            }
        }

        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitFunction(const Assembly::Function &func)
    {
        std::cout << getIndent() << "Function(\n";
        increaseIndent();
        std::cout << getIndent() << "name=\"" << func.getName() << "\",\n";
        std::cout << getIndent() << "global=\"" << func.isGlobal() << "\",\n";
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

    void visitStaticVariable(const Assembly::StaticVariable &staticVar)
    {
        std::cout << getIndent() << "StaticVariable(\n";
        increaseIndent();
        std::cout << getIndent() << "name=\"" << staticVar.getName() << "\",\n";
        std::cout << getIndent() << "global=\"" << staticVar.isGlobal() << "\",\n";
        std::cout << getIndent() << "init=\"" << std::to_string(staticVar.getInit()) << "\",\n";

        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitRet(const Assembly::Ret &ret)
    {
        std::cout << getIndent() << "Ret(),\n";
    }

    void visitMov(const Assembly::Mov &mov, bool indent = true)
    {
        std::cout << getIndent() << "Mov(\n";
        increaseIndent();
        printMember("src", *mov.getSrc());
        printMember("dst", *mov.getDst());
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitUnary(const Assembly::Unary &unary, bool indent = true)
    {
        std::cout << getIndent() << "Unary(\n";
        increaseIndent();
        std::cout << getIndent() << "op=" << (unary.getOp() == Assembly::UnaryOp::Not ? "Not" : "Neg") << ",\n";
        std::cout << getIndent() << "operand=";
        visit(*unary.getOperand(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
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
        std::cout << getIndent() << "),\n";
    }

    void visitCmp(const Assembly::Cmp &cmp, bool indent = true)
    {
        std::cout << getIndent() << "Cmp(\n";
        increaseIndent();
        std::cout << getIndent() << "src=";
        visit(*cmp.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*cmp.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitIdiv(const Assembly::Idiv &idiv, bool indent = true)
    {
        std::cout << getIndent() << "Idiv(\n";
        increaseIndent();
        std::cout << getIndent() << "operand=";
        visit(*idiv.getOperand(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitCdq(const Assembly::Cdq &cdq)
    {
        std::cout << getIndent() << "Cdq()\n";
    }

    void visitJmp(const Assembly::Jmp &jmp, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Jmp(target=" << jmp.getTarget() << ")\n";
    }

    void visitJmpCC(const Assembly::JmpCC &jmpCC, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "JmpCC(\n";
        increaseIndent();
        std::cout << getIndent() << "condCode=" << showCondCode(jmpCC.getCondCode()) << ",\n";
        std::cout << getIndent() << "target=" << jmpCC.getTarget() << '\n';
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitSetCC(const Assembly::SetCC &setCC, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "SetCC(\n";
        increaseIndent();
        std::cout << getIndent() << "condCode=" << showCondCode(setCC.getCondCode()) << ",\n";
        std::cout << getIndent() << "operand=";
        visit(*setCC.getOperand(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitLabel(const Assembly::Label &label, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Label(name=" << label.getName() << ")\n";
    }

    void visitAllocateStack(const Assembly::AllocateStack &alloc, bool indent = true)
    {
        std::cout << getIndent() << "AllocateStack(" << alloc.getOffset() << ")\n";
    }

    void visitDeallocateStack(const Assembly::DeallocateStack &dealloc, bool indent = true)
    {
        std::cout << getIndent() << "DeallocateStack(" << dealloc.getOffset() << ")\n";
    }

    void visitPush(const Assembly::Push &push, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Push(\n";
        increaseIndent();
        std::cout << getIndent() << "operand=";
        visit(*push.getOperand(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitCall(const Assembly::Call &call, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Call(fnName=" << call.getFnName() << ")\n";
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
        std::cout << "Reg(" << showRegName(reg.getName()) << ")\n";
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

    void visitData(const Assembly::Data &data, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Data(" << data.getName() << ")\n";
    }

    template <typename T>
    void printMember(const std::string &name, const T &value)
    {
        std::cout << getIndent() << name << "=";
        visit(value, false);
    }

    std::string showCondCode(Assembly::CondCode condCode)
    {
        switch (condCode)
        {
        case Assembly::CondCode::E:
            return "E";
        case Assembly::CondCode::NE:
            return "NE";
        case Assembly::CondCode::L:
            return "L";
        case Assembly::CondCode::LE:
            return "LE";
        case Assembly::CondCode::G:
            return "G";
        case Assembly::CondCode::GE:
            return "GE";
        default:
            return "Unknown";
        }
    }

    std::string showRegName(Assembly::RegName regName)
    {
        switch (regName)
        {
        case Assembly::RegName::AX:
            return "AX";
        case Assembly::RegName::CX:
            return "CX";
        case Assembly::RegName::DX:
            return "DX";
        case Assembly::RegName::DI:
            return "DI";
        case Assembly::RegName::SI:
            return "SI";
        case Assembly::RegName::R8:
            return "R8";
        case Assembly::RegName::R9:
            return "R9";
        case Assembly::RegName::R10:
            return "R10";
        case Assembly::RegName::R11:
            return "R11";
        default:
            return "Unknown";
        }
    }
};

#endif