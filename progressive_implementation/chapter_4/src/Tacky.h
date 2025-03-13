#ifndef TACKY_H
#define TACKY_H

#include <string>
#include <memory>
#include <vector>

/*
program = Program(function_definition)
function_definition = Function(identifier name, Instruction* instructions)
instruction = Return(val)
    | Unary(unary_operator, val src, val dst)
    | Binary(binary_operator, val src1, val src2, val dst)
    | Copy(val src, val dst)
    | Jump(identifier target)
    | JumpIfZero(val condition, identifier target)
    | JumpIfNotZero(val condition, identifier target)
    | Label(identifier)
val = Constant(int) | Var(identifier)
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
    class Function;
    class Instruction;
    class Return;
    class Unary;
    class Binary;
    class Copy;
    class Jump;
    class JumpIfZero;
    class JumpIfNotZero;
    class Label;
    class Val;
    class Constant;
    class Var;

    enum class NodeType
    {
        Program,
        Function,
        Return,
        Unary,
        Binary,
        Copy,
        Jump,
        JumpIfZero,
        JumpIfNotZero,
        Label,
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

    class Val : public Node
    {
    public:
        Val(NodeType type) : Node(type) {}
        virtual ~Val() override = default;
    };

    class Constant : public Val
    {
    public:
        Constant(int value)
            : Val(NodeType::Constant), _value{value}
        {
        }
        int getValue() const { return _value; }

    private:
        int _value;
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

    class Return : public Instruction
    {
    public:
        Return(std::shared_ptr<Val> value) : Instruction(NodeType::Return), _value{std::move(value)} {}
        std::shared_ptr<Val> getValue() const { return _value; }

    private:
        std::shared_ptr<Val> _value;
    };

    class Function : public Node
    {
    public:
        Function(const std::string &name, std::vector<std::shared_ptr<Instruction>> instructions)
            : Node(NodeType::Function), _name{std::move(name)}, _instructions{std::move(instructions)}
        {
        }

        const std::string &getName() const { return _name; }
        const std::vector<std::shared_ptr<Instruction>> &getInstructions() const { return _instructions; }

    private:
        std::string _name;
        std::vector<std::shared_ptr<Instruction>> _instructions;
    };

    class Program : public Node
    {
    public:
        Program(std::shared_ptr<Function> function) : Node(NodeType::Program), _function{std::move(function)} {}
        const std::shared_ptr<Function> &getFunction() const { return _function; }

    private:
        std::shared_ptr<Function> _function;
    };
};

#endif