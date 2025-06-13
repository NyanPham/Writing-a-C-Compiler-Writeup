#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <optional>
#include <vector>
#include <map>

#include "Types.h"
#include "Const.h"

/*
program = Program(declaration*)
declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)
variable_declaration = (identifier name, initializer? init, type var_type, storage_class?)
function_declaration = (identifier name, identifier* params, block? body, type fun_type, storage_class?)
initializer = SingleInit(exp) | CompoundInit(initializer* list)
type = Char | SChar | UChar | Int | Long | UInt | ULong | Double
    | FunType(type* params, type ret)
    | Array(type elem_type, int size)
    | PointerType(type referenced)
storage_class = Static | Extern
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
exp = Constant(const, type)
    | String(string)
    | Var(identifier, type)
    | Unary(unary_operator, exp, type)
    | Binary(binary_operator, exp, exp, type)
    | Assignment(exp, exp, type)
    | CompoundAssignment(binary_operator, exp, exp, type)
    | PostfixIncr(exp, type)
    | PostfixDecr(exp, type)
    | Conditional(exp condition, exp then, exp else, type)
    | FunctionCall(identifier, exp* args, type)
    | Dereference(exp)
    | AddrOf(exp)
    | Subscript(exp, exp, type)
unary_operator = Complement | Negate | Not | Incr | Decr
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
    | Equal | NotEqual | LessThan | LessOrEqual
    | GreaterThan | GreaterOrEqual
    | BitwiseAnd | BitwiseXor | BitwiseOr | BitShiftLeft | BitShiftRight
const = ConstInt(int) | ConstLong(int) | ConstUInt(int) | ConstULong(int) | ConstDouble(d)
    | ConstChar(int) | ConstUChar(int)
*/

namespace AST
{
    class Node;
    class Constant;
    class String;
    class Cast;
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
    class Dereference;
    class AddrOf;
    class Subscript;
    class ForInit;
    class InitDecl;
    class InitExp;
    class Expression;
    class Statement;
    class Declaration;
    class SingleInit;
    class CompoundInit;
    class Initializer;
    class FunctionDeclaration;
    class VariableDeclaration;
    class BlockItem;
    class Program;

    enum class NodeType
    {
        Program,
        VariableDeclaration,
        FunctionDeclaration,
        SingleInit,
        CompoundInit,
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
        String,
        Cast,
        Unary,
        Binary,
        Var,
        Assignment,
        CompoundAssignment,
        PostfixIncr,
        PostfixDecr,
        Conditional,
        FunctionCall,
        Dereference,
        AddrOf,
        Subscript,
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

    enum class StorageClass
    {
        Static,
        Extern,
    };

    using Block = std::vector<std::shared_ptr<BlockItem>>;
    using CaseMap = std::map<std::optional<std::shared_ptr<Constants::Const>>, std::string, struct CaseMapComparator>;

    struct CaseMapComparator
    {
        bool operator()(const std::optional<std::shared_ptr<Constants::Const>> &lhs,
                        const std::optional<std::shared_ptr<Constants::Const>> &rhs) const
        {
            if (!lhs && !rhs)
                return false;
            if (!lhs)
                return true;
            if (!rhs)
                return false;

            const auto &lhsVal = *lhs.value();
            const auto &rhsVal = *rhs.value();

            if (Constants::isConstInt(lhsVal) && Constants::isConstInt(rhsVal))
            {
                return Constants::getConstInt(lhsVal)->val < Constants::getConstInt(rhsVal)->val;
            }
            else if (Constants::isConstLong(lhsVal) && Constants::isConstLong(rhsVal))
            {
                return Constants::getConstLong(lhsVal)->val < Constants::getConstLong(rhsVal)->val;
            }
            else if (Constants::isConstUInt(lhsVal) && Constants::isConstUInt(rhsVal))
            {
                return Constants::getConstUInt(lhsVal)->val < Constants::getConstUInt(rhsVal)->val;
            }
            else if (Constants::isConstULong(lhsVal) && Constants::isConstULong(rhsVal))
            {
                return Constants::getConstULong(lhsVal)->val < Constants::getConstULong(rhsVal)->val;
            }
            else
            {
                return Constants::isConstInt(lhsVal);
            }
        }
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
        Expression(NodeType type, std::optional<Types::DataType> dataType = std::nullopt) : Node(type), _dataType{dataType} {}
        virtual ~Expression() = default;

        const std::optional<Types::DataType> &getDataType() const { return _dataType; }
        void setDataType(const std::optional<Types::DataType> &dataType) { _dataType = dataType; }

    private:
        std::optional<Types::DataType> _dataType = std::nullopt;
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

    class Initializer : public Node
    {
    public:
        Initializer(NodeType type, std::optional<Types::DataType> dataType = std::nullopt) : Node(type), _dataType{dataType} {}
        virtual ~Initializer() = default;

        const std::optional<Types::DataType> &getDataType() const { return _dataType; }
        void setDataType(const std::optional<Types::DataType> &dataType) { _dataType = dataType; }

    private:
        std::optional<Types::DataType> _dataType = std::nullopt;
    };

    class SingleInit : public Initializer
    {
    public:
        SingleInit(std::shared_ptr<Expression> exp) : Initializer(NodeType::SingleInit), _exp{std::move(exp)} {}

        auto &getExp() const { return _exp; }

    private:
        std::shared_ptr<Expression> _exp;
    };

    class CompoundInit : public Initializer
    {
    public:
        CompoundInit(std::vector<std::shared_ptr<Initializer>> inits) : Initializer(NodeType::CompoundInit), _inits{std::move(inits)} {}

        auto &getInits() const { return _inits; }

    private:
        std::vector<std::shared_ptr<Initializer>> _inits;
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
        VariableDeclaration(const std::string &name, std::optional<std::shared_ptr<Initializer>> init, const Types::DataType &varType, std::optional<StorageClass> storageClass = std::nullopt)
            : Declaration(NodeType::VariableDeclaration), _name{std::move(name)}, _init{std::move(init)}, _varType{varType}, _storageClass{storageClass}
        {
        }

        const std::string &getName() const { return _name; }
        const std::optional<std::shared_ptr<Initializer>> &getOptInit() const { return _init; }
        const Types::DataType &getVarType() const { return _varType; }
        const std::optional<StorageClass> &getOptStorageClass() const { return _storageClass; }

    private:
        std::string _name;
        std::optional<std::shared_ptr<Initializer>> _init;
        Types::DataType _varType;
        std::optional<StorageClass> _storageClass;
    };

    class Constant : public Expression
    {
    public:
        Constant(std::shared_ptr<Constants::Const> c, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::Constant, dataType), _c{c} {}

        const std::shared_ptr<Constants::Const> &getConst() const { return _c; }

    private:
        std::shared_ptr<Constants::Const> _c;
    };

    class String : public Expression
    {
    public:
        String(const std::string &str, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::String, dataType), _str{str} {}

        auto &getStr() const { return _str; }

    private:
        std::string _str;
    };

    class Cast : public Expression
    {
    public:
        Cast(Types::DataType targetType, std::shared_ptr<Expression> exp, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::Cast, dataType), _targetType{targetType}, _exp{exp} {}

        const Types::DataType &getTargetType() const { return _targetType; }
        const std::shared_ptr<Expression> &getExp() const { return _exp; }

    private:
        Types::DataType _targetType;
        std::shared_ptr<Expression> _exp;
    };

    class Binary : public Expression
    {
    public:
        Binary(BinaryOp op, std::shared_ptr<Expression> exp1, std::shared_ptr<Expression> exp2, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::Binary, dataType), _op{op}, _exp1{std::move(exp1)}, _exp2{std::move(exp2)} {}

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
        Unary(UnaryOp op, std::shared_ptr<Expression> exp, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::Unary, dataType), _op{op}, _exp{exp} {}

        UnaryOp getOp() const { return _op; }
        std::shared_ptr<Expression> getExp() const { return _exp; }

    private:
        UnaryOp _op;
        std::shared_ptr<Expression> _exp;
    };

    class Var : public Expression
    {
    public:
        Var(const std::string &name, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::Var, dataType), _name{std::move(name)} {}

        auto getName() const { return _name; }

    private:
        std::string _name;
    };

    class Assignment : public Expression
    {
    public:
        Assignment(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::Assignment, dataType), _left{std::move(left)}, _right{std::move(right)} {}

        auto getLeftExp() const { return _left; }
        auto getRightExp() const { return _right; }

    private:
        std::shared_ptr<Expression> _left;
        std::shared_ptr<Expression> _right;
    };

    class CompoundAssignment : public Expression
    {
    public:
        CompoundAssignment(BinaryOp op, const std::shared_ptr<Expression> &left, const std::shared_ptr<Expression> &right, std::optional<Types::DataType> resultType = std::nullopt, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::CompoundAssignment, dataType), _op{op}, _left{std::move(left)}, _right{std::move(right)}, _resultType{resultType} {}

        auto getOp() const { return _op; }
        auto getLeftExp() const { return _left; }
        auto getRightExp() const { return _right; }
        auto &getResultType() const { return _resultType; }

    private:
        BinaryOp _op;
        std::shared_ptr<Expression> _left;
        std::shared_ptr<Expression> _right;
        std::optional<Types::DataType> _resultType;
    };

    class PostfixIncr : public Expression
    {
    public:
        PostfixIncr(std::shared_ptr<Expression> exp, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::PostfixIncr, dataType), _exp{std::move(exp)} {}

        auto getExp() const { return _exp; }

    private:
        std::shared_ptr<Expression> _exp;
    };

    class PostfixDecr : public Expression
    {
    public:
        PostfixDecr(std::shared_ptr<Expression> exp, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::PostfixDecr, dataType), _exp{std::move(exp)} {}

        auto getExp() const { return _exp; }

    private:
        std::shared_ptr<Expression> _exp;
    };

    class Conditional : public Expression
    {
    public:
        Conditional(std::shared_ptr<Expression> condition, std::shared_ptr<Expression> then, std::shared_ptr<Expression> elseExp, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::Conditional, dataType), _condition{condition}, _then{then}, _else{elseExp} {}

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
        FunctionCall(const std::string &fnName, std::vector<std::shared_ptr<Expression>> args, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::FunctionCall, dataType), _fnName{fnName}, _args{args} {}

        const std::string &getName() const { return _fnName; }
        const std::vector<std::shared_ptr<Expression>> &getArgs() const { return _args; }

    private:
        std::string _fnName;
        std::vector<std::shared_ptr<Expression>> _args;
    };

    class Dereference : public Expression
    {
    public:
        Dereference(const std::shared_ptr<Expression> &exp, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::Dereference, dataType), _innerExp{exp} {}
        auto getInnerExp() const { return _innerExp; }

    private:
        std::shared_ptr<Expression> _innerExp;
    };

    class AddrOf : public Expression
    {
    public:
        AddrOf(const std::shared_ptr<Expression> &exp, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::AddrOf, dataType), _innerExp{exp} {}
        auto getInnerExp() const { return _innerExp; }

    private:
        std::shared_ptr<Expression> _innerExp;
    };

    class Subscript : public Expression
    {
    public:
        Subscript(const std::shared_ptr<Expression> &exp1, const std::shared_ptr<Expression> &exp2, std::optional<Types::DataType> dataType = std::nullopt)
            : Expression(NodeType::Subscript, dataType), _exp1{exp1}, _exp2{exp2} {}

        auto getExp1() const { return _exp1; }
        auto getExp2() const { return _exp2; }

    private:
        std::shared_ptr<Expression> _exp1;
        std::shared_ptr<Expression> _exp2;
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
        FunctionDeclaration(const std::string &name, std::vector<std::string> params, std::optional<Block> body, const Types::DataType &funType, std::optional<StorageClass> storageClass = std::nullopt)
            : Declaration(NodeType::FunctionDeclaration), _name{name}, _params{params}, _body{body}, _funType{funType}, _storageClass{storageClass} {}

        const std::string &getName() const { return _name; }
        const std::vector<std::string> &getParams() const { return _params; }
        const std::optional<Block> &getOptBody() const { return _body; }
        const Types::DataType &getFunType() const { return _funType; }
        const std::optional<StorageClass> &getOptStorageClass() const { return _storageClass; }

    private:
        std::string _name;
        std::vector<std::string> _params;
        std::optional<Block> _body;
        Types::DataType _funType;
        std::optional<StorageClass> _storageClass;
    };

    class Program : public Node
    {
    public:
        Program(std::vector<std::shared_ptr<Declaration>> decls)
            : Node(NodeType::Program), decls{decls} {}

        const std::vector<std::shared_ptr<Declaration>> &getDeclarations() const { return decls; }

    private:
        std::vector<std::shared_ptr<Declaration>> decls;
    };
}

#endif