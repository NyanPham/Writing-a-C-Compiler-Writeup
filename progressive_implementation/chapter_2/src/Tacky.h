#ifndef TACKY_H
#define TACKY_H

#include <string>
#include <memory>
#include <vector>

/*
program = Program(function_definition)
function_definition = Function(identifier name, Instruction* instructions)
instruction = Return(val) | Unary(unary_operator, val src, val dst)
val = Constant(int) | Var(identifier)
unary_operator = Complement | Negate
*/

namespace TACKY
{
    enum class NodeType
    {
        Program,
        Function,
        Return,
        Unary,
        Constant,
        Var,
    };

    class Node;
    class Program;
    class Function;
    class Instruction;
    class Return;
    class Unary;
    class Val;
    class Constant;
    class Var;

    enum class UnaryOp
    {
        Complement,
        Negate,
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