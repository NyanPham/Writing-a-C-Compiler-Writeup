#ifndef TACKY_H
#define TACKY_H

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <stdexcept>
#include "Types.h"
#include "Const.h"
#include "Initializers.h"
#include "Symbols.h"

namespace TACKY
{
    class Node;
    class Program;
    class TopLevel;
    class Function;
    class StaticVariable;
    class StaticConstant;
    class Instruction;
    class Return;
    class SignExtend;
    class DoubleToInt;
    class DoubleToUInt;
    class IntToDouble;
    class UIntToDouble;
    class Truncate;
    class ZeroExtend;
    class Unary;
    class Binary;
    class Copy;
    class GetAddress;
    class Load;
    class Store;
    class AddPtr;
    class CopyToOffset;
    class CopyFromOffset;
    class Jump;
    class JumpIfZero;
    class JumpIfNotZero;
    class Label;
    class FunCall;
    class Val;
    class Constant;
    class Var;

    enum class NodeType
    {
        Program,
        Function,
        StaticVariable,
        StaticConstant,
        Return,
        SignExtend,
        Truncate,
        ZeroExtend,
        DoubleToInt,
        DoubleToUInt,
        IntToDouble,
        UIntToDouble,
        Unary,
        Binary,
        Copy,
        GetAddress,
        Load,
        Store,
        AddPtr,
        CopyToOffset,
        CopyFromOffset,
        Jump,
        JumpIfZero,
        JumpIfNotZero,
        Label,
        FunCall,
        Constant,
        Var,
    };

    enum class UnaryOp
    {
        Complement,
        Negate,
        Not,
    };

    enum class BinaryOp
    {
        Add,
        Subtract,
        Multiply,
        Divide,
        Remainder,
        And,
        Or,
        Equal,
        NotEqual,
        LessThan,
        LessOrEqual,
        GreaterThan,
        GreaterOrEqual,
        BitwiseAnd,
        BitwiseOr,
        BitwiseXor,
        BitShiftLeft,
        BitShiftRight,
    };

    class Node
    {
    public:
        Node(NodeType type) : _type{type} {}
        NodeType getType() const { return _type; }
        virtual ~Node() = default;

    private:
        NodeType _type;
    };

    class Instruction : public Node
    {
    public:
        Instruction(NodeType type) : Node(type) {}
        virtual ~Instruction() override = default;
    };

    class TopLevel : public Node
    {
    public:
        TopLevel(NodeType type) : Node(type) {}
        virtual ~TopLevel() override = default;
    };

    class Val : public Node
    {
    public:
        Val(NodeType type) : Node(type) {}
        virtual ~Val() override = default;

        virtual bool equals(const Val &other) const = 0;
        virtual bool lessThan(const Val &other) const = 0;
    };

    class Constant : public Val
    {
    public:
        Constant(const std::shared_ptr<Constants::Const> &value)
            : Val(NodeType::Constant), _value{value}
        {
        }
        auto &getConst() const { return _value; }

        bool equals(const Val &other) const override
        {
            if (auto *o = dynamic_cast<const Constant *>(&other))
                return *this->_value == *o->_value;
            return false;
        }

        bool lessThan(const Val &other) const override
        {
            if (auto *o = dynamic_cast<const Constant *>(&other))
                return *this->_value < *o->_value;
            return true;
        }

    private:
        std::shared_ptr<Constants::Const> _value;
    };

    class Var : public Val
    {
    public:
        Var(const std::string &name) : Val(NodeType::Var), _name{std::move(name)} {}
        const std::string &getName() const { return _name; }

        bool equals(const Val &other) const override
        {
            if (auto *o = dynamic_cast<const Var *>(&other))
                return this->_name == o->_name;
            return false;
        }

        bool lessThan(const Val &other) const override
        {
            if (auto *o = dynamic_cast<const Var *>(&other))
                return this->_name < o->_name;
            return false;
        }

    private:
        std::string _name;
    };

    inline bool operator==(const Val &a, const Val &b) { return a.equals(b); }
    inline bool operator<(const Val &a, const Val &b) { return a.lessThan(b); }

    class Unary : public Instruction
    {
    public:
        Unary(UnaryOp op, std::shared_ptr<Val> src, std::shared_ptr<Val> dst)
            : Instruction(NodeType::Unary), _op{op}, _src{std::move(src)}, _dst{std::move(dst)} {}

        UnaryOp getOp() const { return _op; }
        std::shared_ptr<Val> getSrc() const { return _src; }
        std::shared_ptr<Val> getDst() const { return _dst; }

    private:
        UnaryOp _op;
        std::shared_ptr<Val> _src;
        std::shared_ptr<Val> _dst;
    };

    class Binary : public Instruction
    {
    public:
        Binary(BinaryOp op, std::shared_ptr<Val> src1, std::shared_ptr<Val> src2, std::shared_ptr<Val> dst)
            : Instruction(NodeType::Binary), _op{op}, _src1{std::move(src1)}, _src2{std::move(src2)}, _dst{std::move(dst)}
        {
        }

        BinaryOp getOp() const { return _op; }
        std::shared_ptr<Val> getSrc1() const { return _src1; }
        std::shared_ptr<Val> getSrc2() const { return _src2; }
        std::shared_ptr<Val> getDst() const { return _dst; }

    private:
        BinaryOp _op;
        std::shared_ptr<Val> _src1;
        std::shared_ptr<Val> _src2;
        std::shared_ptr<Val> _dst;
    };

    class Copy : public Instruction
    {
    public:
        Copy(std::shared_ptr<Val> src, std::shared_ptr<Val> dst)
            : Instruction(NodeType::Copy), _src{std::move(src)}, _dst{std::move(dst)}
        {
        }
        std::shared_ptr<Val> getSrc() const { return _src; }
        std::shared_ptr<Val> getDst() const { return _dst; }

    private:
        std::shared_ptr<Val> _src;
        std::shared_ptr<Val> _dst;
    };

    class GetAddress : public Instruction
    {
    public:
        GetAddress(std::shared_ptr<Val> src, std::shared_ptr<Val> dst)
            : Instruction(NodeType::GetAddress), _src{std::move(src)}, _dst{std::move(dst)}
        {
        }
        std::shared_ptr<Val> getSrc() const { return _src; }
        std::shared_ptr<Val> getDst() const { return _dst; }

    private:
        std::shared_ptr<Val> _src;
        std::shared_ptr<Val> _dst;
    };

    class Load : public Instruction
    {
    public:
        Load(std::shared_ptr<Val> srcPtr, std::shared_ptr<Val> dst)
            : Instruction(NodeType::Load), _srcPtr{std::move(srcPtr)}, _dst{std::move(dst)}
        {
        }
        std::shared_ptr<Val> getSrcPtr() const { return _srcPtr; }
        std::shared_ptr<Val> getDst() const { return _dst; }

    private:
        std::shared_ptr<Val> _srcPtr;
        std::shared_ptr<Val> _dst;
    };

    class Store : public Instruction
    {
    public:
        Store(std::shared_ptr<Val> src, std::shared_ptr<Val> dstPtr)
            : Instruction(NodeType::Store), _src{std::move(src)}, _dstPtr{std::move(dstPtr)}
        {
        }
        std::shared_ptr<Val> getSrc() const { return _src; }
        std::shared_ptr<Val> getDstPtr() const { return _dstPtr; }

    private:
        std::shared_ptr<Val> _src;
        std::shared_ptr<Val> _dstPtr;
    };

    class AddPtr : public Instruction
    {
    public:
        AddPtr(std::shared_ptr<Val> ptr, std::shared_ptr<Val> index, int scale, std::shared_ptr<Val> dst)
            : Instruction(NodeType::AddPtr), _ptr{std::move(ptr)}, _index{std::move(index)}, _scale{scale}, _dst{std::move(dst)}
        {
        }
        std::shared_ptr<Val> getPtr() const { return _ptr; }
        std::shared_ptr<Val> getIndex() const { return _index; }
        int getScale() const { return _scale; }
        std::shared_ptr<Val> getDst() const { return _dst; }

    private:
        std::shared_ptr<Val> _ptr;
        std::shared_ptr<Val> _index;
        int _scale;
        std::shared_ptr<Val> _dst;
    };

    class CopyToOffset : public Instruction
    {
    public:
        CopyToOffset(std::shared_ptr<Val> src, const std::string &dst, ssize_t offset)
            : Instruction(NodeType::CopyToOffset), _src{std::move(src)}, _dst{std::move(dst)}, _offset{offset} {}

        std::shared_ptr<Val> getSrc() const { return _src; }
        const std::string &getDst() const { return _dst; }
        ssize_t getOffset() const { return _offset; }

    private:
        std::shared_ptr<Val> _src;
        std::string _dst;
        ssize_t _offset;
    };

    class CopyFromOffset : public Instruction
    {
    public:
        CopyFromOffset(const std::string &src, ssize_t offset, std::shared_ptr<Val> dst)
            : Instruction(NodeType::CopyFromOffset), _src{std::move(src)}, _offset{offset}, _dst{std::move(dst)} {}

        const std::string &getSrc() const { return _src; }
        ssize_t getOffset() const { return _offset; }
        std::shared_ptr<Val> getDst() const { return _dst; }

    private:
        std::string _src;
        ssize_t _offset;
        std::shared_ptr<Val> _dst;
    };

    class Jump : public Instruction
    {
    public:
        Jump(const std::string &target)
            : Instruction(NodeType::Jump), _target{std::move(target)} {}
        const std::string &getTarget() const { return _target; }

    private:
        std::string _target;
    };

    class JumpIfZero : public Instruction
    {
    public:
        JumpIfZero(std::shared_ptr<Val> cond, const std::string &target)
            : Instruction(NodeType::JumpIfZero), _cond{std::move(cond)}, _target{std::move(target)}
        {
        }
        const std::shared_ptr<Val> getCond() const { return _cond; }
        const std::string &getTarget() const { return _target; }

    private:
        std::shared_ptr<Val> _cond;
        std::string _target;
    };

    class JumpIfNotZero : public Instruction
    {
    public:
        JumpIfNotZero(std::shared_ptr<Val> cond, const std::string &target)
            : Instruction(NodeType::JumpIfNotZero), _cond{std::move(cond)}, _target{std::move(target)}
        {
        }
        const std::shared_ptr<Val> getCond() const { return _cond; }
        const std::string &getTarget() const { return _target; }

    private:
        std::shared_ptr<Val> _cond;
        std::string _target;
    };

    class Label : public Instruction
    {
    public:
        Label(const std::string &name)
            : Instruction(NodeType::Label), _name{std::move(name)} {}

        const std::string &getName() const { return _name; }

    private:
        std::string _name;
    };

    class FunCall : public Instruction
    {
    public:
        FunCall(std::string fnName, std::vector<std::shared_ptr<TACKY::Val>> args, std::optional<std::shared_ptr<TACKY::Val>> dst = std::nullopt)
            : Instruction(NodeType::FunCall), _fnName{fnName}, _args{args}, _dst{dst} {}

        auto &getFnName() const { return _fnName; }
        auto &getArgs() const { return _args; }
        auto &getOptDst() const { return _dst; }

    private:
        std::string _fnName;
        std::vector<std::shared_ptr<TACKY::Val>> _args;
        std::optional<std::shared_ptr<TACKY::Val>> _dst;
    };

    class Return : public Instruction
    {
    public:
        Return(std::optional<std::shared_ptr<Val>> value = std::nullopt) : Instruction(NodeType::Return), _value{std::move(value)} {}
        auto &getOptValue() const { return _value; }

    private:
        std::optional<std::shared_ptr<Val>> _value;
    };

    class SignExtend : public Instruction
    {
    public:
        SignExtend(const std::shared_ptr<Val> src, const std::shared_ptr<Val> dst) : Instruction(NodeType::SignExtend), _src{src}, _dst{dst} {}

        auto &getSrc() const { return _src; }
        auto &getDst() const { return _dst; }

    private:
        std::shared_ptr<Val> _src;
        std::shared_ptr<Val> _dst;
    };

    class Truncate : public Instruction
    {
    public:
        Truncate(const std::shared_ptr<Val> src, const std::shared_ptr<Val> dst) : Instruction(NodeType::Truncate), _src{src}, _dst{dst} {}

        auto &getSrc() const { return _src; }
        auto &getDst() const { return _dst; }

    private:
        std::shared_ptr<Val> _src;
        std::shared_ptr<Val> _dst;
    };

    class ZeroExtend : public Instruction
    {
    public:
        ZeroExtend(const std::shared_ptr<Val> src, const std::shared_ptr<Val> dst) : Instruction(NodeType::ZeroExtend), _src{src}, _dst{dst} {}

        auto &getSrc() const { return _src; }
        auto &getDst() const { return _dst; }

    private:
        std::shared_ptr<Val> _src;
        std::shared_ptr<Val> _dst;
    };

    class DoubleToInt : public Instruction
    {
    public:
        DoubleToInt(const std::shared_ptr<Val> src, const std::shared_ptr<Val> dst) : Instruction(NodeType::DoubleToInt), _src{src}, _dst{dst} {}

        auto &getSrc() const { return _src; }
        auto &getDst() const { return _dst; }

    private:
        std::shared_ptr<Val> _src;
        std::shared_ptr<Val> _dst;
    };

    class DoubleToUInt : public Instruction
    {
    public:
        DoubleToUInt(const std::shared_ptr<Val> src, const std::shared_ptr<Val> dst) : Instruction(NodeType::DoubleToUInt), _src{src}, _dst{dst} {}

        auto &getSrc() const { return _src; }
        auto &getDst() const { return _dst; }

    private:
        std::shared_ptr<Val> _src;
        std::shared_ptr<Val> _dst;
    };

    class IntToDouble : public Instruction
    {
    public:
        IntToDouble(const std::shared_ptr<Val> src, const std::shared_ptr<Val> dst) : Instruction(NodeType::IntToDouble), _src{src}, _dst{dst} {}

        auto &getSrc() const { return _src; }
        auto &getDst() const { return _dst; }

    private:
        std::shared_ptr<Val> _src;
        std::shared_ptr<Val> _dst;
    };

    class UIntToDouble : public Instruction
    {
    public:
        UIntToDouble(const std::shared_ptr<Val> src, const std::shared_ptr<Val> dst) : Instruction(NodeType::UIntToDouble), _src{src}, _dst{dst} {}

        auto &getSrc() const { return _src; }
        auto &getDst() const { return _dst; }

    private:
        std::shared_ptr<Val> _src;
        std::shared_ptr<Val> _dst;
    };

    class StaticVariable : public TopLevel
    {
    public:
        StaticVariable(const std::string &name, bool global, const Types::DataType &t, const std::vector<std::shared_ptr<Initializers::StaticInit>> &inits)
            : TopLevel(NodeType::StaticVariable), _name{std::move(name)}, _global{global}, _dataType{t}, _inits{inits}
        {
        }

        const std::string &getName() const { return _name; }
        bool isGlobal() const { return _global; }
        auto &getDataType() const { return _dataType; }
        auto &getInits() const { return _inits; }

    private:
        std::string _name;
        bool _global;
        Types::DataType _dataType;
        std::vector<std::shared_ptr<Initializers::StaticInit>> _inits;
    };

    class StaticConstant : public TopLevel
    {
    public:
        StaticConstant(const std::string &name, const Types::DataType &t, const std::shared_ptr<Initializers::StaticInit> &init)
            : TopLevel(NodeType::StaticConstant), _name{std::move(name)}, _dataType{t}, _init{init}
        {
        }

        const std::string &getName() const { return _name; }
        auto &getDataType() const { return _dataType; }
        auto &getInit() const { return _init; }

    private:
        std::string _name;
        Types::DataType _dataType;
        std::shared_ptr<Initializers::StaticInit> _init;
    };

    class Function : public TopLevel
    {
    public:
        Function(const std::string &name, bool global, const std::vector<std::string> &params, std::vector<std::shared_ptr<Instruction>> instructions)
            : TopLevel(NodeType::Function), _name{std::move(name)}, _global{global}, _params{params}, _instructions{std::move(instructions)}
        {
        }

        const std::string &getName() const { return _name; }
        bool isGlobal() const { return _global; }
        const std::vector<std::string> &getParams() const { return _params; }
        const std::vector<std::shared_ptr<Instruction>> &getInstructions() const { return _instructions; }
        void setInstructions(const std::vector<std::shared_ptr<Instruction>> &instructions) { _instructions = instructions; }

    private:
        std::string _name;
        bool _global;
        std::vector<std::string> _params;
        std::vector<std::shared_ptr<Instruction>> _instructions;
    };

    class Program : public Node
    {
    public:
        Program(std::vector<std::shared_ptr<TopLevel>> topLevels) : Node(NodeType::Program), _topLevels{std::move(topLevels)} {}
        const std::vector<std::shared_ptr<TopLevel>> &getTopLevels() const { return _topLevels; }

    private:
        std::vector<std::shared_ptr<TopLevel>> _topLevels;
    };

    inline Types::DataType typeOfVal(const std::shared_ptr<Val> &v, const Symbols::SymbolTable &symbolTable)
    {
        if (auto c = std::dynamic_pointer_cast<Constant>(v))
        {
            return Constants::typeOfConst(*c->getConst());
        }
        if (auto var = std::dynamic_pointer_cast<Var>(v))
        {
            return symbolTable.get(var->getName()).type;
        }
        throw std::runtime_error("TackyGen::typeOfVal: unsupported Val subclass");
    }

    inline bool equalInstructions(const Instruction &a, const Instruction &b)
    {
        if (a.getType() != b.getType())
            return false;

        switch (a.getType())
        {
        case NodeType::Unary:
        {
            auto &ua = static_cast<const Unary &>(a);
            auto &ub = static_cast<const Unary &>(b);
            return ua.getOp() == ub.getOp() && *ua.getSrc() == *ub.getSrc() && *ua.getDst() == *ub.getDst();
        }
        case NodeType::Binary:
        {
            auto &ba = static_cast<const Binary &>(a);
            auto &bb = static_cast<const Binary &>(b);
            return ba.getOp() == bb.getOp() && *ba.getSrc1() == *bb.getSrc1() && *ba.getSrc2() == *bb.getSrc2() && *ba.getDst() == *bb.getDst();
        }
        case NodeType::Copy:
        {
            auto &ca = static_cast<const Copy &>(a);
            auto &cb = static_cast<const Copy &>(b);
            return *ca.getSrc() == *cb.getSrc() && *ca.getDst() == *cb.getDst();
        }
        case NodeType::GetAddress:
        {
            auto &ga = static_cast<const GetAddress &>(a);
            auto &gb = static_cast<const GetAddress &>(b);
            return *ga.getSrc() == *gb.getSrc() && *ga.getDst() == *gb.getDst();
        }
        case NodeType::Load:
        {
            auto &la = static_cast<const Load &>(a);
            auto &lb = static_cast<const Load &>(b);
            return *la.getSrcPtr() == *lb.getSrcPtr() && *la.getDst() == *lb.getDst();
        }
        case NodeType::Store:
        {
            auto &sa = static_cast<const Store &>(a);
            auto &sb = static_cast<const Store &>(b);
            return *sa.getSrc() == *sb.getSrc() && *sa.getDstPtr() == *sb.getDstPtr();
        }
        case NodeType::AddPtr:
        {
            auto &aa = static_cast<const AddPtr &>(a);
            auto &ab = static_cast<const AddPtr &>(b);
            return *aa.getPtr() == *ab.getPtr() && *aa.getIndex() == *ab.getIndex() && aa.getScale() == ab.getScale() && *aa.getDst() == *ab.getDst();
        }
        case NodeType::CopyToOffset:
        {
            auto &oa = static_cast<const CopyToOffset &>(a);
            auto &ob = static_cast<const CopyToOffset &>(b);
            return *oa.getSrc() == *ob.getSrc() && oa.getDst() == ob.getDst() && oa.getOffset() == ob.getOffset();
        }
        case NodeType::CopyFromOffset:
        {
            auto &oa = static_cast<const CopyFromOffset &>(a);
            auto &ob = static_cast<const CopyFromOffset &>(b);
            return oa.getSrc() == ob.getSrc() && oa.getOffset() == ob.getOffset() && *oa.getDst() == *ob.getDst();
        }
        case NodeType::Jump:
        {
            auto &ja = static_cast<const Jump &>(a);
            auto &jb = static_cast<const Jump &>(b);
            return ja.getTarget() == jb.getTarget();
        }
        case NodeType::JumpIfZero:
        {
            auto &ja = static_cast<const JumpIfZero &>(a);
            auto &jb = static_cast<const JumpIfZero &>(b);
            return *ja.getCond() == *jb.getCond() && ja.getTarget() == jb.getTarget();
        }
        case NodeType::JumpIfNotZero:
        {
            auto &ja = static_cast<const JumpIfNotZero &>(a);
            auto &jb = static_cast<const JumpIfNotZero &>(b);
            return *ja.getCond() == *jb.getCond() && ja.getTarget() == jb.getTarget();
        }
        case NodeType::Label:
        {
            auto &la = static_cast<const Label &>(a);
            auto &lb = static_cast<const Label &>(b);
            return la.getName() == lb.getName();
        }
        case NodeType::FunCall:
        {
            auto &fa = static_cast<const FunCall &>(a);
            auto &fb = static_cast<const FunCall &>(b);
            if (fa.getFnName() != fb.getFnName())
                return false;
            if (fa.getArgs().size() != fb.getArgs().size())
                return false;
            for (size_t i = 0; i < fa.getArgs().size(); ++i)
            {
                if (*fa.getArgs()[i] != *fb.getArgs()[i])
                    return false;
            }
            if (fa.getOptDst().has_value() != fb.getOptDst().has_value())
                return false;
            if (fa.getOptDst() && fb.getOptDst())
            {
                if (*(*fa.getOptDst()) != *(*fb.getOptDst()))
                    return false;
            }
            return true;
        }
        case NodeType::Return:
        {
            auto &ra = static_cast<const Return &>(a);
            auto &rb = static_cast<const Return &>(b);
            if (ra.getOptValue().has_value() != rb.getOptValue().has_value())
                return false;
            if (ra.getOptValue() && rb.getOptValue())
            {
                if (*(*ra.getOptValue()) != *(*rb.getOptValue()))
                    return false;
            }
            return true;
        }
        case NodeType::SignExtend:
        case NodeType::Truncate:
        case NodeType::ZeroExtend:
        case NodeType::DoubleToInt:
        case NodeType::DoubleToUInt:
        case NodeType::IntToDouble:
        case NodeType::UIntToDouble:
        {
            // All these have src and dst
            auto &ia = static_cast<const Instruction &>(a);
            auto &ib = static_cast<const Instruction &>(b);
            // Use getSrc/getDst for each
            auto getSrc = [](const Instruction &instr) -> const std::shared_ptr<Val> &
            {
                if (auto *p = dynamic_cast<const SignExtend *>(&instr))
                    return p->getSrc();
                if (auto *p = dynamic_cast<const Truncate *>(&instr))
                    return p->getSrc();
                if (auto *p = dynamic_cast<const ZeroExtend *>(&instr))
                    return p->getSrc();
                if (auto *p = dynamic_cast<const DoubleToInt *>(&instr))
                    return p->getSrc();
                if (auto *p = dynamic_cast<const DoubleToUInt *>(&instr))
                    return p->getSrc();
                if (auto *p = dynamic_cast<const IntToDouble *>(&instr))
                    return p->getSrc();
                if (auto *p = dynamic_cast<const UIntToDouble *>(&instr))
                    return p->getSrc();
                throw std::logic_error("Unknown src/dst instruction type");
            };
            auto getDst = [](const Instruction &instr) -> const std::shared_ptr<Val> &
            {
                if (auto *p = dynamic_cast<const SignExtend *>(&instr))
                    return p->getDst();
                if (auto *p = dynamic_cast<const Truncate *>(&instr))
                    return p->getDst();
                if (auto *p = dynamic_cast<const ZeroExtend *>(&instr))
                    return p->getDst();
                if (auto *p = dynamic_cast<const DoubleToInt *>(&instr))
                    return p->getDst();
                if (auto *p = dynamic_cast<const DoubleToUInt *>(&instr))
                    return p->getDst();
                if (auto *p = dynamic_cast<const IntToDouble *>(&instr))
                    return p->getDst();
                if (auto *p = dynamic_cast<const UIntToDouble *>(&instr))
                    return p->getDst();
                throw std::logic_error("Unknown src/dst instruction type");
            };
            return *getSrc(ia) == *getSrc(ib) && *getDst(ia) == *getDst(ib);
        }
        default:
            return false;
        }
    };
}

#endif