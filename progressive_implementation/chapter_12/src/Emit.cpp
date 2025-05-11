#include <string>
#include <memory>
#include <format>
#include <iostream>
#include <fstream>

#include "Emit.h"
#include "Assembly.h"

std::string Emit::suffix(const std::shared_ptr<Assembly::AsmType> &asmType)
{
    if (auto asmLong = Assembly::getLongword(*asmType))
        return "l";
    else if (auto asmQuad = Assembly::getQuadword(*asmType))
        return "q";
    else
        throw std::runtime_error("Internal error: Unknown assembly type to get suffix");
}

std::string Emit::emitZeroInit(const Initializers::StaticInit &init)
{
    if (Initializers::isIntInit(init) || Initializers::isUIntInit(init))
        return ".zero 4";
    else if (Initializers::isLongInit(init) || Initializers::isULongInit(init))
        return ".zero 8";
    else
        throw std::runtime_error("Internal error: Invalid StaticInit to emit zero");
}

std::string Emit::emitInit(const Initializers::StaticInit &init)
{
    if (auto intInit = Initializers::getIntInit(init))
        return std::format(".long {}", static_cast<int32_t>(intInit->val));
    else if (auto longInit = Initializers::getLongInit(init))
        return std::format(".quad {}", static_cast<int64_t>(longInit->val));
    else if (auto uIntInit = Initializers::getUIntInit(init))
        return std::format(".long {}", static_cast<uint32_t>(uIntInit->val));
    else if (auto uLongInit = Initializers::getULongInit(init))
        return std::format(".quad {}", static_cast<uint64_t>(uLongInit->val));
    else
        throw std::runtime_error("Internal error: Invalid StaticInit to emit init");
}

std::string Emit::alignDirective()
{
    // Target Linux
    return ".align";
}

std::string Emit::emitGlobalDirective(bool global, const std::string &label)
{
    if (global)
        return std::format("\t.global {}\n", label);
    return "";
}

std::string Emit::showLongReg(const std::shared_ptr<Assembly::Reg> &reg)
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
    case Assembly::RegName::SP:
        throw std::runtime_error("Internal error: no 32-bit RSP");
    default:
        throw std::runtime_error("Internal Error: Unknown register!");
    }
}

std::string Emit::showOperand(const std::shared_ptr<Assembly::AsmType> &asmType, const std::shared_ptr<Assembly::Operand> &operand)
{
    if (auto reg = std::dynamic_pointer_cast<Assembly::Reg>(operand))
    {
        if (Assembly::isAsmLongword(*asmType))
            return showLongReg(reg);
        else
            return showQuadwordReg(reg);
    }

    if (auto imm = std::dynamic_pointer_cast<Assembly::Imm>(operand))
    {
        return std::format("${}", std::to_string(imm->getValue()));
    }

    if (auto stack = std::dynamic_pointer_cast<Assembly::Stack>(operand))
    {
        return std::format("{}(%rbp)", stack->getOffset());
    }

    if (auto data = std::dynamic_pointer_cast<Assembly::Data>(operand))
    {
        return std::format("{}(%rip)", data->getName());
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
    case Assembly::RegName::SP:
        throw std::runtime_error("Internal error: no one-byte RSP");
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

    return showOperand(std::make_shared<Assembly::AsmType>(Assembly::Longword()), operand);
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
    case Assembly::RegName::SP:
        return "%rsp";
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

    return showOperand(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), operand);
}

std::string Emit::showFunName(const std::shared_ptr<Assembly::Call> &fnCall)
{
    // Target the Linux
    if (_asmSymbolTable.isDefined(fnCall->getFnName()))
        return fnCall->getFnName();
    else
        return std::format("{}@PLT", fnCall->getFnName());
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
    case Assembly::CondCode::A:
        return "a";
    case Assembly::CondCode::AE:
        return "ae";
    case Assembly::CondCode::B:
        return "b";
    case Assembly::CondCode::BE:
        return "be";
    default:
        throw std::runtime_error("Internal Error: Unknown condition code!");
    }
}

std::string Emit::showUnaryOp(Assembly::UnaryOp op)
{
    switch (op)
    {
    case Assembly::UnaryOp::Not:
        return "not";
    case Assembly::UnaryOp::Neg:
        return "neg";
    default:
        throw std::runtime_error("Internal Error: Invalid unary operator!");
    }
}

std::string Emit::showBinaryOp(Assembly::BinaryOp op)
{
    switch (op)
    {
    case Assembly::BinaryOp::Add:
        return "add";
    case Assembly::BinaryOp::Sub:
        return "sub";
    case Assembly::BinaryOp::Mult:
        return "imul";
    case Assembly::BinaryOp::And:
        return "and";
    case Assembly::BinaryOp::Or:
        return "or";
    case Assembly::BinaryOp::Xor:
        return "xor";
    case Assembly::BinaryOp::Sal:
        return "sal";
    case Assembly::BinaryOp::Sar:
        return "sar";
    case Assembly::BinaryOp::Shl:
        return "shl";
    case Assembly::BinaryOp::Shr:
        return "shr";
    default:
        throw std::runtime_error("Internal Error: Invalid binary operator!");
    }
}

std::string Emit::emitInst(std::shared_ptr<Assembly::Instruction> inst)
{
    switch (inst->getType())
    {
    case Assembly::NodeType::Mov:
    {
        auto mov = std::dynamic_pointer_cast<Assembly::Mov>(inst);
        return std::format("\tmov{}\t{}, {}\n", suffix(mov->getAsmType()), showOperand(mov->getAsmType(), mov->getSrc()), showOperand(mov->getAsmType(), mov->getDst()));
    }
    case Assembly::NodeType::Unary:
    {
        auto unary = std::dynamic_pointer_cast<Assembly::Unary>(inst);
        return std::format("\t{}{}\t{}\n", showUnaryOp(unary->getOp()), suffix(unary->getAsmType()), showOperand(unary->getAsmType(), unary->getOperand()));
    }
    case Assembly::NodeType::Binary:
    {
        auto binary = std::dynamic_pointer_cast<Assembly::Binary>(inst);
        // Special logic: emit CX reg as %cl
        if (binary->getOp() == Assembly::BinaryOp::Sal || binary->getOp() == Assembly::BinaryOp::Sar || binary->getOp() == Assembly::BinaryOp::Shl || binary->getOp() == Assembly::BinaryOp::Shr)
        {
            return std::format("\t{}{}\t{}, {}\n", showBinaryOp(binary->getOp()), suffix(binary->getAsmType()), showByteOperand(binary->getSrc()), showOperand(binary->getAsmType(), binary->getDst()));
        }
        else
        {
            return std::format("\t{}{}\t{}, {}\n", showBinaryOp(binary->getOp()), suffix(binary->getAsmType()), showOperand(binary->getAsmType(), binary->getSrc()), showOperand(binary->getAsmType(), binary->getDst()));
        }
    }
    case Assembly::NodeType::Cmp:
    {
        auto cmp = std::dynamic_pointer_cast<Assembly::Cmp>(inst);
        return std::format("\tcmp{}\t{}, {}\n", suffix(cmp->getAsmType()), showOperand(cmp->getAsmType(), cmp->getSrc()), showOperand(cmp->getAsmType(), cmp->getDst()));
    }
    case Assembly::NodeType::Idiv:
    {
        auto idiv = std::dynamic_pointer_cast<Assembly::Idiv>(inst);
        return std::format("\tidiv{}\t{}\n", suffix(idiv->getAsmType()), showOperand(idiv->getAsmType(), idiv->getOperand()));
    }
    case Assembly::NodeType::Div:
    {
        auto div = std::dynamic_pointer_cast<Assembly::Div>(inst);
        return std::format("\tdiv{}\t{}\n", suffix(div->getAsmType()), showOperand(div->getAsmType(), div->getOperand()));
    }
    case Assembly::NodeType::Cdq:
    {
        auto cdq = std::dynamic_pointer_cast<Assembly::Cdq>(inst);
        if (Assembly::isAsmLongword(*cdq->getAsmType()))
            return "\tcdq\n";
        else if (Assembly::isAsmQuadword(*cdq->getAsmType()))
            return "\tcdo\n";
        else
            throw std::runtime_error("Internal error: Invalid type for CDQ");
    }
    case Assembly::NodeType::Jmp:
    {
        auto jmp = std::dynamic_pointer_cast<Assembly::Jmp>(inst);
        return std::format("\tjmp\t{}\n", showLocalLabel(jmp->getTarget()));
    }
    case Assembly::NodeType::JmpCC:
    {
        auto jmpCC = std::dynamic_pointer_cast<Assembly::JmpCC>(inst);
        return std::format("\tj{}\t{}\n", showCondCode(jmpCC->getCondCode()), showLocalLabel(jmpCC->getTarget()));
    }
    case Assembly::NodeType::SetCC:
    {
        auto setCC = std::dynamic_pointer_cast<Assembly::SetCC>(inst);
        return std::format("\tset{}\t{}\n", showCondCode(setCC->getCondCode()), showByteOperand(setCC->getOperand()));
    }
    case Assembly::NodeType::Label:
    {
        auto label = std::dynamic_pointer_cast<Assembly::Label>(inst);
        return std::format("{}:\n", showLocalLabel(label->getName()));
    }
    case Assembly::NodeType::Push:
    {
        auto push = std::dynamic_pointer_cast<Assembly::Push>(inst);
        return std::format("\tpushq\t{}\n", showQuadwordOperand(push->getOperand()));
    }
    case Assembly::NodeType::Call:
    {
        auto call = std::dynamic_pointer_cast<Assembly::Call>(inst);
        return std::format("\tcall\t{}\n", showFunName(call));
    }
    case Assembly::NodeType::Movsx:
    {
        auto movsx = std::dynamic_pointer_cast<Assembly::Movsx>(inst);
        return std::format("\tmovslq \t{}, {}\n", showOperand(std::make_shared<Assembly::AsmType>(Assembly::Longword()), movsx->getSrc()), showOperand(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), movsx->getDst()));
    }
    case Assembly::NodeType::Ret:
    {
        return "\tmovq\t%rbp, %rsp\n\tpopq\t%rbp\n\tret\n";
    }
    case Assembly::NodeType::MovZeroExtend:
    {
        throw std::runtime_error("Internal error: MovZeroExtend should have been removed in instruction rewrite pass");
    }
    default:
    {
        throw std::runtime_error("Unknown instruction type");
    }
    }
}

std::string Emit::emitTopLevel(std::shared_ptr<Assembly::TopLevel> topLevel)
{
    if (auto fun = std::dynamic_pointer_cast<Assembly::Function>(topLevel))
    {
        std::string label = showLabel(fun->getName());
        std::string result = std::format("{}{}:\n\tpushq\t%rbp\n\tmovq\t%rsp, %rbp\n", emitGlobalDirective(fun->isGlobal(), label), label);
        for (const auto &inst : fun->getInstructions())
        {
            result += emitInst(inst);
        }
        return result;
    }
    else if (auto staticVar = std::dynamic_pointer_cast<Assembly::StaticVariable>(topLevel))
    {
        auto label = showLabel(staticVar->getName());

        if (Initializers::isZero(staticVar->getInit()))
        {
            return std::format("{}\t.bss\n\t{} {}\n{}:\n\t{}\n", emitGlobalDirective(staticVar->isGlobal(), label), alignDirective(), std::to_string(staticVar->getAlignment()), label, emitZeroInit(staticVar->getInit()));
        }
        else
        {
            return std::format("{}\t.data\n\t{} {}\n{}:\n\t{}\n", emitGlobalDirective(staticVar->isGlobal(), label), alignDirective(), std::to_string(staticVar->getAlignment()), label, emitInit(staticVar->getInit()));
        }
    }
    else
        throw std::runtime_error("Internal error: Unknown assembly toplevel type");
}

void Emit::emit(std::shared_ptr<Assembly::Program> prog, const std::string &output)
{
    std::string content{""};

    for (const auto &tl : prog->getTopLevels())
    {
        content += emitTopLevel(tl) + "\n";
    }

    content += emitStackNote();
    std::ofstream uout(output);

    if (!uout.is_open())
    {
        throw std::runtime_error("Failed to open output file");
    }

    uout << content;
    uout.close();
}

std::string Emit::emitStackNote()
{
    return "\t.section .note.GNU-stack,\"\",@progbits\n";
}
