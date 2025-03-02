#include <string>
#include <memory>
#include <format>
#include <iostream>
#include <fstream>

#include "Emit.h"
#include "Assembly.h"

std::string Emit::emitImm(std::shared_ptr<Assembly::Imm> imm)
{
    return std::format("${}", imm->getValue());
}

std::string Emit::emitReg(std::shared_ptr<Assembly::Register> reg)
{
    return std::format("%{}", "eax");
}

std::string Emit::showOperand(std::shared_ptr<Assembly::Operand> operand)
{
    switch (operand->getType())
    {
    case Assembly::NodeType::Imm:
    {
        return emitImm(std::dynamic_pointer_cast<Assembly::Imm>(operand));
    }
    case Assembly::NodeType::Register:
    {
        return emitReg(std::dynamic_pointer_cast<Assembly::Register>(operand));
    }

    default:
    {
        throw std::runtime_error("Unknown operand type");
    }
    }
}

std::string Emit::showLabel(const std::string &name)
{
    return std::format("_{}", name);
}

std::string Emit::emitRet(std::shared_ptr<Assembly::Ret> ret)
{
    return "\tret\n";
}

std::string Emit::emitMov(std::shared_ptr<Assembly::Mov> mov)
{
    return std::format("\tmovl\t{}, {}\n", showOperand(mov->getSrc()), showOperand(mov->getDst()));
}

std::string Emit::emitInst(std::shared_ptr<Assembly::Instruction> inst)
{
    switch (inst->getType())
    {
    case Assembly::NodeType::Mov:
    {
        return emitMov(std::dynamic_pointer_cast<Assembly::Mov>(inst));
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
    std::string result = std::format("\t.globl {}\n{}:\n", showLabel(fun->getName()), showLabel(fun->getName()));
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
    return "\t.section .note.GNU-stack,\"\",@progbits\n";
}
