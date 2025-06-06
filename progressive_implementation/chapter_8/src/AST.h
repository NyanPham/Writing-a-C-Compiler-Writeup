#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <optional>
#include <vector>

/*
program = Program(function_definition)
function_definition = Function(identifier name, block body)
block = Block(block_item*)
block_item = S(Statement) | D(Declaration)
declaration = Declaration(identifier name, exp? init)
for_init = InitDecl(declaration) | InitExp(exp?)
statement = Return(exp)
    | Expression(exp)
    | If(exp condition, statement then, statement? else)
    | Compound(block)
    | Break
    | Continue
    | While(exp condition, statement body, identifier id)
    | DoWhile(statement body, exp condition, identifier id)
    | For(for_init init, exp? condition, exp? post, statement body, identifier id)
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
    class Null;
    class LabeledStatement;
    class Goto;
    class ForInit;
    class InitDecl;
    class InitExp;
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
        If,
        Compound,
        Break,
        Continue,
        While,
        DoWhile,
        For,
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
        auto getElseClause() const { return _elseClause; }

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
        auto &getCondition() const { return _condition; }
        auto &getPost() const { return _post; }
        auto &getBody() const { return _body; }
        auto &getId() const { return _id; }

    private:
        std::shared_ptr<ForInit> _init;
        std::optional<std::shared_ptr<Expression>> _condition;
        std::optional<std::shared_ptr<Expression>> _post;
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
        InitDecl(std::shared_ptr<Declaration> decl)
            : ForInit(NodeType::InitDecl), _decl{std::move(decl)}
        {
        }

        auto getDecl() const { return _decl; }

    private:
        std::shared_ptr<Declaration> _decl;
    };

    class InitExp : public ForInit
    {
    public:
        InitExp(std::optional<std::shared_ptr<Expression>> exp)
            : ForInit(NodeType::InitExp), _exp{std::move(exp)}
        {
        }

        bool hasExp() const { return _exp.has_value(); }
        auto getOptExp() const { return _exp; }
        auto getExp() const { return _exp.value(); }

    private:
        std::optional<std::shared_ptr<Expression>> _exp;
    };

    class FunctionDefinition : public Node
    {
    public:
        FunctionDefinition(const std::string &name, Block body)
            : Node(NodeType::FunctionDefinition), _name{name}, _body{std::move(body)} {}
        const std::string &getName() const { return _name; }
        const Block &getBody() const { return _body; }

    private:
        std::string _name;
        Block _body;
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