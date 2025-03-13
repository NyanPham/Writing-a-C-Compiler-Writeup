#include <string>
#include <memory>
#include <format>
#include <iostream>
#include <fstream>

#include "Emit.h"
#include "Assembly.h"

std::string Emit::emitReg(std::shared_ptr<Assembly::Reg> reg, OperandSize size)
{
    switch (size)
    {
    case OperandSize::Byte1:
    {
        switch (reg->getName())
        {
        case Assembly::RegName::AX:
            return "%al";
        case Assembly::RegName::DX:
            return "%dl";
        case Assembly::RegName::CX:
            return "%cl";
        case Assembly::RegName::R10:
            return "%r10b";
        case Assembly::RegName::R11:
            return "%r11b";
        default:
            throw std::runtime_error("Internal Error: Unknown register!");
        }
    }
    default:
    {
        switch (reg->getName())
        {
        case Assembly::RegName::AX:
            return "%eax";
        case Assembly::RegName::DX:
            return "%edx";
        case Assembly::RegName::CX:
            return "%ecx";
        case Assembly::RegName::R10:
            return "%r10d";
        case Assembly::RegName::R11:
            return "%r11d";
        default:
            throw std::runtime_error("Internal Error: Unknown register!");
        }
    }
    }
}

std::string Emit::emitImm(std::shared_ptr<Assembly::Imm> imm)
{
    return std::format("${}", imm->getValue());
}

std::string Emit::emitStack(std::shared_ptr<Assembly::Stack> stack)
{
    return std::format("{}(%rbp)", stack->getOffset());
}

std::string Emit::emitPseudo(std::shared_ptr<Assembly::Pseudo> pseudo)
{
    return pseudo->getName();
}

std::string Emit::showOperand(std::shared_ptr<Assembly::Operand> operand)
{
    switch (operand->getType())
    {
    case Assembly::NodeType::Reg:
    {
        return emitReg(std::dynamic_pointer_cast<Assembly::Reg>(operand), OperandSize::Byte4);
    }
    case Assembly::NodeType::Imm:
    {
        return emitImm(std::dynamic_pointer_cast<Assembly::Imm>(operand));
    }
    case Assembly::NodeType::Stack:
    {
        return emitStack(std::dynamic_pointer_cast<Assembly::Stack>(operand));
    }
    case Assembly::NodeType::Pseudo:
    {
        return emitPseudo(std::dynamic_pointer_cast<Assembly::Pseudo>(operand)); // For debugging only
    }
    default:
    {
        throw std::runtime_error("Unknown operand type");
    }
    }
}

std::string Emit::showByteOperand(std::shared_ptr<Assembly::Operand> operand)
{
    switch (operand->getType())
    {
    case Assembly::NodeType::Reg:
    {
        return emitReg(std::dynamic_pointer_cast<Assembly::Reg>(operand), OperandSize::Byte1);
    }
    default:
        return showOperand(operand);
    }
}

std::string Emit::showLabel(const std::string &name)
{
    return std::format("{}", name);
}

std::string Emit::showLocalLabel(const std::string &name)
{
    return std::format(".L{}", name);
}

std::string Emit::showCondCode(Assembly::CondCode condCode)
{
    switch (condCode)
    {
    case Assembly::CondCode::E:
        return "e";
    case Assembly::CondCode::NE:
        return "ne";
    case Assembly::CondCode::L:
        return "l";
    case Assembly::CondCode::LE:
        return "le";
    case Assembly::CondCode::G:
        return "g";
    case Assembly::CondCode::GE:
        return "ge";
    default:
        throw std::runtime_error("Internal Error: Unknown condition code!");
    }
}

std::string Emit::showUnaryOp(Assembly::UnaryOp op)
{
    switch (op)
    {
    case Assembly::UnaryOp::Not:
        return "notl";
    case Assembly::UnaryOp::Neg:
        return "negl";
    default:
        throw std::runtime_error("Internal Error: Invalid unary operator!");
    }
}

std::string Emit::showBinaryOp(Assembly::BinaryOp op)
{
    switch (op)
    {
    case Assembly::BinaryOp::Add:
        return "addl";
    case Assembly::BinaryOp::Sub:
        return "subl";
    case Assembly::BinaryOp::Mult:
        return "imull";
    case Assembly::BinaryOp::And:
        return "andl";
    case Assembly::BinaryOp::Or:
        return "orl ";
    case Assembly::BinaryOp::Xor:
        return "xorl";
    case Assembly::BinaryOp::Sal:
        return "sall";
    case Assembly::BinaryOp::Sar:
        return "sarl";
    default:
        throw std::runtime_error("Internal Error: Invalid binary operator!");
    }
}

std::string Emit::emitMov(std::shared_ptr<Assembly::Mov> mov)
{
    return std::format("\tmovl\t{}, {}\n", showOperand(mov->getSrc()), showOperand(mov->getDst()));
}

std::string Emit::emitRet(std::shared_ptr<Assembly::Ret> ret)
{
    return "\tmovq\t%rbp, %rsp\n\tpopq\t%rbp\n\tret\n";
}

std::string Emit::emitUnary(std::shared_ptr<Assembly::Unary> unary)
{
    return std::format("\t{}\t{}\n", showUnaryOp(unary->getOp()), showOperand(unary->getOperand()));
}

std::string Emit::emitBinary(std::shared_ptr<Assembly::Binary> binary)
{
    if (binary->getOp() == Assembly::BinaryOp::Sal || binary->getOp() == Assembly::BinaryOp::Sar)
    {
        return std::format("\t{}\t{}, {}\n", showBinaryOp(binary->getOp()), showByteOperand(binary->getSrc()), showOperand(binary->getDst()));
    }
    else
    {
        return std::format("\t{}\t{}, {}\n", showBinaryOp(binary->getOp()), showOperand(binary->getSrc()), showOperand(binary->getDst()));
    }
}

std::string Emit::emitCmp(std::shared_ptr<Assembly::Cmp> cmp)
{
    return std::format("\tcmpl\t{}, {}\n", showOperand(cmp->getSrc()), showOperand(cmp->getDst()));
}

std::string Emit::emitIdiv(std::shared_ptr<Assembly::Idiv> idiv)
{
    return std::format("\tidivl\t{}\n", showOperand(idiv->getOperand()));
}

std::string Emit::emitCdq(std::shared_ptr<Assembly::Cdq> cdq)
{
    return "\tcdq\n";
}

std::string Emit::emitJmp(std::shared_ptr<Assembly::Jmp> jmp)
{
    return std::format("\tjmp\t{}\n", showLocalLabel(jmp->getTarget()));
}

std::string Emit::emitJmpCC(std::shared_ptr<Assembly::JmpCC> jmpCC)
{
    return std::format("\tj{}\t{}\n", showCondCode(jmpCC->getCondCode()), showLocalLabel(jmpCC->getTarget()));
}

std::string Emit::emitSetCC(std::shared_ptr<Assembly::SetCC> setCC)
{
    return std::format("\tset{}\t{}\n", showCondCode(setCC->getCondCode()), showOperand(setCC->getOperand()));
}

std::string Emit::emitLabel(std::shared_ptr<Assembly::Label> label)
{
    return std::format("{}:\n", showLocalLabel(label->getName()));
}

std::string Emit::emitAllocateStack(std::shared_ptr<Assembly::AllocateStack> allocateStack)
{
    return std::format("\tsubq\t{}, %rsp\n", allocateStack->getOffset());
}

std::string Emit::emitInst(std::shared_ptr<Assembly::Instruction> inst)
{
    switch (inst->getType())
    {
    case Assembly::NodeType::Mov:
    {
        return emitMov(std::dynamic_pointer_cast<Assembly::Mov>(inst));
    }
    case Assembly::NodeType::Unary:
    {
        return emitUnary(std::dynamic_pointer_cast<Assembly::Unary>(inst));
    }
    case Assembly::NodeType::Binary:
    {
        return emitBinary(std::dynamic_pointer_cast<Assembly::Binary>(inst));
    }
    case Assembly::NodeType::Cmp:
    {
        return emitCmp(std::dynamic_pointer_cast<Assembly::Cmp>(inst));
    }
    case Assembly::NodeType::Idiv:
    {
        return emitIdiv(std::dynamic_pointer_cast<Assembly::Idiv>(inst));
    }
    case Assembly::NodeType::Cdq:
    {
        return emitCdq(std::dynamic_pointer_cast<Assembly::Cdq>(inst));
    }
    case Assembly::NodeType::Jmp:
    {
        return emitJmp(std::dynamic_pointer_cast<Assembly::Jmp>(inst));
    }
    case Assembly::NodeType::JmpCC:
    {
        return emitJmpCC(std::dynamic_pointer_cast<Assembly::JmpCC>(inst));
    }
    case Assembly::NodeType::SetCC:
    {
        return emitSetCC(std::dynamic_pointer_cast<Assembly::SetCC>(inst));
    }
    case Assembly::NodeType::Label:
    {
        return emitLabel(std::dynamic_pointer_cast<Assembly::Label>(inst));
    }
    case Assembly::NodeType::AllocateStack:
    {
        return emitAllocateStack(std::dynamic_pointer_cast<Assembly::AllocateStack>(inst));
    }
    case Assembly::NodeType::Ret:
    {
        return emitRet(std::dynamic_pointer_cast<Assembly::Ret>(inst));
    }
    default:
    {
        throw std::runtime_error("Unknown instruction type");
    }
    }
}

std::string Emit::emitFunction(std::shared_ptr<Assembly::Function> fun)
{
    std::string result = std::format("\t.globl {}\n{}:\n\tpushq\t%rbp\n\tmovq\t%rsp, %rbp\n", showLabel(fun->getName()), showLabel(fun->getName()));
    for (const auto &inst : fun->getInstructions())
    {
        result += emitInst(inst);
    }
    return result;
}

std::string Emit::emitProgram(std::shared_ptr<Assembly::Program> prog)
{
    std::string result = emitFunction(prog->getFunction());
    result += emitStackNote();

    return result;
}

void Emit::emit(std::shared_ptr<Assembly::Program> prog, const std::string &output)
{
    std::string result = emitProgram(prog);
    std::ofstream uout(output);

    if (!uout.is_open())
    {
        throw std::runtime_error("Failed to open output file");
    }

    uout << result;
    uout.close();
}

std::string Emit::emitStackNote()
{
    return "\n\t.section .note.GNU-stack,\"\",@progbits\n";
}
