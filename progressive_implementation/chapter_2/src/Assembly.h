#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <string>
#include <memory>
#include <vector>

/*
program = Program(function_definition)
function_definition = Function(identifier name, instruction* instructions)
instruction = Mov(operand src, operand dst)
    | Unary(unary_operator, operand dst)
    | AllocateStack(int)
    | Ret
unary_operator = Neg | Not
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Stack(int)
reg = AX | R10
*/

namespace Assembly
{
    class Node;
    class Operand;
    class Instruction;
    class Imm;
    class Reg;
    class Pseudo;
    class Stack;
    class Mov;
    class Unary;
    class AllocateStack;
    class Ret;
    class Function;
    class Program;

    enum class NodeType
    {
        Program,
        Function,
        Ret,
        Mov,
        Unary,
        AllocateStack,
        Imm,
        Reg,
        Pseudo,
        Stack,
    };

    enum class RegName
    {
        AX,
        R10,
    };

    enum class UnaryOp
    {
        Not,
        Neg,
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

    class Operand : public Node
    {
    public:
        Operand(NodeType type) : Node(type) {}
        virtual ~Operand() = default;
    };

    class Instruction : public Node
    {
    public:
        Instruction(NodeType type) : Node(type) {}
        virtual ~Instruction() = default;
    };

    class Imm : public Operand
    {
    public:
        Imm(int value)
            : Operand(NodeType::Imm), _value{value}
        {
        }

        int getValue() const { return _value; }

    private:
        int _value;
    };

    class Reg : public Operand
    {
    public:
        Reg(RegName name) : Operand(NodeType::Reg), _name{name} {}
        RegName getName() const { return _name; }

    private:
        RegName _name;
    };

    class Pseudo : public Operand
    {
    public:
        Pseudo(const std::string &name) : Operand(NodeType::Pseudo), _name{name} {}
        const std::string &getName() const { return _name; }

    private:
        std::string _name;
    };

    class Stack : public Operand
    {
    public:
        Stack(int offset) : Operand(NodeType::Stack), _offset{offset} {}
        int getOffset() const { return _offset; }

    private:
        int _offset;
    };

    class Mov : public Instruction
    {
    public:
        Mov(std::shared_ptr<Operand> src, std::shared_ptr<Operand> dst)
            : Instruction(NodeType::Mov), _src{src}, _dst{dst}
        {
        }

        std::shared_ptr<Operand> getSrc() const { return _src; }
        std::shared_ptr<Operand> getDst() const { return _dst; }

    private:
        std::shared_ptr<Operand> _src;
        std::shared_ptr<Operand> _dst;
    };

    class Unary : public Instruction
    {
    public:
        Unary(UnaryOp op, std::shared_ptr<Operand> operand) : Instruction(NodeType::Unary), _op{op}, _operand{std::move(operand)} {}
        UnaryOp getOp() const { return _op; }
        std::shared_ptr<Operand> getOperand() const { return _operand; }

    private:
        UnaryOp _op;
        std::shared_ptr<Operand> _operand;
    };

    class AllocateStack : public Instruction
    {
    public:
        AllocateStack(int offset) : Instruction(NodeType::AllocateStack), _offset{offset} {}
        int getOffset() const { return _offset; }

    private:
        int _offset;
    };

    class Ret : public Instruction
    {
    public:
        Ret() : Instruction(NodeType::Ret) {}
    };

    class Function : public Node
    {
    public:
        Function(const std::string &name, const std::vector<std::shared_ptr<Instruction>> &instructions)
            : Node(NodeType::Function), _name{name}, _instructions{instructions}
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
        Program(std::shared_ptr<Function> fun) : Node(NodeType::Program), _fun{fun} {}

        std::shared_ptr<Function> getFunction() const { return _fun; }

    private:
        std::shared_ptr<Function> _fun;
    };
}

#endif