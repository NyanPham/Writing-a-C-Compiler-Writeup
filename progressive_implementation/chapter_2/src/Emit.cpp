#include <string>
#include <memory>
#include <format>
#include <iostream>
#include <fstream>

#include "Emit.h"
#include "Assembly.h"

std::string Emit::emitReg(std::shared_ptr<Assembly::Reg> reg)
{
    switch (reg->getName())
    {
    case Assembly::RegName::AX:
        return "%eax";
    case Assembly::RegName::R10:
        return "%r10d";
    default:
        throw std::runtime_error("Internal Error: Unknown register!");
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
        return emitReg(std::dynamic_pointer_cast<Assembly::Reg>(operand));
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

std::string Emit::showLabel(const std::string &name)
{
    return std::format("{}", name);
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
