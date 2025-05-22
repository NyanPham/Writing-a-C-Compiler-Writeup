#ifndef TACKY_H
#define TACKY_H

#include <string>
#include <memory>
#include <vector>

#include "Types.h"
#include "Const.h"
#include "Initializers.h"

/*
program = Program(top_level*)
top_level = Function(identifier name, bool global, identifier* params, Instruction* instructions)
    | StaticVariable(identifier name, bool global, Types.t t, Initializers.static_init init)
instruction = Return(val)
    | SignExtend(val src, val dst)
    | Truncate(val src, val dst)
    | ZeroExtend(val src, val dst)
    | DoubleToInt(val src, val dst)
    | DoubleToUInt(val src, val dst)
    | IntToDouble(val src, val dst)
    | UIntToDouble(val src, val dst)
    | Unary(unary_operator, val src, val dst)
    | Binary(binary_operator, val src1, val src2, val dst)
    | Copy(val src, val dst)
    | Jump(identifier target)
    | JumpIfZero(val condition, identifier target)
    | JumpIfNotZero(val condition, identifier target)
    | Label(identifier)
    | FunCall(identifier fn_name, val* args, val dst)
val = Constant(const) | Var(identifier)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
    | Equal | NotEqual | LessThan | LessOrEqual
    | GreaterThan | GreaterOrEqual
    | BitwiseAnd | BitwiseXor | BitwiseOr | BitShiftLeft | BitShiftRight
*/

namespace TACKY
{
    class Node;
    class Program;
    class TopLevel;
    class Function;
    class StaticVariable;
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
    };

    class Constant : public Val
    {
    public:
        Constant(const std::shared_ptr<Constants::Const> &value)
            : Val(NodeType::Constant), _value{value}
        {
        }
        auto &getConst() const { return _value; }

    private:
        std::shared_ptr<Constants::Const> _value;
    };

    class Var : public Val
    {
    public:
        Var(const std::string &name) : Val(NodeType::Var), _name{std::move(name)} {}
        const std::string &getName() const { return _name; }

    private:
        std::string _name;
    };

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
        FunCall(std::string fnName, std::vector<std::shared_ptr<TACKY::Val>> args, std::shared_ptr<TACKY::Val> dst)
            : Instruction(NodeType::FunCall), _fnName{fnName}, _args{args}, _dst{dst} {}

        const std::string &getFnName() const { return _fnName; }
        const std::vector<std::shared_ptr<TACKY::Val>> &getArgs() const { return _args; }
        const std::shared_ptr<TACKY::Val> &getDst() const { return _dst; }

    private:
        std::string _fnName;
        std::vector<std::shared_ptr<TACKY::Val>> _args;
        std::shared_ptr<TACKY::Val> _dst;
    };

    class Return : public Instruction
    {
    public:
        Return(std::shared_ptr<Val> value) : Instruction(NodeType::Return), _value{std::move(value)} {}
        std::shared_ptr<Val> getValue() const { return _value; }

    private:
        std::shared_ptr<Val> _value;
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
        StaticVariable(const std::string &name, bool global, const Types::DataType &t, const Initializers::StaticInit &init)
            : TopLevel(NodeType::StaticVariable), _name{std::move(name)}, _global{global}, _dataType{t}, _init{init}
        {
        }

        const std::string &getName() const { return _name; }
        bool isGlobal() const { return _global; }
        auto &getDataType() const { return _dataType; }
        auto &getInit() const { return _init; }

    private:
        std::string _name;
        bool _global;
        Types::DataType _dataType;
        Initializers::StaticInit _init;
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
};

#endif