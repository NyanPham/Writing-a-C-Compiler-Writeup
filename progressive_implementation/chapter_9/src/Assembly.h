#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <string>
#include <memory>
#include <vector>

/*
program = Program(function_definition*)
function_definition = Function(identifier name, instruction* instructions)
instruction = Mov(operand src, operand dst)
    | Unary(unary_operator, operand dst)
    | Binary(binary_operator, operand src, operand dst)
    | Cmp(operand src, operand dst)
    | Idiv(operand)
    | Cdq
    | Jmp(identifier)
    | JmpCC(cond_code, identifier)
    | SetCC(cond_code, operand)
    | Label(identifier)
    | AllocateStack(int)
    | DeallocateStack(int)
    | Push(operand)
    | Call(identifier)
    | Ret
unary_operator = Neg | Not
binary_operator = Add | Sub | Mult | And | Or | Xor | Sal | Sar
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Stack(int)
cond_code = E | NE | L | LE | G | GE
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11
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
    class Binary;
    class Cmp;
    class Idiv;
    class Cdq;
    class Jmp;
    class JmpCC;
    class SetCC;
    class Label;
    class AllocateStack;
    class DeallocateStack;
    class Push;
    class Call;
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
        Binary,
        Cmp,
        Idiv,
        Cdq,
        Jmp,
        JmpCC,
        SetCC,
        Label,
        AllocateStack,
        DeallocateStack,
        Push,
        Call,
        Imm,
        Reg,
        Pseudo,
        Stack,
    };

    enum class RegName
    {
        AX,
        DX,
        CX,
        DI,
        SI,
        R8,
        R9,
        R10,
        R11,
    };

    enum class CondCode
    {
        E,
        NE,
        L,
        LE,
        G,
        GE
    };

    enum class UnaryOp
    {
        Not,
        Neg,
    };

    enum class BinaryOp
    {
        Add,
        Sub,
        Mult,
        And,
        Or,
        Xor,
        Sal,
        Sar,
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

    class Binary : public Instruction
    {
    public:
        Binary(BinaryOp op, std::shared_ptr<Operand> src, std::shared_ptr<Operand> dst)
            : Instruction(NodeType::Binary), _op{op}, _src{src}, _dst{dst} {}

        BinaryOp getOp() const { return _op; }
        std::shared_ptr<Operand> getSrc() const { return _src; }
        std::shared_ptr<Operand> getDst() const { return _dst; }

    private:
        BinaryOp _op;
        std::shared_ptr<Operand> _src;
        std::shared_ptr<Operand> _dst;
    };

    class Cmp : public Instruction
    {
    public:
        Cmp(std::shared_ptr<Operand> src, std::shared_ptr<Operand> dst)
            : Instruction(NodeType::Cmp), _src{std::move(src)}, _dst{std::move(dst)} {}

        auto getSrc() const { return _src; }
        auto getDst() const { return _dst; }

    private:
        std::shared_ptr<Operand> _src;
        std::shared_ptr<Operand> _dst;
    };

    class Idiv : public Instruction
    {
    public:
        Idiv(std::shared_ptr<Operand> operand) : Instruction(NodeType::Idiv), _operand{operand} {}
        std::shared_ptr<Operand> getOperand() const { return _operand; }

    private:
        std::shared_ptr<Operand> _operand;
    };

    class Cdq : public Instruction
    {
    public:
        Cdq() : Instruction(NodeType::Cdq) {}
    };

    class Jmp : public Instruction
    {
    public:
        Jmp(const std::string &target)
            : Instruction(NodeType::Jmp), _target{std::move(target)} {}

        auto getTarget() const { return _target; }

    private:
        std::string _target;
    };

    class JmpCC : public Instruction
    {
    public:
        JmpCC(CondCode condCode, const std::string &target)
            : Instruction(NodeType::JmpCC), _condCode{condCode}, _target{std::move(target)} {}

        auto getCondCode() const { return _condCode; }
        auto getTarget() const { return _target; }

    private:
        CondCode _condCode;
        std::string _target;
    };

    class SetCC : public Instruction
    {
    public:
        SetCC(CondCode condCode, std::shared_ptr<Operand> operand)
            : Instruction(NodeType::SetCC), _condCode{condCode}, _operand{std::move(operand)} {}

        auto getCondCode() const { return _condCode; }
        auto getOperand() const { return _operand; }

    private:
        CondCode _condCode;
        std::shared_ptr<Operand> _operand;
    };

    class Label : public Instruction
    {
    public:
        Label(const std::string &name)
            : Instruction(NodeType::Label), _name{std::move(name)} {}

        auto getName() const { return _name; }

    private:
        std::string _name;
    };

    class AllocateStack : public Instruction
    {
    public:
        AllocateStack(int offset) : Instruction(NodeType::AllocateStack), _offset{offset} {}
        int getOffset() const { return _offset; }

    private:
        int _offset;
    };

    class DeallocateStack : public Instruction
    {
    public:
        DeallocateStack(int offset) : Instruction(NodeType::DeallocateStack), _offset{offset} {}
        int getOffset() const { return _offset; }

    private:
        int _offset;
    };

    class Push : public Instruction
    {
    public:
        Push(const std::shared_ptr<Operand> &operand) : Instruction(NodeType::Push), _operand{operand} {}
        const std::shared_ptr<Operand> &getOperand() const { return _operand; }

    private:
        std::shared_ptr<Operand> _operand;
    };

    class Call : public Instruction
    {
    public:
        Call(const std::string &fnName) : Instruction(NodeType::Call), _fnName{fnName} {}
        const std::string &getFnName() const { return _fnName; }

    private:
        std::string _fnName;
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
        Program(const std::vector<std::shared_ptr<Function>> &fns) : Node(NodeType::Program), _fns{fns} {}

        const std::vector<std::shared_ptr<Function>> &getFunctions() const { return _fns; }

    private:
        std::vector<std::shared_ptr<Function>> _fns;
    };
}

#endif