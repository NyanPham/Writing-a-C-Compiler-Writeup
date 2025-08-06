#ifndef CODE_GEN_PRETTY_PRINT_H
#define CODE_GEN_PRETTY_PRINT_H

#include <iostream>
#include <sstream>
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
        case Assembly::NodeType::StaticConstant:
            visitStaticConstant(static_cast<const Assembly::StaticConstant &>(node));
            break;
        case Assembly::NodeType::Ret:
            visitRet(static_cast<const Assembly::Ret &>(node));
            break;
        case Assembly::NodeType::Mov:
            visitMov(static_cast<const Assembly::Mov &>(node), indent);
            break;
        case Assembly::NodeType::Movsx:
            visitMovsx(static_cast<const Assembly::Movsx &>(node), indent);
            break;
        case Assembly::NodeType::MovZeroExtend:
            visitMovZeroExtend(static_cast<const Assembly::MovZeroExtend &>(node), indent);
            break;
        case Assembly::NodeType::Lea:
            visitLea(static_cast<const Assembly::Lea &>(node), indent);
            break;
        case Assembly::NodeType::Cvttsd2si:
            visitCvttsd2si(static_cast<const Assembly::Cvttsd2si &>(node), indent);
            break;
        case Assembly::NodeType::Cvtsi2sd:
            visitCvtsi2sd(static_cast<const Assembly::Cvtsi2sd &>(node), indent);
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
        case Assembly::NodeType::Div:
            visitDiv(static_cast<const Assembly::Div &>(node), indent);
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
        case Assembly::NodeType::Push:
            visitPush(static_cast<const Assembly::Push &>(node), indent);
            break;
        case Assembly::NodeType::Pop:
            visitPop(static_cast<const Assembly::Pop &>(node), indent);
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
        case Assembly::NodeType::Memory:
            visitMemory(static_cast<const Assembly::Memory &>(node), indent);
            break;
        case Assembly::NodeType::Data:
            visitData(static_cast<const Assembly::Data &>(node), indent);
            break;
        case Assembly::NodeType::PseudoMem:
            visitPseudoMem(static_cast<const Assembly::PseudoMem &>(node), indent);
            break;
        case Assembly::NodeType::Indexed:
            visitIndexed(static_cast<const Assembly::Indexed &>(node), indent);
            break;
        default:
            std::cerr << "Unknown node type" << std::endl;
            break;
        }
    }

    std::string showRegName(Assembly::RegName regName)
    {
        switch (regName)
        {
        case Assembly::RegName::AX:
            return "AX";
        case Assembly::RegName::BX:
            return "BX";
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
        case Assembly::RegName::R12:
            return "R12";
        case Assembly::RegName::R13:
            return "R13";
        case Assembly::RegName::R14:
            return "R14";
        case Assembly::RegName::R15:
            return "R15";
        case Assembly::RegName::SP:
            return "RSP";
        case Assembly::RegName::BP:
            return "BP";
        case Assembly::RegName::XMM0:
            return "XMM0";
        case Assembly::RegName::XMM1:
            return "XMM1";
        case Assembly::RegName::XMM2:
            return "XMM2";
        case Assembly::RegName::XMM3:
            return "XMM3";
        case Assembly::RegName::XMM4:
            return "XMM4";
        case Assembly::RegName::XMM5:
            return "XMM5";
        case Assembly::RegName::XMM6:
            return "XMM6";
        case Assembly::RegName::XMM7:
            return "XMM7";
        case Assembly::RegName::XMM8:
            return "XMM8";
        case Assembly::RegName::XMM9:
            return "XMM9";
        case Assembly::RegName::XMM10:
            return "XMM10";
        case Assembly::RegName::XMM11:
            return "XMM11";
        case Assembly::RegName::XMM12:
            return "XMM12";
        case Assembly::RegName::XMM13:
            return "XMM13";
        case Assembly::RegName::XMM14:
            return "XMM14";
        case Assembly::RegName::XMM15:
            return "XMM15";
        default:
        {
            std::ostringstream oss;
            oss << "Unknown(RegName=" << static_cast<int>(regName) << ")";
            return oss.str();
        }
        }
    }

private:
    int indentLevel;

    void increaseIndent() { indentLevel++; }
    void decreaseIndent() { indentLevel--; }
    std::string getIndent() const { return std::string(indentLevel * 4, ' '); }

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
            else if (auto c = std::dynamic_pointer_cast<Assembly::StaticConstant>(tl))
            {
                visitStaticConstant(*c);
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
        std::cout << getIndent() << "alignment=" << std::to_string(staticVar.getAlignment()) << ",\n";
        std::cout << getIndent() << "inits=[\n";
        increaseIndent();
        for (const auto &init : staticVar.getInits())
        {
            std::cout << getIndent() << Initializers::toString(*init) << ",\n";
        }
        decreaseIndent();
        std::cout << getIndent() << "]\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitStaticConstant(const Assembly::StaticConstant &staticVar)
    {
        std::cout << getIndent() << "StaticConstant(\n";
        increaseIndent();
        std::cout << getIndent() << "name=\"" << staticVar.getName() << "\",\n";
        std::cout << getIndent() << "alignment=" << std::to_string(staticVar.getAlignment()) << "\",\n";
        std::cout << getIndent() << "init=\"" << Initializers::toString(staticVar.getInit()) << "\",\n";

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
        printAsmType(*mov.getAsmType());
        printMember("src", *mov.getSrc());
        printMember("dst", *mov.getDst());
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitMovsx(const Assembly::Movsx &movsx, bool indent = true)
    {
        std::cout << getIndent() << "Movsx(\n";
        increaseIndent();
        printAsmType(*movsx.getSrcType());
        printAsmType(*movsx.getDstType());
        printMember("src", *movsx.getSrc());
        printMember("dst", *movsx.getDst());
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitMovZeroExtend(const Assembly::MovZeroExtend &movzx, bool indent = true)
    {
        std::cout << getIndent() << "MovZeroExtend(\n";
        increaseIndent();
        printAsmType(*movzx.getSrcType());
        printAsmType(*movzx.getDstType());
        printMember("src", *movzx.getSrc());
        printMember("dst", *movzx.getDst());
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitLea(const Assembly::Lea &lea, bool indent = true)
    {
        std::cout << getIndent() << "Lea(\n";
        increaseIndent();
        printMember("src", *lea.getSrc());
        printMember("dst", *lea.getDst());
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitCvttsd2si(const Assembly::Cvttsd2si &cvt, bool indent = true)
    {
        std::cout << getIndent() << "Cvttsd2si(\n";
        increaseIndent();
        printAsmType(*cvt.getAsmType());
        printMember("src", *cvt.getSrc());
        printMember("dst", *cvt.getDst());
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitCvtsi2sd(const Assembly::Cvtsi2sd &cvt, bool indent = true)
    {
        std::cout << getIndent() << "Cvtsi2sd(\n";
        increaseIndent();
        printAsmType(*cvt.getAsmType());
        printMember("src", *cvt.getSrc());
        printMember("dst", *cvt.getDst());
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitUnary(const Assembly::Unary &unary, bool indent = true)
    {
        std::cout << getIndent() << "Unary(\n";
        increaseIndent();
        printAsmType(*unary.getAsmType());
        std::cout << getIndent() << "op=";
        if (unary.getOp() == Assembly::UnaryOp::Not)
            std::cout << "Not,\n";
        else if (unary.getOp() == Assembly::UnaryOp::Neg)
            std::cout << "Neg,\n";
        else
            std::cout << "ShrOneOp,\n";
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
        case Assembly::BinaryOp::DivDouble:
            std::cout << "Div";
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
        case Assembly::BinaryOp::Shl:
            std::cout << "Shl";
            break;
        case Assembly::BinaryOp::Shr:
            std::cout << "Shr";
            break;
        default:
            break;
        }
        std::cout << ",\n";
        printAsmType(*binary.getAsmType());
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
        printAsmType(*cmp.getAsmType());
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
        printAsmType(*idiv.getAsmType());
        std::cout << getIndent() << "operand=";
        visit(*idiv.getOperand(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitDiv(const Assembly::Div &div, bool indent = true)
    {
        std::cout << getIndent() << "Div(\n";
        increaseIndent();
        printAsmType(*div.getAsmType());
        std::cout << getIndent() << "operand=";
        visit(*div.getOperand(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitCdq(const Assembly::Cdq &cdq)
    {
        std::cout << getIndent() << "Cdq(";
        printAsmType(*cdq.getAsmType());
        std::cout << getIndent() << "),\n";
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

    void visitPop(const Assembly::Pop &pop, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Pop(\n";
        increaseIndent();
        std::cout << getIndent() << "reg=";
        visit(*pop.getReg(), false);
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

    void visitMemory(const Assembly::Memory &mem, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Memory(reg=" << showRegName(mem.getReg()->getName()) << ", offset=" << mem.getOffset() << ")\n";
    }

    void visitData(const Assembly::Data &data, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Data(" << data.getName() << ")\n";
    }

    void visitPseudoMem(const Assembly::PseudoMem &pmem, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "PseudoMem(name=" << pmem.getBase() << ", offset=" << pmem.getOffset() << ")\n";
    }

    void visitIndexed(const Assembly::Indexed &indexed, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Indexed(\n";
        increaseIndent();
        std::cout << getIndent() << "base=";
        visit(*indexed.getBase(), false);
        std::cout << getIndent() << "index=";
        visit(*indexed.getIndex(), false);
        std::cout << getIndent() << "scale=" << indexed.getScale() << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    template <typename T>
    void printMember(const std::string &name, const T &value)
    {
        std::cout << getIndent() << name << "=";
        visit(value, false);
    }

    void printAsmType(const Assembly::AsmType &type)
    {
        std::cout << getIndent() << "asmType=" << Assembly::asmTypeToString(type) << '\n';
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
        case Assembly::CondCode::B:
            return "B";
        case Assembly::CondCode::BE:
            return "BE";
        case Assembly::CondCode::A:
            return "A";
        case Assembly::CondCode::AE:
            return "AE";
        case Assembly::CondCode::P:
            return "P";
        case Assembly::CondCode::NP:
            return "NP";
        default:
            return "Unknown";
        }
    }
};

#endif