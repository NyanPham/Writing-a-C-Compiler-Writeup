#ifndef TACKY_PRETTY_PRINT_H
#define TACKY_PRETTY_PRINT_H

#include <iostream>
#include <string>
#include "./../Tacky.h"

class TackyPrettyPrint
{
public:
    TackyPrettyPrint() : indentLevel(0) {}

    void print(const TACKY::Program &program)
    {
        visitProgram(program);
    }

private:
    int indentLevel;

    void increaseIndent() { indentLevel++; }
    void decreaseIndent() { indentLevel--; }
    std::string getIndent() const { return std::string(indentLevel * 4, ' '); }

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
        case TACKY::NodeType::StaticVariable:
            visitStaticVariable(static_cast<const TACKY::StaticVariable &>(node));
            break;
        case TACKY::NodeType::StaticConstant:
            visitStaticConstant(static_cast<const TACKY::StaticConstant &>(node));
            break;
        case TACKY::NodeType::Return:
            visitReturn(static_cast<const TACKY::Return &>(node), indent);
            break;
        case TACKY::NodeType::Unary:
            visitUnary(static_cast<const TACKY::Unary &>(node), indent);
            break;
        case TACKY::NodeType::Binary:
            visitBinary(static_cast<const TACKY::Binary &>(node), indent);
            break;
        case TACKY::NodeType::Copy:
            visitCopy(static_cast<const TACKY::Copy &>(node), indent);
            break;
        case TACKY::NodeType::GetAddress:
            visitGetAddress(static_cast<const TACKY::GetAddress &>(node), indent);
            break;
        case TACKY::NodeType::Load:
            visitLoad(static_cast<const TACKY::Load &>(node), indent);
            break;
        case TACKY::NodeType::Store:
            visitStore(static_cast<const TACKY::Store &>(node), indent);
            break;
        case TACKY::NodeType::Jump:
            visitJump(static_cast<const TACKY::Jump &>(node), indent);
            break;
        case TACKY::NodeType::JumpIfZero:
            visitJumpIfZero(static_cast<const TACKY::JumpIfZero &>(node), indent);
            break;
        case TACKY::NodeType::JumpIfNotZero:
            visitJumpIfNotZero(static_cast<const TACKY::JumpIfNotZero &>(node), indent);
            break;
        case TACKY::NodeType::Label:
            visitLabel(static_cast<const TACKY::Label &>(node), indent);
            break;
        case TACKY::NodeType::FunCall:
            visitFunCall(static_cast<const TACKY::FunCall &>(node), indent);
            break;
        case TACKY::NodeType::Constant:
            visitConstant(static_cast<const TACKY::Constant &>(node), indent);
            break;
        case TACKY::NodeType::Var:
            visitVar(static_cast<const TACKY::Var &>(node), indent);
            break;
        case TACKY::NodeType::SignExtend:
            visitSignExtend(static_cast<const TACKY::SignExtend &>(node), indent);
            break;
        case TACKY::NodeType::Truncate:
            visitTruncate(static_cast<const TACKY::Truncate &>(node), indent);
            break;
        case TACKY::NodeType::ZeroExtend:
            visitZeroExtend(static_cast<const TACKY::ZeroExtend &>(node), indent);
            break;
        case TACKY::NodeType::DoubleToInt:
            visitDoubleToInt(static_cast<const TACKY::DoubleToInt &>(node), indent);
            break;
        case TACKY::NodeType::DoubleToUInt:
            visitDoubleToUInt(static_cast<const TACKY::DoubleToUInt &>(node), indent);
            break;
        case TACKY::NodeType::IntToDouble:
            visitIntToDouble(static_cast<const TACKY::IntToDouble &>(node), indent);
            break;
        case TACKY::NodeType::UIntToDouble:
            visitUIntToDouble(static_cast<const TACKY::UIntToDouble &>(node), indent);
            break;
        case TACKY::NodeType::AddPtr:
            visitAddPtr(static_cast<const TACKY::AddPtr &>(node), indent);
            break;
        case TACKY::NodeType::CopyToOffset:
            visitCopyToOffset(static_cast<const TACKY::CopyToOffset &>(node), indent);
            break;
        case TACKY::NodeType::CopyFromOffset:
            visitCopyFromOffset(static_cast<const TACKY::CopyFromOffset &>(node), indent);
            break;
        default:
            std::cerr << "Unknown node type" << std::endl;
            break;
        }
    }

    void visitProgram(const TACKY::Program &program)
    {
        std::cout << getIndent() << "Program(\n";
        increaseIndent();

        for (const auto &tl : program.getTopLevels())
        {
            if (auto varDef = std::dynamic_pointer_cast<TACKY::StaticVariable>(tl))
            {
                visitStaticVariable(*varDef);
            }
            else if (auto funcDef = std::dynamic_pointer_cast<TACKY::Function>(tl))
            {
                visitFunction(*funcDef);
            }
        }

        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitFunction(const TACKY::Function &func)
    {
        std::cout << getIndent() << "Function(\n";
        increaseIndent();
        std::cout << getIndent() << "name=\"" << func.getName() << "\",\n";
        std::cout << getIndent() << "global=" << func.isGlobal() << ",\n";
        std::cout << getIndent() << "params=[\n";

        for (const auto &param : func.getParams())
            std::cout << getIndent() << "\t\"" << param << "\",\n";

        std::cout << getIndent() << "],\n";

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

    void visitStaticVariable(const TACKY::StaticVariable &staticVar)
    {
        std::cout << getIndent() << "StaticVariable(\n";
        increaseIndent();
        std::cout << getIndent() << "name=\"" << staticVar.getName() << "\",\n";
        std::cout << getIndent() << "global=" << staticVar.isGlobal() << ",\n";
        std::cout << getIndent() << "type=" << Types::dataTypeToString(staticVar.getDataType()) << ",\n";
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

    void visitStaticConstant(const TACKY::StaticConstant &staticConst)
    {
        std::cout << getIndent() << "StaticConstant(\n";
        increaseIndent();
        std::cout << getIndent() << "name=\"" << staticConst.getName() << "\",\n";
        std::cout << getIndent() << "type=" << Types::dataTypeToString(staticConst.getDataType()) << ",\n";
        std::cout << getIndent() << "init=" << Initializers::toString(*staticConst.getInit()) << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitReturn(const TACKY::Return &ret, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Return(\n";
        increaseIndent();
        if (ret.getOptValue().has_value() && ret.getOptValue().value())
        {
            visit(*ret.getOptValue().value());
        }
        else
        {
            std::cout << getIndent() << "None";
        }
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitUnary(const TACKY::Unary &unary, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Unary(\n";
        increaseIndent();
        std::cout << getIndent() << "op=";
        switch (unary.getOp())
        {
        case TACKY::UnaryOp::Complement:
            std::cout << "Complement";
            break;
        case TACKY::UnaryOp::Negate:
            std::cout << "Negate";
            break;
        case TACKY::UnaryOp::Not:
            std::cout << "Not";
            break;
        }
        std::cout << ",\n";
        std::cout << getIndent() << "src=";
        visit(*unary.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*unary.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitBinary(const TACKY::Binary &binary, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Binary(\n";
        increaseIndent();
        std::cout << getIndent() << "op=";
        switch (binary.getOp())
        {
        case TACKY::BinaryOp::Add:
            std::cout << "Add";
            break;
        case TACKY::BinaryOp::Subtract:
            std::cout << "Subtract";
            break;
        case TACKY::BinaryOp::Multiply:
            std::cout << "Multiply";
            break;
        case TACKY::BinaryOp::Divide:
            std::cout << "Divide";
            break;
        case TACKY::BinaryOp::Remainder:
            std::cout << "Remainder";
            break;
        case TACKY::BinaryOp::And:
            std::cout << "And";
            break;
        case TACKY::BinaryOp::Or:
            std::cout << "Or";
            break;
        case TACKY::BinaryOp::Equal:
            std::cout << "Equal";
            break;
        case TACKY::BinaryOp::NotEqual:
            std::cout << "NotEqual";
            break;
        case TACKY::BinaryOp::LessThan:
            std::cout << "LessThan";
            break;
        case TACKY::BinaryOp::LessOrEqual:
            std::cout << "LessOrEqual";
            break;
        case TACKY::BinaryOp::GreaterThan:
            std::cout << "GreaterThan";
            break;
        case TACKY::BinaryOp::GreaterOrEqual:
            std::cout << "GreaterOrEqual";
            break;
        case TACKY::BinaryOp::BitwiseAnd:
            std::cout << "BitwiseAnd";
            break;
        case TACKY::BinaryOp::BitwiseOr:
            std::cout << "BitwiseOr";
            break;
        case TACKY::BinaryOp::BitwiseXor:
            std::cout << "BitwiseXor";
            break;
        case TACKY::BinaryOp::BitShiftLeft:
            std::cout << "BitShiftLeft";
            break;
        case TACKY::BinaryOp::BitShiftRight:
            std::cout << "BitShiftRight";
            break;
        }
        std::cout << ",\n";
        std::cout << getIndent() << "src1=";
        visit(*binary.getSrc1(), false);
        std::cout << getIndent() << "src2=";
        visit(*binary.getSrc2(), false);
        std::cout << getIndent() << "dst=";
        visit(*binary.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitCopy(const TACKY::Copy &copy, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Copy(\n";
        increaseIndent();
        std::cout << getIndent() << "src=";
        visit(*copy.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*copy.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitAddPtr(const TACKY::AddPtr &addPtr, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "AddPtr(\n";
        increaseIndent();
        std::cout << getIndent() << "ptr=";
        visit(*addPtr.getPtr(), false);
        std::cout << getIndent() << "index=";
        visit(*addPtr.getIndex(), false);
        std::cout << getIndent() << "scale=" << addPtr.getScale() << ",\n";
        std::cout << getIndent() << "dst=";
        visit(*addPtr.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitCopyToOffset(const TACKY::CopyToOffset &copyToOffset, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "CopyToOffset(\n";
        increaseIndent();
        std::cout << getIndent() << "src=";
        visit(*copyToOffset.getSrc(), false);
        std::cout << getIndent() << "dst=" << copyToOffset.getDst() << ",\n";
        std::cout << getIndent() << "offset=" << copyToOffset.getOffset() << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitCopyFromOffset(const TACKY::CopyFromOffset &copyFromOffset, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "CopyFromOffset(\n";
        increaseIndent();
        std::cout << getIndent() << "src=\"" << copyFromOffset.getSrc() << "\",\n";
        std::cout << getIndent() << "offset=" << copyFromOffset.getOffset() << ",\n";
        std::cout << getIndent() << "dst=";
        visit(*copyFromOffset.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitGetAddress(const TACKY::GetAddress &getAddr, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "GetAddress(\n";
        increaseIndent();
        std::cout << getIndent() << "src=";
        visit(*getAddr.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*getAddr.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitLoad(const TACKY::Load &load, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Load(\n";
        increaseIndent();
        std::cout << getIndent() << "src_ptr=";
        visit(*load.getSrcPtr(), false);
        std::cout << getIndent() << "dst=";
        visit(*load.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitStore(const TACKY::Store &store, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Store(\n";
        increaseIndent();
        std::cout << getIndent() << "src=";
        visit(*store.getSrc(), false);
        std::cout << getIndent() << "dst_ptr=";
        visit(*store.getDstPtr(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitJump(const TACKY::Jump &jump, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Jump(target=" << jump.getTarget() << ")\n";
    }

    void visitJumpIfZero(const TACKY::JumpIfZero &jumpIfZero, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "JumpIfZero(\n";
        increaseIndent();
        std::cout << getIndent() << "cond=";
        visit(*jumpIfZero.getCond(), false);
        std::cout << getIndent() << "target=" << jumpIfZero.getTarget() << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitJumpIfNotZero(const TACKY::JumpIfNotZero &jumpIfNotZero, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "JumpIfNotZero(\n";
        increaseIndent();
        std::cout << getIndent() << "cond=";
        visit(*jumpIfNotZero.getCond(), false);
        std::cout << getIndent() << "target=" << jumpIfNotZero.getTarget() << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitLabel(const TACKY::Label &label, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Label(name=" << label.getName() << ")\n";
    }

    void visitFunCall(const TACKY::FunCall &fnCall, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "FunCall(\n";
        increaseIndent();
        std::cout << getIndent() << "name=" << fnCall.getFnName() << "\n";
        std::cout << getIndent() << "args=[\n";
        increaseIndent();
        for (const auto &arg : fnCall.getArgs())
        {
            visit(*arg, true);
        }
        decreaseIndent();
        std::cout << getIndent() << "],\n";

        std::cout << getIndent() << "dst=";
        if (fnCall.getOptDst().has_value() && fnCall.getOptDst().value())
        {
            visit(*fnCall.getOptDst().value(), false);
        }
        else
        {
            std::cout << "None";
        }
        std::cout << "\n";

        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitConstant(const TACKY::Constant &constant, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Constant(" << Constants::toString(*(constant.getConst())) << "),\n";
    }

    void visitVar(const TACKY::Var &var, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Var(" << var.getName() << ")\n";
    }

    void visitSignExtend(const TACKY::SignExtend &signExtend, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "SignExtend(\n";
        increaseIndent();
        std::cout << getIndent() << "src=";
        visit(*signExtend.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*signExtend.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitTruncate(const TACKY::Truncate &truncate, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Truncate(\n";
        increaseIndent();
        std::cout << getIndent() << "src=";
        visit(*truncate.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*truncate.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitZeroExtend(const TACKY::ZeroExtend &zeroExtend, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "ZeroExtend(\n";
        increaseIndent();
        std::cout << getIndent() << "src=";
        visit(*zeroExtend.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*zeroExtend.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitDoubleToInt(const TACKY::DoubleToInt &node, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "DoubleToInt(\n";
        increaseIndent();
        std::cout << getIndent() << "src=";
        visit(*node.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*node.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitDoubleToUInt(const TACKY::DoubleToUInt &node, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "DoubleToUInt(\n";
        increaseIndent();
        std::cout << getIndent() << "src=";
        visit(*node.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*node.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitIntToDouble(const TACKY::IntToDouble &node, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "IntToDouble(\n";
        increaseIndent();
        std::cout << getIndent() << "src=";
        visit(*node.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*node.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitUIntToDouble(const TACKY::UIntToDouble &node, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "UIntToDouble(\n";
        increaseIndent();
        std::cout << getIndent() << "src=";
        visit(*node.getSrc(), false);
        std::cout << getIndent() << "dst=";
        visit(*node.getDst(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }
};

#endif