#ifndef AST_PRETTY_PRINT_H
#define AST_PRETTY_PRINT_H

#include <iostream>
#include <string>
#include <memory>
#include "./../AST.h"
#include "./../Types.h"
#include "./../Const.h"

class ASTPrettyPrint
{
public:
    ASTPrettyPrint() : indentLevel(0) {}

    void print(const AST::Program &program)
    {
        visit(program);
    }

    void visit(const AST::Node &node, bool indent = true)
    {
        switch (node.getType())
        {
        case AST::NodeType::Program:
            visitProgram(static_cast<const AST::Program &>(node));
            break;
        case AST::NodeType::FunctionDeclaration:
            visitFunctionDeclaration(static_cast<const AST::FunctionDeclaration &>(node));
            break;
        case AST::NodeType::VariableDeclaration:
            visitVariableDeclaration(static_cast<const AST::VariableDeclaration &>(node), indent);
            break;
        case AST::NodeType::Return:
            visitReturn(static_cast<const AST::Return &>(node), indent);
            break;
        case AST::NodeType::Constant:
            visitConstant(static_cast<const AST::Constant &>(node), indent);
            break;
        case AST::NodeType::String:
            visitString(static_cast<const AST::String &>(node), indent);
            break;
        case AST::NodeType::Unary:
            visitUnary(static_cast<const AST::Unary &>(node), indent);
            break;
        case AST::NodeType::Binary:
            visitBinary(static_cast<const AST::Binary &>(node), indent);
            break;
        case AST::NodeType::Var:
            visitVar(static_cast<const AST::Var &>(node), indent);
            break;
        case AST::NodeType::Assignment:
            visitAssignment(static_cast<const AST::Assignment &>(node), indent);
            break;
        case AST::NodeType::CompoundAssignment:
            visitCompoundAssignment(static_cast<const AST::CompoundAssignment &>(node), indent);
            break;
        case AST::NodeType::PostfixIncr:
            visitPostfixIncr(static_cast<const AST::PostfixIncr &>(node), indent);
            break;
        case AST::NodeType::PostfixDecr:
            visitPostfixDecr(static_cast<const AST::PostfixDecr &>(node), indent);
            break;
        case AST::NodeType::Conditional:
            visitConditional(static_cast<const AST::Conditional &>(node), indent);
            break;
        case AST::NodeType::FunctionCall:
            visitFunctionCall(static_cast<const AST::FunctionCall &>(node), indent);
            break;
        case AST::NodeType::ExpressionStmt:
            visitExpressionStmt(static_cast<const AST::ExpressionStmt &>(node), indent);
            break;
        case AST::NodeType::If:
            visitIf(static_cast<const AST::If &>(node), indent);
            break;
        case AST::NodeType::Compound:
            visitCompound(static_cast<const AST::Compound &>(node), indent);
            break;
        case AST::NodeType::Break:
            visitBreak(static_cast<const AST::Break &>(node), indent);
            break;
        case AST::NodeType::Continue:
            visitContinue(static_cast<const AST::Continue &>(node), indent);
            break;
        case AST::NodeType::While:
            visitWhile(static_cast<const AST::While &>(node), indent);
            break;
        case AST::NodeType::DoWhile:
            visitDoWhile(static_cast<const AST::DoWhile &>(node), indent);
            break;
        case AST::NodeType::For:
            visitFor(static_cast<const AST::For &>(node), indent);
            break;
        case AST::NodeType::Null:
            visitNull(static_cast<const AST::Null &>(node), indent);
            break;
        case AST::NodeType::LabeledStatement:
            visitLabeledStatement(static_cast<const AST::LabeledStatement &>(node), indent);
            break;
        case AST::NodeType::Goto:
            visitGoto(static_cast<const AST::Goto &>(node), indent);
            break;
        case AST::NodeType::Switch:
            visitSwitch(static_cast<const AST::Switch &>(node), indent);
            break;
        case AST::NodeType::Case:
            visitCase(static_cast<const AST::Case &>(node), indent);
            break;
        case AST::NodeType::Default:
            visitDefault(static_cast<const AST::Default &>(node), indent);
            break;
        case AST::NodeType::Cast:
            visitCast(static_cast<const AST::Cast &>(node), indent);
            break;
        case AST::NodeType::InitDecl:
            visitInitDecl(static_cast<const AST::InitDecl &>(node), indent);
            break;
        case AST::NodeType::InitExp:
            visitInitExp(static_cast<const AST::InitExp &>(node), indent);
            break;
        case AST::NodeType::Dereference:
            visitDereference(static_cast<const AST::Dereference &>(node), indent);
            break;
        case AST::NodeType::AddrOf:
            visitAddrOf(static_cast<const AST::AddrOf &>(node), indent);
            break;
        case AST::NodeType::SingleInit:
        case AST::NodeType::CompoundInit:
            visitInitializer(static_cast<const AST::Initializer &>(node), indent);
            break;
        case AST::NodeType::Subscript:
            visitSubscript(static_cast<const AST::Subscript &>(node), indent);
            break;
        case AST::NodeType::SizeOfT:
            visitSizeOfT(static_cast<const AST::SizeOfT &>(node), indent);
            break;
        case AST::NodeType::SizeOf:
            visitSizeOf(static_cast<const AST::SizeOf &>(node), indent);
            break;
        case AST::NodeType::StructDeclaration:
            visitStructDeclaration(static_cast<const AST::StructDeclaration &>(node), indent);
            break;
        case AST::NodeType::MemberDeclaration:
            visitMemberDeclaration(static_cast<const AST::MemberDeclaration &>(node), indent);
            break;
        case AST::NodeType::Dot:
            visitDot(static_cast<const AST::Dot &>(node), indent);
            break;
        case AST::NodeType::Arrow:
            visitArrow(static_cast<const AST::Arrow &>(node), indent);
            break;
        default:
            std::cerr << "Unknown node type" << std::endl;
            break;
        }
    }

    void visitAddrOf(const AST::AddrOf &addr, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "AddrOf(\n";
        increaseIndent();
        std::cout << getIndent() << "exp=";
        visit(*addr.getInnerExp(), false);
        std::cout << getIndent() << "dataType=";
        if (addr.getDataType().has_value())
            std::cout << Types::dataTypeToString(addr.getDataType().value());
        else
            std::cout << "unchecked";
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

private:
    int indentLevel;

    void increaseIndent() { indentLevel++; }
    void decreaseIndent() { indentLevel--; }
    std::string getIndent() const { return std::string(indentLevel * 4, ' '); }

    void visitProgram(const AST::Program &program)
    {
        std::cout << getIndent() << "Program(\n";
        increaseIndent();

        for (const auto &decl : program.getDeclarations())
        {
            visit(*decl);
        }

        decreaseIndent();
        std::cout << getIndent() << ")\n";
    }

    void visitFunctionDeclaration(const AST::FunctionDeclaration &fnDecl)
    {
        std::cout << getIndent() << "FunctionDeclaration(\n";
        increaseIndent();
        std::cout << getIndent() << "name=\"" << fnDecl.getName() << "\",\n";
        std::cout << getIndent() << "params=[\n";

        for (const auto &param : fnDecl.getParams())
        {
            std::cout << getIndent() << "\t\"" << param << "\",\n";
        }
        std::cout << getIndent() << "],\n";

        if (fnDecl.getOptBody().has_value())
        {
            std::cout << getIndent() << "body=\n";
            increaseIndent();

            for (auto item : fnDecl.getOptBody().value())
            {
                visit(*item);
            }

            decreaseIndent();
        }
        else
        {
            std::cout << getIndent() << "body=None\n";
        }

        std::cout << getIndent() << "funType=" << Types::dataTypeToString(fnDecl.getFunType()) << ",\n";

        std::cout << getIndent() << "storageClass=";
        if (fnDecl.getOptStorageClass().has_value())
        {
            if (fnDecl.getOptStorageClass().value() == AST::StorageClass::Static)
            {
                std::cout << "Static,\n";
            }
            else
            {
                std::cout << "Extern,\n";
            }
        }
        else
        {
            std::cout << "None,\n";
        }

        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitVariableDeclaration(const AST::VariableDeclaration &varDecl, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "VariableDeclaration(\n";
        increaseIndent();
        std::cout << getIndent() << "name=\"" << varDecl.getName() << "\",\n";
        std::cout << getIndent() << "varType=" << Types::dataTypeToString(varDecl.getVarType()) << ",\n";
        std::cout << getIndent() << "init=";
        if (varDecl.getOptInit().has_value())
        {
            visitInitializer(*varDecl.getOptInit().value(), false);
        }
        else
        {
            std::cout << "None\n";
        }

        std::cout << getIndent() << "storageClass=";
        if (varDecl.getOptStorageClass().has_value())
        {
            if (varDecl.getOptStorageClass().value() == AST::StorageClass::Static)
            {
                std::cout << "Static,\n";
            }
            else
            {
                std::cout << "Extern,\n";
            }
        }
        else
        {
            std::cout << "None,\n";
        }

        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitInitializer(const AST::Initializer &init, bool indent = true)
    {
        if (init.getType() == AST::NodeType::SingleInit)
        {
            if (indent)
                std::cout << getIndent();
            std::cout << "SingleInit(\n";
            increaseIndent();
            std::cout << getIndent() << "exp=";
            visit(*static_cast<const AST::SingleInit &>(init).getExp(), false);
            std::cout << getIndent() << "dataType=";
            if (init.getDataType().has_value())
                std::cout << Types::dataTypeToString(init.getDataType().value());
            else
                std::cout << "unchecked";
            std::cout << "\n";
            decreaseIndent();
            std::cout << getIndent() << "),\n";
        }
        else if (init.getType() == AST::NodeType::CompoundInit)
        {
            if (indent)
                std::cout << getIndent();
            std::cout << "CompoundInit(\n";
            increaseIndent();
            const auto &inits = static_cast<const AST::CompoundInit &>(init).getInits();
            for (const auto &subInit : inits)
            {
                visitInitializer(*subInit, true);
            }
            std::cout << getIndent() << "dataType=";
            if (init.getDataType().has_value())
                std::cout << Types::dataTypeToString(init.getDataType().value());
            else
                std::cout << "unchecked";
            std::cout << "\n";
            decreaseIndent();
            std::cout << getIndent() << "),\n";
        }
    }

    void visitCast(const AST::Cast &cast, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Cast(\n";
        increaseIndent();
        std::cout << getIndent() << "targetType=" << Types::dataTypeToString(cast.getTargetType()) << ",\n";
        std::cout << getIndent() << "exp=";
        visit(*cast.getExp(), false);
        std::cout << getIndent() << "dataType=";
        if (cast.getDataType().has_value())
        {
            std::cout << Types::dataTypeToString(cast.getDataType().value());
        }
        else
        {
            std::cout << "unchecked";
        }
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitInitDecl(const AST::InitDecl &initDecl, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "InitDecl(\n";
        increaseIndent();
        std::cout << getIndent() << "decl=";
        visit(*initDecl.getDecl(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitInitExp(const AST::InitExp &initExp, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "InitExp(\n";
        increaseIndent();
        std::cout << getIndent() << "exp=";
        if (initExp.getOptExp().has_value())
        {
            visit(*initExp.getOptExp().value(), false);
        }
        else
        {
            std::cout << "None\n";
        }
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitReturn(const AST::Return &ret, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Return(\n";
        increaseIndent();
        if (ret.getOptValue().has_value())
        {
            visit(*ret.getOptValue().value());
        }
        else
        {
            std::cout << getIndent() << "None\n";
        }
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitConstant(const AST::Constant &constant, bool indent)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Constant(" << Constants::toString(*(constant.getConst())) << ", ";
        if (constant.getDataType().has_value())
        {
            std::cout << Types::dataTypeToString(constant.getDataType().value());
        }
        else
        {
            std::cout << "unchecked";
        }
        std::cout << ")\n";
    }

    void visitString(const AST::String &strNode, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "String(\"" << strNode.getStr() << "\"";
        if (strNode.getDataType().has_value())
        {
            std::cout << ", " << Types::dataTypeToString(strNode.getDataType().value());
        }
        else
        {
            std::cout << ", unchecked";
        }
        std::cout << ")\n";
    }

    void visitUnary(const AST::Unary &unary, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Unary(\n";
        increaseIndent();
        std::cout << getIndent() << "op=";
        switch (unary.getOp())
        {
        case AST::UnaryOp::Complement:
            std::cout << "Complement";
            break;
        case AST::UnaryOp::Negate:
            std::cout << "Negate";
            break;
        case AST::UnaryOp::Not:
            std::cout << "Not";
            break;
        case AST::UnaryOp::Incr:
            std::cout << "Incr";
            break;
        case AST::UnaryOp::Decr:
            std::cout << "Decr";
            break;
        default:
            break;
        }
        std::cout << ",\n";
        std::cout << getIndent() << "exp=";
        visit(*unary.getExp(), false);
        std::cout << getIndent() << "dataType=";
        if (unary.getDataType().has_value())
        {
            std::cout << Types::dataTypeToString(unary.getDataType().value());
        }
        else
        {
            std::cout << "unchecked";
        }
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitBinary(const AST::Binary &binary, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Binary(\n";
        increaseIndent();
        std::cout << getIndent() << "op=";
        switch (binary.getOp())
        {
        case AST::BinaryOp::Add:
            std::cout << "Add";
            break;
        case AST::BinaryOp::Subtract:
            std::cout << "Subtract";
            break;
        case AST::BinaryOp::Multiply:
            std::cout << "Multiply";
            break;
        case AST::BinaryOp::Divide:
            std::cout << "Divide";
            break;
        case AST::BinaryOp::Remainder:
            std::cout << "Remainder";
            break;
        case AST::BinaryOp::And:
            std::cout << "And";
            break;
        case AST::BinaryOp::Or:
            std::cout << "Or";
            break;
        case AST::BinaryOp::Equal:
            std::cout << "Equal";
            break;
        case AST::BinaryOp::NotEqual:
            std::cout << "NotEqual";
            break;
        case AST::BinaryOp::LessThan:
            std::cout << "LessThan";
            break;
        case AST::BinaryOp::LessOrEqual:
            std::cout << "LessOrEqual";
            break;
        case AST::BinaryOp::GreaterThan:
            std::cout << "GreaterThan";
            break;
        case AST::BinaryOp::GreaterOrEqual:
            std::cout << "GreaterOrEqual";
            break;
        case AST::BinaryOp::BitwiseXor:
            std::cout << "BitwiseXor";
            break;
        case AST::BinaryOp::BitwiseOr:
            std::cout << "BitwiseOr";
            break;
        case AST::BinaryOp::BitShiftLeft:
            std::cout << "BitShiftLeft";
            break;
        case AST::BinaryOp::BitShiftRight:
            std::cout << "BitShiftRight";
            break;
        default:
            break;
        }
        std::cout << ",\n";
        std::cout << getIndent() << "left=";
        visit(*binary.getExp1(), false);
        std::cout << getIndent() << "right=";
        visit(*binary.getExp2(), false);
        std::cout << getIndent() << "dataType=";
        if (binary.getDataType().has_value())
        {
            std::cout << Types::dataTypeToString(binary.getDataType().value());
        }
        else
        {
            std::cout << "unchecked";
        }
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitVar(const AST::Var &var, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Var(" << var.getName() << ", dataType=";
        if (var.getDataType().has_value())
        {
            std::cout << Types::dataTypeToString(var.getDataType().value());
        }
        else
        {
            std::cout << "unchecked";
        }
        std::cout << ")\n";
    }

    void visitAssignment(const AST::Assignment &assignment, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Assignment(\n";
        increaseIndent();
        std::cout << getIndent() << "left=";
        visit(*assignment.getLeftExp(), false);
        std::cout << getIndent() << "right=";
        visit(*assignment.getRightExp(), false);
        std::cout << getIndent() << "dataType=";
        if (assignment.getDataType().has_value())
        {
            std::cout << Types::dataTypeToString(assignment.getDataType().value());
        }
        else
        {
            std::cout << "unchecked";
        }
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitCompoundAssignment(const AST::CompoundAssignment &compoundAssignment, bool indent = true)
    {
        if (indent)
        {
            std::cout << getIndent();
        }
        std::cout << "CompoundAssignment(\n";
        increaseIndent();
        std::cout << getIndent() << "op=" << getOpString(compoundAssignment.getOp()) << ",\n";
        std::cout << getIndent() << "left=";
        visit(*compoundAssignment.getLeftExp(), false);
        std::cout << getIndent() << "right=";
        visit(*compoundAssignment.getRightExp(), false);
        std::cout << getIndent() << "resultType=";
        if (compoundAssignment.getDataType().has_value())
        {
            std::cout << Types::dataTypeToString(compoundAssignment.getResultType().value());
        }
        else
        {
            std::cout << "unchecked";
        }
        std::cout << "\n";
        std::cout << getIndent() << "dataType=";
        if (compoundAssignment.getDataType().has_value())
        {
            std::cout << Types::dataTypeToString(compoundAssignment.getDataType().value());
        }
        else
        {
            std::cout << "unchecked";
        }
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    std::string getOpString(AST::BinaryOp op)
    {
        switch (op)
        {
        case AST::BinaryOp::Add:
            return "Add";
        case AST::BinaryOp::Subtract:
            return "Subtract";
        case AST::BinaryOp::Multiply:
            return "Multiply";
        case AST::BinaryOp::Divide:
            return "Divide";
        case AST::BinaryOp::Remainder:
            return "Remainder";
        case AST::BinaryOp::And:
            return "And";
        case AST::BinaryOp::Or:
            return "Or";
        case AST::BinaryOp::Equal:
            return "Equal";
        case AST::BinaryOp::NotEqual:
            return "NotEqual";
        case AST::BinaryOp::LessThan:
            return "LessThan";
        case AST::BinaryOp::LessOrEqual:
            return "LessOrEqual";
        case AST::BinaryOp::GreaterThan:
            return "GreaterThan";
        case AST::BinaryOp::GreaterOrEqual:
            return "GreaterOrEqual";
        case AST::BinaryOp::BitwiseAnd:
            return "BitwiseAnd";
        case AST::BinaryOp::BitwiseOr:
            return "BitwiseOr";
        case AST::BinaryOp::BitwiseXor:
            return "BitwiseXor";
        case AST::BinaryOp::BitShiftLeft:
            return "BitShiftLeft";
        case AST::BinaryOp::BitShiftRight:
            return "BitShiftRight";
        default:
            return "Unknown";
        }
    }

    void visitPostfixIncr(const AST::PostfixIncr &postfixIncr, bool indent = true)
    {
        if (indent)
        {
            std::cout << getIndent();
        }
        std::cout << "PostfixIncr(\n";
        increaseIndent();
        std::cout << getIndent() << "exp=";
        visit(*postfixIncr.getExp(), false);
        std::cout << getIndent() << "dataType=";
        if (postfixIncr.getDataType().has_value())
        {
            std::cout << Types::dataTypeToString(postfixIncr.getDataType().value());
        }
        else
        {
            std::cout << "unchecked";
        }
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitPostfixDecr(const AST::PostfixDecr &postfixDecr, bool indent = true)
    {
        if (indent)
        {
            std::cout << getIndent();
        }
        std::cout << "PostfixDecr(\n";
        increaseIndent();
        std::cout << getIndent() << "exp=";
        visit(*postfixDecr.getExp(), false);
        std::cout << getIndent() << "dataType=";
        if (postfixDecr.getDataType().has_value())
        {
            std::cout << Types::dataTypeToString(postfixDecr.getDataType().value());
        }
        else
        {
            std::cout << "unchecked";
        }
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitConditional(const AST::Conditional &conditional, bool indent = true)
    {
        if (indent)
        {
            std::cout << getIndent();
        }
        std::cout << "Conditional(\n";
        increaseIndent();
        std::cout << getIndent() << "condition=";
        visit(*conditional.getCondition(), false);
        std::cout << getIndent() << "then=";
        visit(*conditional.getThen(), false);
        std::cout << getIndent() << "else=";
        visit(*conditional.getElse(), false);
        std::cout << getIndent() << "dataType=";
        if (conditional.getDataType().has_value())
        {
            std::cout << Types::dataTypeToString(conditional.getDataType().value());
        }
        else
        {
            std::cout << "unchecked";
        }
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitFunctionCall(const AST::FunctionCall &fnCall, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "FunctionCall(\n";
        increaseIndent();
        std::cout << getIndent() << "name=\"" << fnCall.getName() << "\",\n";
        std::cout << getIndent() << "args=[\n";
        for (const auto &arg : fnCall.getArgs())
        {
            visit(*arg, true);
        }
        std::cout << getIndent() << "],\n";
        std::cout << getIndent() << "dataType=";
        if (fnCall.getDataType().has_value())
        {
            std::cout << Types::dataTypeToString(fnCall.getDataType().value());
        }
        else
        {
            std::cout << "unchecked";
        }
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitExpressionStmt(const AST::ExpressionStmt &exprStmt, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "ExpressionStmt(\n";
        increaseIndent();
        visit(*exprStmt.getExp());
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitIf(const AST::If &ifStmt, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "If(\n";
        increaseIndent();
        std::cout << getIndent() << "condition=";
        visit(*ifStmt.getCondition(), false);
        std::cout << getIndent() << "thenClause=";
        visit(*ifStmt.getThenClause(), false);

        if (ifStmt.getOptElseClause().has_value())
        {
            std::cout << getIndent() << "elseClause=";
            visit(*ifStmt.getOptElseClause().value(), false);
        }

        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitCompound(const AST::Compound &compound, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Compound(\n";
        increaseIndent();

        for (const auto &item : compound.getBlock())
        {
            visit(*item);
        }

        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitBreak(const AST::Break &brk, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Break(" << brk.getId() << ")\n";
    }
    void visitContinue(const AST::Continue &cont, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Continue(" << cont.getId() << ")\n";
    }
    void visitWhile(const AST::While &whileLoop, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "While(\n";
        increaseIndent();
        std::cout << getIndent() << "condition=";
        visit(*whileLoop.getCondition(), false);
        std::cout << getIndent() << "body=";
        visit(*whileLoop.getBody(), false);
        std::cout << getIndent() << "id=\"" << whileLoop.getId() << "\"\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }
    void visitDoWhile(const AST::DoWhile &doLoop, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "DoWhile(\n";
        increaseIndent();
        std::cout << getIndent() << "body=";
        visit(*doLoop.getBody(), false);
        std::cout << getIndent() << "condition=";
        visit(*doLoop.getCondition(), false);
        std::cout << getIndent() << "id=\"" << doLoop.getId() << "\"\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }
    void visitFor(const AST::For &forLoop, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "For(\n";
        increaseIndent();

        std::cout << getIndent() << "init=";
        auto &init = forLoop.getInit();

        if (init->getType() == AST::NodeType::InitDecl)
        {
            visit(*static_cast<AST::InitDecl &>(*init).getDecl(), false);
        }
        else
        {
            auto &expInit = static_cast<AST::InitExp &>(*init);

            if (expInit.getOptExp().has_value())
            {
                visit(*expInit.getOptExp().value(), false);
            }
            else
            {
                std::cout << "None\n";
            }
        }

        std::cout << getIndent() << "condition=";
        if (forLoop.getOptCondition().has_value())
        {
            visit(*forLoop.getOptCondition().value(), false);
        }
        else
        {
            std::cout << "None\n";
        }

        std::cout << getIndent() << "post=";
        if (forLoop.getOptPost().has_value())
        {
            visit(*forLoop.getOptPost().value(), false);
        }
        else
        {
            std::cout << "None\n";
        }

        std::cout << getIndent() << "body=";
        visit(*forLoop.getBody(), false);
        std::cout << getIndent() << "id=\"" << forLoop.getId() << "\"\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitNull(const AST::Null &null, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Null()\n";
    }

    void visitLabeledStatement(const AST::LabeledStatement &labeledStmt, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "LabeledStatement(\n";
        increaseIndent();
        std::cout << getIndent() << "label=" << labeledStmt.getLabel() << '\n';
        std::cout << getIndent() << "statement=";
        visit(*labeledStmt.getStatement(), false);
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitGoto(const AST::Goto &gotoStmt, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Goto(" << gotoStmt.getLabel() << ")\n";
    }

    void visitSwitch(const AST::Switch &switchStmt, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Switch(\n";
        increaseIndent();
        std::cout << getIndent() << "control=";
        visit(*switchStmt.getControl(), false);
        std::cout << getIndent() << "body=";
        visit(*switchStmt.getBody(), false);

        if (switchStmt.getOptCases().has_value())
        {
            std::cout << getIndent() << "cases=\n";
            increaseIndent();
            for (const auto &[key, value] : switchStmt.getOptCases().value())
            {
                if (key.has_value())
                {
                    std::cout << getIndent() << "Case(" << Constants::toString(*key.value()) << ": " << value << ")\n";
                }
                else
                {
                    std::cout << getIndent() << "Default(" << value << ")\n";
                }
            }
            decreaseIndent();
        }
        else
        {
            std::cout << getIndent() << "cases=None\n";
        }

        std::cout << getIndent() << "id=\"" << switchStmt.getId() << "\"\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitCase(const AST::Case &caseStmt, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Case(\n";
        increaseIndent();
        std::cout << getIndent() << "value=";
        visit(*caseStmt.getValue(), false);
        std::cout << getIndent() << "body=";
        visit(*caseStmt.getBody(), false);
        std::cout << getIndent() << "id=\"" << caseStmt.getId() << "\"\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitDefault(const AST::Default &defaultStmt, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();

        std::cout << "Case(\n";
        increaseIndent();
        std::cout << getIndent() << "body=";
        visit(*defaultStmt.getBody(), false);
        std::cout << getIndent() << "id=\"" << defaultStmt.getId() << "\"\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitDereference(const AST::Dereference &deref, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Dereference(\n";
        increaseIndent();
        std::cout << getIndent() << "exp=";
        visit(*deref.getInnerExp(), false);
        std::cout << getIndent() << "dataType=";
        if (deref.getDataType().has_value())
            std::cout << Types::dataTypeToString(deref.getDataType().value());
        else
            std::cout << "unchecked";
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitSubscript(const AST::Subscript &sub, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Subscript(\n";
        increaseIndent();
        std::cout << getIndent() << "exp1=";
        visit(*sub.getExp1(), false);
        std::cout << getIndent() << "exp2=";
        visit(*sub.getExp2(), false);
        std::cout << getIndent() << "dataType=";
        if (sub.getDataType().has_value())
            std::cout << Types::dataTypeToString(sub.getDataType().value());
        else
            std::cout << "unchecked";
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitSizeOfT(const AST::SizeOfT &sizeOfT, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "SizeOfT(typeName=" << Types::dataTypeToString(*sizeOfT.getTypeName());
        if (sizeOfT.getDataType().has_value())
        {
            std::cout << ", dataType=" << Types::dataTypeToString(sizeOfT.getDataType().value());
        }
        else
        {
            std::cout << ", dataType=unchecked";
        }
        std::cout << ")\n";
    }

    void visitSizeOf(const AST::SizeOf &sizeOf, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "SizeOf(\n";
        increaseIndent();
        std::cout << getIndent() << "exp=";
        visit(*sizeOf.getInnerExp(), false);
        std::cout << getIndent() << "dataType=";
        if (sizeOf.getDataType().has_value())
        {
            std::cout << Types::dataTypeToString(sizeOf.getDataType().value());
        }
        else
        {
            std::cout << "unchecked";
        }
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitStructDeclaration(const AST::StructDeclaration &strct, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "StructDeclaration(\n";
        increaseIndent();
        std::cout << getIndent() << "tag=\"" << strct.getTag() << "\",\n";
        std::cout << getIndent() << "members=[\n";
        increaseIndent();
        for (const auto &member : strct.getMembers())
        {
            visit(*member, true);
        }
        decreaseIndent();
        std::cout << getIndent() << "]\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitMemberDeclaration(const AST::MemberDeclaration &member, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "MemberDeclaration(";
        std::cout << "name=\"" << member.getMemberName() << "\", ";
        std::cout << "type=" << Types::dataTypeToString(*member.getMemberType());
        std::cout << "),\n";
    }

    void visitDot(const AST::Dot &dot, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Dot(\n";
        increaseIndent();
        std::cout << getIndent() << "struct=";
        visit(*dot.getStruct(), false);
        std::cout << getIndent() << "member=\"" << dot.getMember() << "\"";
        if (dot.getDataType().has_value())
            std::cout << ", dataType=" << Types::dataTypeToString(dot.getDataType().value());
        else
            std::cout << ", dataType=unchecked";
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }

    void visitArrow(const AST::Arrow &arrow, bool indent = true)
    {
        if (indent)
            std::cout << getIndent();
        std::cout << "Arrow(\n";
        increaseIndent();
        std::cout << getIndent() << "struct=";
        visit(*arrow.getStruct(), false);
        std::cout << getIndent() << "member=\"" << arrow.getMember() << "\"";
        if (arrow.getDataType().has_value())
            std::cout << ", dataType=" << Types::dataTypeToString(arrow.getDataType().value());
        else
            std::cout << ", dataType=unchecked";
        std::cout << "\n";
        decreaseIndent();
        std::cout << getIndent() << "),\n";
    }
};

#endif