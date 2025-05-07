#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <optional>
#include <vector>
#include <map>

/*
program = Program(function_declarations*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, exp? init)
function_declaration = (identifier name, identifier* params, block? body)
block = Block(block_item*)
block_item = S(Statement) | D(Declaration)
for_init = InitDecl(variable_declaration) | InitExp(exp?)
statement = Return(exp)
    | Expression(exp)
    | If(exp condition, statement then, statement? else)
    | Compound(block)
    | Break
    | Continue
    | While(exp condition, statement body, identifier id)
    | DoWhile(statement body, exp condition, identifier id)
    | For(for_init init, exp? condition, exp? post, statement body, identifier id)
    | Switch(exp control, statement body, Map<int?, identifier> cases, identifier id)
    | Case(exp, statement body, identifier id)
    | Default(statement body, identifier id)
    | Null
    | LabeledStatement(indentifier label, statement)
    | Goto(identifier label)
exp = Constant(int)
    | Var(identifier)
    | Unary(unary_operator, exp)
    | Binary(binary_operator, exp, exp)
    | Assignment(exp, exp)
    | CompoundAssignment(binary_operator, exp, exp)
    | PostfixIncr(exp)
    | PostfixDecr(exp)
    | Conditional(exp condition, exp then, exp else)
    | FunctionCall(identifier, exp* args)
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
    class Conditional;
    class Binary;
    class Unary;
    class Return;
    class ExpressionStmt;
    class If;
    class Compound;
    class Break;
    class Continue;
    class While;
    class DoWhile;
    class For;
    class Switch;
    class Case;
    class Default;
    class Null;
    class LabeledStatement;
    class Goto;
    class FunctionCall;
    class ForInit;
    class InitDecl;
    class InitExp;
    class Expression;
    class Statement;
    class Declaration;
    class FunctionDeclaration;
    class VariableDeclaration;
    class BlockItem;
    class Program;

    enum class NodeType
    {
        Program,
        VariableDeclaration,
        FunctionDeclaration,
        Return,
        ExpressionStmt,
        If,
        Compound,
        Break,
        Continue,
        While,
        DoWhile,
        For,
        Switch,
        Case,
        Default,
        Null,
        LabeledStatement,
        Goto,
        Constant,
        Unary,
        Binary,
        Var,
        Assignment,
        CompoundAssignment,
        PostfixIncr,
        PostfixDecr,
        Conditional,
        FunctionCall,
        InitDecl,
        InitExp,
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

    using Block = std::vector<std::shared_ptr<BlockItem>>;
    using CaseMap = std::map<std::optional<int>, std::string>;

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

    class ForInit : public Node
    {
    public:
        ForInit(NodeType type) : Node(type) {}
        virtual ~ForInit() = default;
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
        Declaration(NodeType type) : BlockItem(type) {}
        virtual ~Declaration() = default;
    };

    class VariableDeclaration : public Declaration
    {
    public:
        VariableDeclaration(std::string name, std::optional<std::shared_ptr<Expression>> init)
            : Declaration(NodeType::VariableDeclaration), _name{std::move(name)}, _init{std::move(init)}
        {
        }

        const std::string &getName() const { return _name; }
        const std::optional<std::shared_ptr<Expression>> &getOptInit() const { return _init; }

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

    class Conditional : public Expression
    {
    public:
        Conditional(std::shared_ptr<Expression> condition, std::shared_ptr<Expression> then, std::shared_ptr<Expression> elseExp)
            : Expression(NodeType::Conditional), _condition{condition}, _then{then}, _else{elseExp} {}

        auto getCondition() const { return _condition; }
        auto getThen() const { return _then; }
        auto getElse() const { return _else; }

    private:
        std::shared_ptr<Expression> _condition;
        std::shared_ptr<Expression> _then;
        std::shared_ptr<Expression> _else;
    };

    class FunctionCall : public Expression
    {
    public:
        FunctionCall(const std::string &fnName, std::vector<std::shared_ptr<Expression>> args)
            : Expression(NodeType::FunctionCall), _fnName{fnName}, _args{args}
        {
        }

        const std::string &getName() const { return _fnName; }
        const std::vector<std::shared_ptr<Expression>> &getArgs() const { return _args; }

    private:
        std::string _fnName;
        std::vector<std::shared_ptr<Expression>> _args;
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

    class If : public Statement
    {
    public:
        If(std::shared_ptr<AST::Expression> condition, std::shared_ptr<AST::Statement> thenClause, std::optional<std::shared_ptr<AST::Statement>> elseClause)
            : Statement(NodeType::If), _condition{condition}, _thenClause{thenClause}, _elseClause{elseClause} {}

        auto getCondition() const { return _condition; }
        auto getThenClause() const { return _thenClause; }
        auto getOptElseClause() const { return _elseClause; }

    private:
        std::shared_ptr<AST::Expression> _condition;
        std::shared_ptr<AST::Statement> _thenClause;
        std::optional<std::shared_ptr<AST::Statement>> _elseClause;
    };

    class Compound : public Statement
    {
    public:
        Compound(const Block &block) : Statement(NodeType::Compound), _block{std::move(block)} {}
        auto getBlock() const { return _block; }

    private:
        Block _block;
    };

    class Break : public Statement
    {
    public:
        Break(const std::string &id)
            : Statement(NodeType::Break), _id{std::move(id)} {}

        const std::string &getId() const { return _id; }

    private:
        const std::string _id;
    };

    class Continue : public Statement
    {
    public:
        Continue(const std::string &id)
            : Statement(NodeType::Continue), _id{std::move(id)} {}

        const std::string &getId() const { return _id; }

    private:
        const std::string _id;
    };

    class While : public Statement
    {
    public:
        While(std::shared_ptr<Expression> condition, std::shared_ptr<Statement> body, const std::string &id)
            : Statement(NodeType::While), _condition{condition}, _body{body}, _id{id} {}

        const std::shared_ptr<Expression> &getCondition() const { return _condition; }
        const std::shared_ptr<Statement> &getBody() const { return _body; }
        const std::string &getId() const { return _id; }

    private:
        std::shared_ptr<Expression> _condition;
        std::shared_ptr<Statement> _body;
        std::string _id;
    };

    class DoWhile : public Statement
    {
    public:
        DoWhile(std::shared_ptr<Statement> body, std::shared_ptr<Expression> condition, const std::string &id)
            : Statement(NodeType::DoWhile), _body{body}, _condition{condition}, _id{id} {}

        const std::shared_ptr<Statement> &getBody() const { return _body; }
        const std::shared_ptr<Expression> &getCondition() const { return _condition; }
        const std::string &getId() const { return _id; }

    private:
        std::shared_ptr<Statement> _body;
        std::shared_ptr<Expression> _condition;
        std::string _id;
    };

    class For : public Statement
    {
    public:
        For(std::shared_ptr<ForInit> init,
            std::optional<std::shared_ptr<Expression>> condition,
            std::optional<std::shared_ptr<Expression>> post,
            std::shared_ptr<Statement> body,
            std::string id)
            : Statement(NodeType::For), _init{std::move(init)}, _condition{std::move(condition)}, _post{std::move(post)}, _body{std::move(body)}, _id{std::move(id)}
        {
        }

        bool hasCondition() { return _condition.has_value(); }
        bool hasPost() { return _post.has_value(); }

        auto &getInit() const { return _init; }
        auto &getOptCondition() const { return _condition; }
        auto &getOptPost() const { return _post; }
        auto &getBody() const { return _body; }
        auto &getId() const { return _id; }

    private:
        std::shared_ptr<ForInit> _init;
        std::optional<std::shared_ptr<Expression>> _condition;
        std::optional<std::shared_ptr<Expression>> _post;
        std::shared_ptr<Statement> _body;
        std::string _id;
    };

    class Switch : public Statement
    {
    public:
        Switch(
            std::shared_ptr<Expression> control,
            std::shared_ptr<Statement> body,
            std::optional<CaseMap> cases,
            std::string id)
            : Statement(NodeType::Switch), _control{control}, _body{body}, _cases{cases}, _id{id} {}

        const auto &getControl() const { return _control; }
        const auto &getBody() const { return _body; }
        const std::optional<CaseMap> &getOptCases() const { return _cases; }
        const auto &getId() const { return _id; }

    private:
        std::shared_ptr<Expression> _control;
        std::shared_ptr<Statement> _body;
        std::optional<CaseMap> _cases;
        std::string _id;
    };

    class Case : public Statement
    {
    public:
        Case(
            std::shared_ptr<Expression> value,
            std::shared_ptr<Statement> body,
            std::string id)
            : Statement(NodeType::Case), _value{value}, _body{body}, _id{id}
        {
        }

        const auto &getValue() const { return _value; }
        const auto &getBody() const { return _body; }
        const auto &getId() const { return _id; }

    private:
        std::shared_ptr<Expression> _value;
        std::shared_ptr<Statement> _body;
        std::string _id;
    };

    class Default : public Statement
    {
    public:
        Default(
            std::shared_ptr<Statement> body,
            std::string id)
            : Statement(NodeType::Default), _body{body}, _id{id}
        {
        }

        const auto &getBody() const { return _body; }
        const auto &getId() const { return _id; }

    private:
        std::shared_ptr<Statement> _body;
        std::string _id;
    };

    class Null : public Statement
    {
    public:
        Null() : Statement(NodeType::Null) {}
    };

    class LabeledStatement : public Statement
    {
    public:
        LabeledStatement(const std::string &label, std::shared_ptr<AST::Statement> statement)
            : Statement(NodeType::LabeledStatement), _label{std::move(label)}, _statement{statement}
        {
        }
        auto getLabel() const { return _label; }
        auto getStatement() const { return _statement; }

    private:
        std::string _label;
        std::shared_ptr<AST::Statement> _statement;
    };

    class Goto : public Statement
    {
    public:
        Goto(const std::string &label)
            : Statement(NodeType::Goto), _label{std::move(label)}
        {
        }

        auto getLabel() const { return _label; }

    private:
        std::string _label;
    };

    class InitDecl : public ForInit
    {
    public:
        InitDecl(std::shared_ptr<VariableDeclaration> decl)
            : ForInit(NodeType::InitDecl), _decl{std::move(decl)}
        {
        }

        auto getDecl() const { return _decl; }

    private:
        std::shared_ptr<VariableDeclaration> _decl;
    };

    class InitExp : public ForInit
    {
    public:
        InitExp(std::optional<std::shared_ptr<Expression>> exp)
            : ForInit(NodeType::InitExp), _exp{std::move(exp)}
        {
        }

        auto getOptExp() const { return _exp; }

    private:
        std::optional<std::shared_ptr<Expression>> _exp;
    };

    class FunctionDeclaration : public Declaration
    {
    public:
        FunctionDeclaration(const std::string &name, std::vector<std::string> params, std::optional<Block> body)
            : Declaration(NodeType::FunctionDeclaration), _name{name}, _params{params}, _body{body} {}

        const std::string &getName() const { return _name; }
        const std::vector<std::string> &getParams() const { return _params; }
        const std::optional<Block> &getOptBody() const { return _body; }

    private:
        std::string _name;
        std::vector<std::string> _params;
        std::optional<Block> _body;
    };

    class Program : public Node
    {
    public:
        Program(std::vector<std::shared_ptr<FunctionDeclaration>> fnDecls)
            : Node(NodeType::Program), _fnDecls{fnDecls} {}

        const std::vector<std::shared_ptr<FunctionDeclaration>> &getFunctionDeclarations() const { return _fnDecls; }

    private:
        std::vector<std::shared_ptr<FunctionDeclaration>> _fnDecls;
    };
}

#endif