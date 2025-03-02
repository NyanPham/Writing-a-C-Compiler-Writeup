#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <string>
#include <memory>
#include <vector>

/*
program = Program(function_definition)
function_definition = Function(identifier name, instruction* instructions)
instruction = Mov(operand src, operand dst) | Ret
operand = Imm(int) | Register
*/

namespace Assembly
{
    class Node;
    class Operand;
    class Instruction;
    class Imm;
    class Register;
    class Mov;
    class Ret;
    class Function;
    class Program;

    enum class NodeType
    {
        Program,
        Function,
        Ret,
        Mov,
        Imm,
        Register,
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

    class Register : public Operand
    {
    public:
        Register() : Operand(NodeType::Register) {}
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

    class Ret : public Instruction
    {
    public:
        Ret() : Instruction(NodeType::Ret) {}
    };

    class Function : public Node
    {
    public:
        Function(std::string &&name, std::vector<std::shared_ptr<Instruction>> &&instructions)
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
        Program(std::shared_ptr<Function> fun) : Node(NodeType::Program), _fun{fun} {}

        std::shared_ptr<Function> getFunction() const { return _fun; }

    private:
        std::shared_ptr<Function> _fun;
    };
}

#endif