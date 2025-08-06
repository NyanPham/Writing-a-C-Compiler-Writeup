#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <string>
#include <memory>
#include <vector>
#include <cstdint>

#include "Initializers.h"
#include "./utils/VariantHelper.h"

/*
program = Program(top_level*)
asm_type =
    | Byte
    | Longword
    | Quadword
    | Double
    | ByteArray(int size, int alignment)
top_level = Function(identifier name, bool global, instruction* instructions)
    | StaticVariable(identifier name, bool global, int alignment, static_init* inits)
    | StaticConstant(identifier name, int alignment, static_init init)
instruction = Mov(asm_type, operand src, operand dst)
    | Movsx(asm_type src_type, asm_type dst_type, operand src, operand dst)
    | MovZeroExtend(asm_type src_type, asm_type dst_type, operand src, operand dst)
    | Lea(operand src, operand dst)
    | Cvttsd2si(asm_type, operand src, operand dst)
    | Cvtsi2sd(asm_type, operand src, operand dst)
    | Unary(unary_operator, asm_type, operand dst)
    | Binary(binary_operator, asm_type, operand src, operand dst)
    | Cmp(asm_type, operand src, operand dst)
    | Idiv(asm_type, operand)
    | Div(asm_type, operand)
    | Cdq(asm_type)
    | Jmp(identifier)
    | JmpCC(cond_code, identifier)
    | SetCC(cond_code, operand)
    | Label(identifier)
    | Push(operand)
    | Pop(reg)
    | Call(identifier)
    | Ret
unary_operator = Neg | Not | ShrOneOp
binary_operator = Add | Sub | Mult | DivDouble | And | Or | Xor | Sal | Sar | Shl | Shr
operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Memory(reg, int) | Data(identifier, int offset) | PseudoMem(identifier, int) | Indexed(reg base, reg index, int scale)
cond_code = E | NE | L | LE | G | GE | A | AE | B | BE | P | NP
reg = AX | CX | DX | DI | SI | R8 | R9 | R10 | R11 | SP | BP | XMM0 | XMM1 | XMM2 | XMM3 | XMM4 | XMM5 | XMM6 | XMM7 | XMM14 | XMM15
*/

namespace Assembly
{
    class Node;
    class Operand;
    class Instruction;
    class Imm;
    class Reg;
    class Pseudo;
    class Memory;
    class PseudoMem;
    class Indexed;
    class Data;
    class Mov;
    class Movsx;
    class Cvttsd2si;
    class Cvtsi2sd;
    class Unary;
    class Binary;
    class Cmp;
    class Idiv;
    class Cdq;
    class Jmp;
    class JmpCC;
    class SetCC;
    class Label;
    class Push;
    class Pop;
    class Call;
    class Ret;
    class StaticVariable;
    class StaticConstant;
    class Function;
    class TopLevel;
    class Program;

    enum class NodeType
    {
        Program,
        Function,
        StaticVariable,
        StaticConstant,
        Ret,
        Mov,
        Movsx,
        MovZeroExtend,
        Lea,
        Cvttsd2si,
        Cvtsi2sd,
        Unary,
        Binary,
        Cmp,
        Idiv,
        Div,
        Cdq,
        Jmp,
        JmpCC,
        SetCC,
        Label,
        Push,
        Pop,
        Call,
        Imm,
        Reg,
        Pseudo,
        Memory,
        Data,
        PseudoMem,
        Indexed,
    };

    struct Byte
    {
        Byte() {}

        std::string toString() const { return "Byte"; }
    };

    struct Longword
    {
        Longword() {}

        std::string toString() const { return "Longword"; }
    };

    struct Quadword
    {
        Quadword() {}

        std::string toString() const { return "Quadword"; }
    };

    struct Double
    {
        Double() {}

        std::string toString() const { return "Double"; }
    };

    struct ByteArray
    {
        int size;
        int alignment;

        ByteArray(int size, int alignment) : size{size}, alignment{alignment} {}

        std::string toString() const { return "ByteArray(int=" + std::to_string(size) + ", alignment=" + std::to_string(alignment) + ")"; }
    };

    using AsmType = std::variant<Byte, Longword, Quadword, Double, ByteArray>;

    inline std::string asmTypeToString(const AsmType &type)
    {
        return std::visit([](const auto &t)
                          { return t.toString(); }, type);
    }

    inline bool isAsmByte(const AsmType &type) { return isVariant<Byte>(type); }
    inline bool isAsmLongword(const AsmType &type) { return isVariant<Longword>(type); }
    inline bool isAsmQuadword(const AsmType &type) { return isVariant<Quadword>(type); }
    inline bool isAsmDouble(const AsmType &type) { return isVariant<Double>(type); }
    inline bool isAsmByteArray(const AsmType &type) { return isVariant<ByteArray>(type); }

    inline std::optional<Byte> getByte(const AsmType &type)
    {
        return getVariant<Byte>(type);
    }

    inline std::optional<Longword> getLongword(const AsmType &type)
    {
        return getVariant<Longword>(type);
    }

    inline std::optional<Quadword> getQuadword(const AsmType &type)
    {
        return getVariant<Quadword>(type);
    }

    inline std::optional<Double> getDouble(const AsmType &type)
    {
        return getVariant<Double>(type);
    }

    inline std::optional<ByteArray> getByteArray(const AsmType &type)
    {
        return getVariant<ByteArray>(type);
    }

    enum class RegName
    {
        AX,
        BX,
        CX,
        DX,
        DI,
        SI,
        R8,
        R9,
        R10,
        R11,
        R12,
        R13,
        R14,
        R15,
        SP,
        BP,
        XMM0,
        XMM1,
        XMM2,
        XMM3,
        XMM4,
        XMM5,
        XMM6,
        XMM7,
        XMM8,
        XMM9,
        XMM10,
        XMM11,
        XMM12,
        XMM13,
        XMM14,
        XMM15,
    };

    enum class CondCode
    {
        E,
        NE,
        L,
        LE,
        G,
        GE,
        A,
        AE,
        B,
        BE,
        P,
        NP,
    };

    enum class UnaryOp
    {
        Not,
        Neg,
        ShrOneOp,
    };

    enum class BinaryOp
    {
        Add,
        Sub,
        Mult,
        DivDouble,
        And,
        Or,
        Xor,
        Sal,
        Sar,
        Shl,
        Shr,
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

    class TopLevel : public Node
    {
    public:
        TopLevel(NodeType type) : Node(type) {}
        virtual ~TopLevel() = default;
    };

    class Imm : public Operand
    {
    public:
        Imm(uint64_t value)
            : Operand(NodeType::Imm), _value{value}
        {
        }

        uint64_t getValue() const { return _value; }

    private:
        uint64_t _value;
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

    class Memory : public Operand
    {
    public:
        Memory(std::shared_ptr<Reg> reg, int offset)
            : Operand(NodeType::Memory), _reg{reg}, _offset{offset} {}

        auto &getReg() const { return _reg; }
        auto getOffset() const { return _offset; }

    private:
        std::shared_ptr<Reg> _reg;
        int _offset;
    };

    class Data : public Operand
    {
    public:
        Data(const std::string &name, int offset) : Operand(NodeType::Data), _name{name}, _offset{offset} {}
        const std::string &getName() const { return _name; }
        auto getOffset() const { return _offset; }

    private:
        std::string _name;
        int _offset;
    };

    class PseudoMem : public Operand
    {
    public:
        PseudoMem(const std::string &base, int offset)
            : Operand(NodeType::PseudoMem), _base{base}, _offset{offset} {}

        const std::string &getBase() const { return _base; }
        int getOffset() const { return _offset; }

    private:
        std::string _base;
        int _offset;
    };

    class Indexed : public Operand
    {
    public:
        Indexed(std::shared_ptr<Reg> base, std::shared_ptr<Reg> index, int scale)
            : Operand(NodeType::Indexed), _base{base}, _index{index}, _scale{scale} {}

        auto &getBase() const { return _base; }
        auto &getIndex() const { return _index; }
        int getScale() const { return _scale; }

    private:
        std::shared_ptr<Reg> _base;
        std::shared_ptr<Reg> _index;
        int _scale;
    };

    class Mov : public Instruction
    {
    public:
        Mov(std::shared_ptr<AsmType> asmType, std::shared_ptr<Operand> src, std::shared_ptr<Operand> dst)
            : Instruction(NodeType::Mov), _asmType{asmType}, _src{src}, _dst{dst}
        {
        }

        auto &getAsmType() const { return _asmType; }
        auto &getSrc() const { return _src; }
        auto &getDst() const { return _dst; }

    private:
        std::shared_ptr<AsmType> _asmType;
        std::shared_ptr<Operand> _src;
        std::shared_ptr<Operand> _dst;
    };

    class Movsx : public Instruction
    {
    public:
        Movsx(std::shared_ptr<AsmType> srcType, std::shared_ptr<AsmType> dstType, std::shared_ptr<Operand> src, std::shared_ptr<Operand> dst)
            : Instruction(NodeType::Movsx), _srcType{srcType}, _dstType{dstType}, _src{src}, _dst{dst}
        {
        }

        auto &getSrcType() const { return _srcType; }
        auto &getDstType() const { return _dstType; }
        auto &getSrc() const { return _src; }
        auto &getDst() const { return _dst; }

    private:
        std::shared_ptr<AsmType> _srcType;
        std::shared_ptr<AsmType> _dstType;
        std::shared_ptr<Operand> _src;
        std::shared_ptr<Operand> _dst;
    };

    class MovZeroExtend : public Instruction
    {
    public:
        MovZeroExtend(std::shared_ptr<AsmType> srcType, std::shared_ptr<AsmType> dstType, std::shared_ptr<Operand> src, std::shared_ptr<Operand> dst)
            : Instruction(NodeType::MovZeroExtend), _srcType{srcType}, _dstType{dstType}, _src{src}, _dst{dst}
        {
        }

        auto &getSrcType() const { return _srcType; }
        auto &getDstType() const { return _dstType; }
        auto &getSrc() const { return _src; }
        auto &getDst() const { return _dst; }

    private:
        std::shared_ptr<AsmType> _srcType;
        std::shared_ptr<AsmType> _dstType;
        std::shared_ptr<Operand> _src;
        std::shared_ptr<Operand> _dst;
    };

    class Lea : public Instruction
    {
    public:
        Lea(std::shared_ptr<Operand> src, std::shared_ptr<Operand> dst)
            : Instruction(NodeType::Lea), _src{src}, _dst{dst}
        {
        }

        auto &getSrc() const { return _src; }
        auto &getDst() const { return _dst; }

    private:
        std::shared_ptr<Operand> _src;
        std::shared_ptr<Operand> _dst;
    };

    class Cvttsd2si : public Instruction
    {
    public:
        Cvttsd2si(std::shared_ptr<AsmType> asmType, std::shared_ptr<Operand> src, std::shared_ptr<Operand> dst)
            : Instruction(NodeType::Cvttsd2si), _asmType{asmType}, _src{src}, _dst{dst}
        {
        }

        auto &getAsmType() const { return _asmType; }
        auto &getSrc() const { return _src; }
        auto &getDst() const { return _dst; }

    private:
        std::shared_ptr<AsmType> _asmType;
        std::shared_ptr<Operand> _src;
        std::shared_ptr<Operand> _dst;
    };

    class Cvtsi2sd : public Instruction
    {
    public:
        Cvtsi2sd(std::shared_ptr<AsmType> asmType, std::shared_ptr<Operand> src, std::shared_ptr<Operand> dst)
            : Instruction(NodeType::Cvtsi2sd), _asmType{asmType}, _src{src}, _dst{dst}
        {
        }

        auto &getAsmType() const { return _asmType; }
        auto &getSrc() const { return _src; }
        auto &getDst() const { return _dst; }

    private:
        std::shared_ptr<AsmType> _asmType;
        std::shared_ptr<Operand> _src;
        std::shared_ptr<Operand> _dst;
    };

    class Unary : public Instruction
    {
    public:
        Unary(UnaryOp op, std::shared_ptr<AsmType> asmType, std::shared_ptr<Operand> operand) : Instruction(NodeType::Unary), _op{op}, _asmType{asmType}, _operand{std::move(operand)} {}

        UnaryOp getOp() const { return _op; }
        auto &getAsmType() const { return _asmType; }
        auto &getOperand() const { return _operand; }

    private:
        UnaryOp _op;
        std::shared_ptr<AsmType> _asmType;
        std::shared_ptr<Operand> _operand;
    };

    class Binary : public Instruction
    {
    public:
        Binary(BinaryOp op, std::shared_ptr<AsmType> asmType, std::shared_ptr<Operand> src, std::shared_ptr<Operand> dst)
            : Instruction(NodeType::Binary), _op{op}, _asmType{asmType}, _src{src}, _dst{dst} {}

        BinaryOp getOp() const { return _op; }
        auto &getAsmType() const { return _asmType; }
        std::shared_ptr<Operand> getSrc() const { return _src; }
        std::shared_ptr<Operand> getDst() const { return _dst; }

    private:
        BinaryOp _op;
        std::shared_ptr<AsmType> _asmType;
        std::shared_ptr<Operand> _src;
        std::shared_ptr<Operand> _dst;
    };

    class Cmp : public Instruction
    {
    public:
        Cmp(std::shared_ptr<AsmType> asmType, std::shared_ptr<Operand> src, std::shared_ptr<Operand> dst)
            : Instruction(NodeType::Cmp), _asmType{asmType}, _src{std::move(src)}, _dst{std::move(dst)} {}

        auto &getAsmType() const { return _asmType; }
        auto getSrc() const { return _src; }
        auto getDst() const { return _dst; }

    private:
        std::shared_ptr<AsmType> _asmType;
        std::shared_ptr<Operand> _src;
        std::shared_ptr<Operand> _dst;
    };

    class Idiv : public Instruction
    {
    public:
        Idiv(std::shared_ptr<AsmType> asmType, std::shared_ptr<Operand> operand) : Instruction(NodeType::Idiv), _asmType{asmType}, _operand{operand} {}
        auto &getAsmType() const { return _asmType; }
        std::shared_ptr<Operand> getOperand() const { return _operand; }

    private:
        std::shared_ptr<AsmType> _asmType;
        std::shared_ptr<Operand> _operand;
    };

    class Div : public Instruction
    {
    public:
        Div(std::shared_ptr<AsmType> asmType, std::shared_ptr<Operand> operand) : Instruction(NodeType::Div), _asmType{asmType}, _operand{operand} {}
        auto &getAsmType() const { return _asmType; }
        std::shared_ptr<Operand> getOperand() const { return _operand; }

    private:
        std::shared_ptr<AsmType> _asmType;
        std::shared_ptr<Operand> _operand;
    };

    class Cdq : public Instruction
    {
    public:
        Cdq(std::shared_ptr<AsmType> asmType) : Instruction(NodeType::Cdq), _asmType{asmType} {}
        auto &getAsmType() const { return _asmType; }

    private:
        std::shared_ptr<AsmType> _asmType;
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

    class Push : public Instruction
    {
    public:
        Push(const std::shared_ptr<Operand> &operand) : Instruction(NodeType::Push), _operand{operand} {}
        const std::shared_ptr<Operand> &getOperand() const { return _operand; }

    private:
        std::shared_ptr<Operand> _operand;
    };

    class Pop : public Instruction
    {
    public:
        Pop(std::shared_ptr<Reg> reg) : Instruction(NodeType::Pop), _reg{reg} {}
        std::shared_ptr<Reg> getReg() const { return _reg; }

    private:
        std::shared_ptr<Reg> _reg;
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

    class StaticVariable : public TopLevel
    {
    public:
        StaticVariable(const std::string &name, bool global, int alignment, std::vector<std::shared_ptr<Initializers::StaticInit>> inits)
            : TopLevel(NodeType::StaticVariable), _name{name}, _global{global}, _alignment{alignment}, _inits{inits} {}

        const std::string &getName() const { return _name; }
        bool isGlobal() const { return _global; }
        auto &getAlignment() const { return _alignment; }
        auto &getInits() const { return _inits; }

    private:
        std::string _name;
        bool _global;
        int _alignment;
        std::vector<std::shared_ptr<Initializers::StaticInit>> _inits;
    };

    class StaticConstant : public TopLevel
    {
    public:
        StaticConstant(const std::string &name, int alignment, Initializers::StaticInit init)
            : TopLevel(NodeType::StaticConstant), _name{name}, _alignment{alignment}, _init{init} {}

        const std::string &getName() const { return _name; }
        auto &getAlignment() const { return _alignment; }
        auto &getInit() const { return _init; }

    private:
        std::string _name;
        int _alignment;
        Initializers::StaticInit _init;
    };

    class Function : public TopLevel
    {
    public:
        Function(const std::string &name, bool global, const std::vector<std::shared_ptr<Instruction>> &instructions)
            : TopLevel(NodeType::Function), _name{name}, _global{global}, _instructions{instructions}
        {
        }

        const std::string &getName() const { return _name; }
        bool isGlobal() const { return _global; }
        const std::vector<std::shared_ptr<Instruction>> &getInstructions() const { return _instructions; }

    private:
        std::string _name;
        bool _global;
        std::vector<std::shared_ptr<Instruction>> _instructions;
    };

    class Program : public Node
    {
    public:
        Program(const std::vector<std::shared_ptr<TopLevel>> &topLevels) : Node(NodeType::Program), _topLevels{topLevels} {}

        const std::vector<std::shared_ptr<TopLevel>> &getTopLevels() const { return _topLevels; }

    private:
        std::vector<std::shared_ptr<TopLevel>> _topLevels;
    };

    // We use structs and variant to represent AsmType.
    // Comparison operators for STL containers
    // REVIEW: These operators are correct for use in std::set and std::map.
    // - Operand equality and ordering is based on type and the underlying value (e.g., RegName for Reg, string for Pseudo).
    // - Reg equality and ordering is based on RegName.
    // - This ensures that sets/maps of Operand or Reg will behave as expected.
    inline bool operator==(const Operand &lhs, const Operand &rhs)
    {
        if (lhs.getType() != rhs.getType())
            return false;
        switch (lhs.getType())
        {
        case NodeType::Pseudo:
            return static_cast<const Pseudo &>(lhs).getName() == static_cast<const Pseudo &>(rhs).getName();
        case NodeType::Reg:
            return static_cast<const Reg &>(lhs).getName() == static_cast<const Reg &>(rhs).getName();
        case NodeType::Imm:
            return static_cast<const Imm &>(lhs).getValue() == static_cast<const Imm &>(rhs).getValue();
        case NodeType::Memory:
            return static_cast<const Memory &>(lhs).getOffset() == static_cast<const Memory &>(rhs).getOffset() &&
                   *static_cast<const Memory &>(lhs).getReg() == *static_cast<const Memory &>(rhs).getReg();
        case NodeType::Data:
            return static_cast<const Data &>(lhs).getName() == static_cast<const Data &>(rhs).getName() &&
                   static_cast<const Data &>(lhs).getOffset() == static_cast<const Data &>(rhs).getOffset();
        case NodeType::PseudoMem:
            return static_cast<const PseudoMem &>(lhs).getBase() == static_cast<const PseudoMem &>(rhs).getBase() &&
                   static_cast<const PseudoMem &>(lhs).getOffset() == static_cast<const PseudoMem &>(rhs).getOffset();
        case NodeType::Indexed:
            return *static_cast<const Indexed &>(lhs).getBase() == *static_cast<const Indexed &>(rhs).getBase() &&
                   *static_cast<const Indexed &>(lhs).getIndex() == *static_cast<const Indexed &>(rhs).getIndex() &&
                   static_cast<const Indexed &>(lhs).getScale() == static_cast<const Indexed &>(rhs).getScale();
        default:
            return false;
        }
    }

    inline bool operator<(const Operand &lhs, const Operand &rhs)
    {
        if (lhs.getType() != rhs.getType())
            return lhs.getType() < rhs.getType();
        switch (lhs.getType())
        {
        case NodeType::Pseudo:
            return static_cast<const Pseudo &>(lhs).getName() < static_cast<const Pseudo &>(rhs).getName();
        case NodeType::Reg:
            return static_cast<const Reg &>(lhs).getName() < static_cast<const Reg &>(rhs).getName();
        case NodeType::Imm:
            return static_cast<const Imm &>(lhs).getValue() < static_cast<const Imm &>(rhs).getValue();
        case NodeType::Memory:
            if (*static_cast<const Memory &>(lhs).getReg() != *static_cast<const Memory &>(rhs).getReg())
                return *static_cast<const Memory &>(lhs).getReg() < *static_cast<const Memory &>(rhs).getReg();
            return static_cast<const Memory &>(lhs).getOffset() < static_cast<const Memory &>(rhs).getOffset();
        case NodeType::Data:
            if (static_cast<const Data &>(lhs).getName() != static_cast<const Data &>(rhs).getName())
                return static_cast<const Data &>(lhs).getName() < static_cast<const Data &>(rhs).getName();
            return static_cast<const Data &>(lhs).getOffset() < static_cast<const Data &>(rhs).getOffset();
        case NodeType::PseudoMem:
            if (static_cast<const PseudoMem &>(lhs).getBase() != static_cast<const PseudoMem &>(rhs).getBase())
                return static_cast<const PseudoMem &>(lhs).getBase() < static_cast<const PseudoMem &>(rhs).getBase();
            return static_cast<const PseudoMem &>(lhs).getOffset() < static_cast<const PseudoMem &>(rhs).getOffset();
        case NodeType::Indexed:
            if (*static_cast<const Indexed &>(lhs).getBase() != *static_cast<const Indexed &>(rhs).getBase())
                return *static_cast<const Indexed &>(lhs).getBase() < *static_cast<const Indexed &>(rhs).getBase();
            if (*static_cast<const Indexed &>(lhs).getIndex() != *static_cast<const Indexed &>(rhs).getIndex())
                return *static_cast<const Indexed &>(lhs).getIndex() < *static_cast<const Indexed &>(rhs).getIndex();
            return static_cast<const Indexed &>(lhs).getScale() < static_cast<const Indexed &>(rhs).getScale();
        default:
            return false;
        }
    }

    inline bool operator==(const Reg &lhs, const Reg &rhs)
    {
        return lhs.getName() == rhs.getName();
    }

    inline bool operator<(const Reg &lhs, const Reg &rhs)
    {
        return lhs.getName() < rhs.getName();
    }
    // We could use enums, but types might have fields later on, like pointer, array, struct types, similar to FunType in Types.h
}

#endif