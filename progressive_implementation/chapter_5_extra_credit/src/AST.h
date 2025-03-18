#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <optional>
#include <vector>

/*
program = Program(function_definition)
function_definition = Function(identifier name, block_item* body)
block_item = S(Statement) | D(Declaration)
declaration = Declaration(identifier name, exp? init)
statement = Return(exp) | Expression(exp) | Null
exp = Constant(int)
    | Var(identifier)
    | Unary(unary_operator, exp)
    | Binary(binary_operator, exp, exp)
    | Assignment(exp, exp)
    | CompoundAssignment(binary_operator, exp, exp)
    | PostfixIncr(exp)
    | PostfixDecr(exp)
unary_operator = Complement | Negate | Not | Incr | Decr
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
    | Equal | NotEqual | LessThan | LessOrEqual
    | GreaterThan | GreaterOrEqual
    | BitwiseAnd | BitwiseXor | BitwiseOr | BitShiftLeft | BitShiftRight
*/

namespace AST
{
    class Node;
    class Constant;
    class Var;
    class Assignment;
    class CompoundAssignment;
    class PostfixIncr;
    class PostfixDecr;
    class Binary;
    class Unary;
    class Return;
    class ExpressionStmt;
    class Null;
    class Expression;
    class Statement;
    class Declaration;
    class BlockItem;
    class FunctionDefinition;
    class Program;

    enum class NodeType
    {
        Program,
        FunctionDefinition,
        Declaration,
        Return,
        ExpressionStmt,
        Null,
        Constant,
        Unary,
        Binary,
        Var,
        Assignment,
        CompoundAssignment,
        PostfixIncr,
        PostfixDecr,
    };

    enum class UnaryOp
    {
        Complement,
        Negate,
        Not,
        Incr,
        Decr,
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

    class BlockItem : public Node
    {
    public:
        BlockItem(NodeType type) : Node(type) {}
        virtual ~BlockItem() = default;
    };

    class Statement : public BlockItem
    {
    public:
        Statement(NodeType type) : BlockItem(type) {}
        virtual ~Statement() = default;
    };

    class Declaration : public BlockItem
    {
    public:
        Declaration(std::string name, std::optional<std::shared_ptr<Expression>> init)
            : BlockItem(NodeType::Declaration), _name{std::move(name)}, _init{std::move(init)}
        {
        }

        const std::string &getName() const { return _name; }
        const std::optional<std::shared_ptr<Expression>> &getInit() const { return _init; }

    private:
        std::string _name;
        std::optional<std::shared_ptr<Expression>> _init;
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

    class Var : public Expression
    {
    public:
        Var(const std::string &name) : Expression(NodeType::Var), _name{std::move(name)} {}
        auto getName() const { return _name; }

    private:
        std::string _name;
    };

    class Assignment : public Expression
    {
    public:
        Assignment(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
            : Expression(NodeType::Assignment), _left{std::move(left)}, _right{std::move(right)}
        {
        }

        auto getLeftExp() const { return _left; }
        auto getRightExp() const { return _right; }

    private:
        std::shared_ptr<Expression> _left;
        std::shared_ptr<Expression> _right;
    };

    class CompoundAssignment : public Expression
    {
    public:
        CompoundAssignment(BinaryOp op, std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
            : Expression(NodeType::CompoundAssignment), _op{op}, _left{std::move(left)}, _right{std::move(right)}
        {
        }

        auto getOp() const { return _op; }
        auto getLeftExp() const { return _left; }
        auto getRightExp() const { return _right; }

    private:
        BinaryOp _op;
        std::shared_ptr<Expression> _left;
        std::shared_ptr<Expression> _right;
    };

    class PostfixIncr : public Expression
    {
    public:
        PostfixIncr(std::shared_ptr<Expression> exp)
            : Expression(NodeType::PostfixIncr), _exp{std::move(exp)}
        {
        }

        auto getExp() const { return _exp; }

    private:
        std::shared_ptr<Expression> _exp;
    };

    class PostfixDecr : public Expression
    {
    public:
        PostfixDecr(std::shared_ptr<Expression> exp)
            : Expression(NodeType::PostfixDecr), _exp{std::move(exp)}
        {
        }

        auto getExp() const { return _exp; }

    private:
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

    class ExpressionStmt : public Statement
    {
    public:
        ExpressionStmt(std::shared_ptr<Expression> exp)
            : Statement(NodeType::ExpressionStmt), _exp{std::move(exp)} {}

        const std::shared_ptr<Expression> &getExp() const { return _exp; }

    private:
        std::shared_ptr<Expression> _exp;
    };

    class Null : public Statement
    {
    public:
        Null() : Statement(NodeType::Null) {}
    };

    class FunctionDefinition : public Node
    {
    public:
        FunctionDefinition(const std::string &name, std::vector<std::shared_ptr<BlockItem>> body)
            : Node(NodeType::FunctionDefinition), _name{name}, _body{std::move(body)} {}
        const std::string &getName() const { return _name; }
        const std::vector<std::shared_ptr<BlockItem>> &getBody() const { return _body; }

    private:
        std::string _name;
        std::vector<std::shared_ptr<BlockItem>> _body;
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