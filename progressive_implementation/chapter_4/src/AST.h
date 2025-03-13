#ifndef AST_H
#define AST_H

#include <string>
#include <memory>

/*
program = Program(function_definition)
function_definition = Function(identifier name, statement body)
statement = Return(exp)
exp = Constant(int)
    | Unary(unary_operator, exp)
    | Binary(binary_operator, exp, exp)
unary_operator = Complement | Negate | Not
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
    | Equal | NotEqual | LessThan | LessOrEqual
    | GreaterThan | GreaterOrEqual
    | BitwiseAnd | BitwiseXor | BitwiseOr | BitShiftLeft | BitShiftRight
*/

namespace AST
{
    class Node;
    class Constant;
    class Binary;
    class Unary;
    class Return;
    class Expression;
    class Statement;
    class FunctionDefinition;
    class Program;

    enum class NodeType
    {
        Program,
        FunctionDefinition,
        Return,
        Constant,
        Unary,
        Binary,
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

    class Expression : public Node
    {
    public:
        Expression(NodeType type) : Node(type) {}
        virtual ~Expression() = default;
    };

    class Statement : public Node
    {
    public:
        Statement(NodeType type) : Node(type) {}
        virtual ~Statement() = default;
    };

    class Constant : public Expression
    {
    public:
        Constant(int value)
            : Expression(NodeType::Constant), _value{value} {}
        int getValue() const { return _value; }

    private:
        int _value;
    };

    class Binary : public Expression
    {
    public:
        Binary(BinaryOp op, std::shared_ptr<Expression> exp1, std::shared_ptr<Expression> exp2)
            : Expression(NodeType::Binary), _op{op}, _exp1{std::move(exp1)}, _exp2{std::move(exp2)} {}

        BinaryOp getOp() const { return _op; }
        std::shared_ptr<Expression> getExp1() const { return _exp1; }
        std::shared_ptr<Expression> getExp2() const { return _exp2; }

    private:
        BinaryOp _op;
        std::shared_ptr<Expression> _exp1;
        std::shared_ptr<Expression> _exp2;
    };

    class Unary : public Expression
    {
    public:
        Unary(UnaryOp op, std::shared_ptr<Expression> exp)
            : Expression(NodeType::Unary), _op{op}, _exp{exp} {}

        UnaryOp getOp() const { return _op; }
        std::shared_ptr<Expression> getExp() const { return _exp; }

    private:
        UnaryOp _op;
        std::shared_ptr<Expression> _exp;
    };

    class Return : public Statement
    {
    public:
        Return(std::shared_ptr<Expression> value)
            : Statement(NodeType::Return), _value{value} {}
        std::shared_ptr<Expression> getValue() const { return _value; }

    private:
        std::shared_ptr<Expression> _value;
    };

    class FunctionDefinition : public Node
    {
    public:
        FunctionDefinition(const std::string &name, std::shared_ptr<Statement> body)
            : Node(NodeType::FunctionDefinition), _name{name}, _body{body} {}
        const std::string &getName() const { return _name; }
        std::shared_ptr<Statement> getBody() const { return _body; }

    private:
        std::string _name;
        std::shared_ptr<Statement> _body;
    };

    class Program : public Node
    {
    public:
        Program(std::shared_ptr<FunctionDefinition> funDef)
            : Node(NodeType::Program), _funDef{funDef} {}
        std::shared_ptr<FunctionDefinition> getFunctionDefinition() const { return _funDef; }

    private:
        std::shared_ptr<FunctionDefinition> _funDef;
    };
}

#endif