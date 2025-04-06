#include <string>
#include <memory>
#include <format>
#include <iostream>
#include <fstream>

#include "Emit.h"
#include "Assembly.h"

std::string Emit::showReg(const std::shared_ptr<Assembly::Reg> &reg)
{
    switch (reg->getName())
    {
    case Assembly::RegName::AX:
        return "%eax";
    case Assembly::RegName::DX:
        return "%edx";
    case Assembly::RegName::CX:
        return "%ecx";
    case Assembly::RegName::DI:
        return "%edi";
    case Assembly::RegName::SI:
        return "%esi";
    case Assembly::RegName::R8:
        return "%r8d";
    case Assembly::RegName::R9:
        return "%r9d";
    case Assembly::RegName::R10:
        return "%r10d";
    case Assembly::RegName::R11:
        return "%r11d";
    default:
        throw std::runtime_error("Internal Error: Unknown register!");
    }
}

std::string Emit::showOperand(const std::shared_ptr<Assembly::Operand> &operand)
{
    if (auto reg = std::dynamic_pointer_cast<Assembly::Reg>(operand))
    {
        return showReg(reg);
    }

    if (auto imm = std::dynamic_pointer_cast<Assembly::Imm>(operand))
    {
        return std::format("${}", std::to_string(imm->getValue()));
    }

    if (auto stack = std::dynamic_pointer_cast<Assembly::Stack>(operand))
    {
        return std::format("{}(%rbp)", stack->getOffset());
    }

    // Printing pseudo is only for debugging
    if (auto pseudo = std::dynamic_pointer_cast<Assembly::Pseudo>(operand))
    {
        return pseudo->getName();
    }

    throw std::runtime_error("Internal Error: Unknown operand!");
}

std::string Emit::showByteReg(const std::shared_ptr<Assembly::Reg> &reg)
{
    switch (reg->getName())
    {
    case Assembly::RegName::AX:
        return "%al";
    case Assembly::RegName::DX:
        return "%dl";
    case Assembly::RegName::CX:
        return "%cl";
    case Assembly::RegName::DI:
        return "%dil";
    case Assembly::RegName::SI:
        return "%sil";
    case Assembly::RegName::R8:
        return "%r8b";
    case Assembly::RegName::R9:
        return "%r9b";
    case Assembly::RegName::R10:
        return "%r10b";
    case Assembly::RegName::R11:
        return "%r11b";
    default:
        throw std::runtime_error("Internal Error: Unknown register!");
    }
}

std::string Emit::showByteOperand(const std::shared_ptr<Assembly::Operand> &operand)
{
    if (auto reg = std::dynamic_pointer_cast<Assembly::Reg>(operand))
    {
        return showByteReg(reg);
    }

    return showOperand(operand);
}

std::string Emit::showQuadwordReg(const std::shared_ptr<Assembly::Reg> &reg)
{
    switch (reg->getName())
    {
    case Assembly::RegName::AX:
        return "%rax";
    case Assembly::RegName::DX:
        return "%rdx";
    case Assembly::RegName::CX:
        return "%rcx";
    case Assembly::RegName::DI:
        return "%rdi";
    case Assembly::RegName::SI:
        return "%rsi";
    case Assembly::RegName::R8:
        return "%r8";
    case Assembly::RegName::R9:
        return "%r9";
    case Assembly::RegName::R10:
        return "%r10";
    case Assembly::RegName::R11:
        return "%r11";
    default:
        throw std::runtime_error("Internal Error: Unknown register!");
    }
}

std::string Emit::showQuadwordOperand(const std::shared_ptr<Assembly::Operand> &operand)
{
    if (auto reg = std::dynamic_pointer_cast<Assembly::Reg>(operand))
    {
        return showQuadwordReg(reg);
    }

    return showOperand(operand);
}

std::string Emit::showFunName(const std::shared_ptr<Assembly::Call> &fnCall)
{
    return std::format("{}@PLT", fnCall->getFnName());
}

// Move here

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
    return std::format("\tsubq\t${}, %rsp\n", allocateStack->getOffset());
}

std::string Emit::emitDeallocateStack(std::shared_ptr<Assembly::DeallocateStack> deallocateStack)
{
    return std::format("\taddq\t${}, %rsp\n", deallocateStack->getOffset());
}

std::string Emit::emitPush(std::shared_ptr<Assembly::Push> push)
{
    return std::format("\tpushq\t{}\n", showQuadwordOperand(push->getOperand()));
}

std::string Emit::emitCall(std::shared_ptr<Assembly::Call> call)
{
    return std::format("\tcall\t{}\n", showFunName(call));
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
    case Assembly::NodeType::DeallocateStack:
    {
        return emitDeallocateStack(std::dynamic_pointer_cast<Assembly::DeallocateStack>(inst));
    }
    case Assembly::NodeType::Push:
    {
        return emitPush(std::dynamic_pointer_cast<Assembly::Push>(inst));
    }
    case Assembly::NodeType::Call:
    {
        return emitCall(std::dynamic_pointer_cast<Assembly::Call>(inst));
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
    std::string content{""};

    for (const auto &fn : prog->getFunctions())
    {
        content += emitFunction(fn) + "\n";
    }

    content += emitStackNote();

    return content;
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
