#include "OptimizeUtils.h"

namespace OptimizeUtils
{

    std::optional<std::shared_ptr<TACKY::Val>> getDst(const std::shared_ptr<TACKY::Instruction> &instr)
    {
        using namespace TACKY;
        switch (instr->getType())
        {
        case NodeType::Copy:
            return std::static_pointer_cast<Copy>(instr)->getDst();
        case NodeType::FunCall:
        {
            auto fn = std::static_pointer_cast<FunCall>(instr);
            if (fn->getOptDst())
                return fn->getOptDst().value();
            return std::nullopt;
        }
        case NodeType::Unary:
            return std::static_pointer_cast<Unary>(instr)->getDst();
        case NodeType::Binary:
            return std::static_pointer_cast<Binary>(instr)->getDst();
        case NodeType::SignExtend:
            return std::static_pointer_cast<SignExtend>(instr)->getDst();
        case NodeType::ZeroExtend:
            return std::static_pointer_cast<ZeroExtend>(instr)->getDst();
        case NodeType::DoubleToInt:
            return std::static_pointer_cast<DoubleToInt>(instr)->getDst();
        case NodeType::DoubleToUInt:
            return std::static_pointer_cast<DoubleToUInt>(instr)->getDst();
        case NodeType::UIntToDouble:
            return std::static_pointer_cast<UIntToDouble>(instr)->getDst();
        case NodeType::IntToDouble:
            return std::static_pointer_cast<IntToDouble>(instr)->getDst();
        case NodeType::Truncate:
            return std::static_pointer_cast<Truncate>(instr)->getDst();
        case NodeType::GetAddress:
            return std::static_pointer_cast<GetAddress>(instr)->getDst();
        case NodeType::Load:
            return std::static_pointer_cast<Load>(instr)->getDst();
        case NodeType::AddPtr:
            return std::static_pointer_cast<AddPtr>(instr)->getDst();
        case NodeType::CopyToOffset:
        {
            auto c2o = std::static_pointer_cast<CopyToOffset>(instr);
            // CopyToOffset's dst is a string, wrap as Var
            return std::make_shared<Var>(c2o->getDst());
        }
        case NodeType::CopyFromOffset:
            return std::static_pointer_cast<CopyFromOffset>(instr)->getDst();
        default:
            // Store, Return, Jump, JumpIfZero, JumpIfNotZero, Label, etc.
            return std::nullopt;
        }
    }

    bool isStatic(const std::string &varName, const Symbols::SymbolTable &symbolTable)
    {
        auto sym = symbolTable.get(varName);
        return Symbols::isStaticAttr(sym.attrs);
    }

}