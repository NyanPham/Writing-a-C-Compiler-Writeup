#include "TypeChecker.h"
#include "AST.h"
#include "Types.h"
#include "Symbols.h"

// Helper function to compare types
bool isCompatibleType(const Types::DataType &type1, const Types::DataType &type2)
{
    if (Types::isIntType(type1) && Types::isIntType(type2))
        return true;

    if (Types::isFunType(type1) && Types::isFunType(type2))
    {
        auto funType1 = Types::getFunType(type1).value();
        auto funType2 = Types::getFunType(type2).value();

        return (funType1.paramCount == funType2.paramCount);
    }

    return false;
}

std::shared_ptr<AST::Expression> TypeChecker::typeCheckExp(const std::shared_ptr<AST::Expression> &exp)
{
    switch (exp->getType())
    {
    case AST::NodeType::FunctionCall:
    {
        auto fnCall = std::dynamic_pointer_cast<AST::FunctionCall>(exp);
        auto t = symbolTable.get(fnCall->getName()).type;

        if (Types::isIntType(t))
            throw std::runtime_error("Tried to use variable as a function: " + fnCall->getName());

        if (Types::isFunType(t))
        {
            auto funType = Types::getFunType(t).value();

            if (funType.paramCount != fnCall->getArgs().size())
            {
                throw std::runtime_error("Function called with wrong number of arguments: " + fnCall->getName());
            }

            for (const auto &arg : fnCall->getArgs())
            {
                typeCheckExp(arg);
            }

            return std::make_shared<AST::FunctionCall>(fnCall->getName(), fnCall->getArgs());
        }

        throw std::runtime_error("Internal Error:Unknown type of symbol!");
    }
    case AST::NodeType::Var:
    {
        auto var = std::dynamic_pointer_cast<AST::Var>(exp);
        auto t = symbolTable.get(var->getName()).type;

        if (Types::isIntType(t))
        {
            return std::make_shared<AST::Var>(var->getName());
        }

        if (Types::isFunType(t))
        {
            throw std::runtime_error("Tried to use function as a variable: " + var->getName());
        }

        throw std::runtime_error("Internal Error: Unknown type of symbol!");
    }
    case AST::NodeType::Unary:
    {
        auto unary = std::dynamic_pointer_cast<AST::Unary>(exp);
        auto newInnerExp = typeCheckExp(unary->getExp());

        return std::make_shared<AST::Unary>(unary->getOp(), newInnerExp);
    }
    case AST::NodeType::Binary:
    {
        auto binary = std::dynamic_pointer_cast<AST::Binary>(exp);
        auto newExp1 = typeCheckExp(binary->getExp1());
        auto newExp2 = typeCheckExp(binary->getExp2());

        return std::make_shared<AST::Binary>(binary->getOp(), newExp1, newExp2);
    }
    case AST::NodeType::Assignment:
    {
        auto assignment = std::dynamic_pointer_cast<AST::Assignment>(exp);
        auto newLeftExp = typeCheckExp(assignment->getLeftExp());
        auto newRightExp = typeCheckExp(assignment->getRightExp());

        return std::make_shared<AST::Assignment>(newLeftExp, newRightExp);
    }
    case AST::NodeType::CompoundAssignment:
    {
        auto compoundAssignment = std::dynamic_pointer_cast<AST::CompoundAssignment>(exp);
        auto newLeftExp = typeCheckExp(compoundAssignment->getLeftExp());
        auto newRightExp = typeCheckExp(compoundAssignment->getRightExp());

        return std::make_shared<AST::CompoundAssignment>(compoundAssignment->getOp(), newLeftExp, newRightExp);
    }
    case AST::NodeType::PostfixDecr:
    {
        auto postfixDecr = std::dynamic_pointer_cast<AST::PostfixDecr>(exp);
        auto newInnerExp = typeCheckExp(postfixDecr->getExp());

        return std::make_shared<AST::PostfixDecr>(newInnerExp);
    }
    case AST::NodeType::PostfixIncr:
    {
        auto postfixIncr = std::dynamic_pointer_cast<AST::PostfixIncr>(exp);
        auto newInnerExp = typeCheckExp(postfixIncr->getExp());

        return std::make_shared<AST::PostfixIncr>(newInnerExp);
    }
    case AST::NodeType::Conditional:
    {
        auto conditional = std::dynamic_pointer_cast<AST::Conditional>(exp);
        auto newCond = typeCheckExp(conditional->getCondition());
        auto newThen = typeCheckExp(conditional->getThen());
        auto newElse = typeCheckExp(conditional->getElse());

        return std::make_shared<AST::Conditional>(newCond, newThen, newElse);
    }
    case AST::NodeType::Constant:
    {
        return exp;
    }
    default:
        throw std::runtime_error("Internal Error: Unknown type of expression!");
    }
}

AST::Block TypeChecker::typeCheckBlock(const AST::Block &blk)
{
    AST::Block newBlock;
    newBlock.reserve(blk.size());

    for (const auto &blkItm : blk)
    {
        newBlock.push_back(typeCheckBlockItem(blkItm));
    }

    return newBlock;
}

std::shared_ptr<AST::BlockItem> TypeChecker::typeCheckBlockItem(const std::shared_ptr<AST::BlockItem> &blkItem)
{
    switch (blkItem->getType())
    {
    case AST::NodeType::VariableDeclaration:
    case AST::NodeType::FunctionDeclaration:
    {
        return typeCheckDeclaration(std::dynamic_pointer_cast<AST::Declaration>(blkItem));
    }
    default:
    {
        return typeCheckStatement(std::dynamic_pointer_cast<AST::Statement>(blkItem));
    }
    }
}

std::shared_ptr<AST::Statement> TypeChecker::typeCheckStatement(const std::shared_ptr<AST::Statement> &stmt)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Return:
    {
        auto newRetValue = typeCheckExp(std::dynamic_pointer_cast<AST::Return>(stmt)->getValue());

        return std::make_shared<AST::Return>(newRetValue);
    }
    case AST::NodeType::ExpressionStmt:
    {
        auto newExp = typeCheckExp(std::dynamic_pointer_cast<AST::ExpressionStmt>(stmt)->getExp());

        return std::make_shared<AST::ExpressionStmt>(newExp);
    }
    case AST::NodeType::If:
    {
        auto ifStmt = std::dynamic_pointer_cast<AST::If>(stmt);
        auto newCond = typeCheckExp(ifStmt->getCondition());
        auto newThenCls = typeCheckStatement(ifStmt->getThenClause());
        auto newOptElseCls = std::optional<std::shared_ptr<AST::Statement>>{std::nullopt};
        if (ifStmt->getOptElseClause().has_value())
        {
            newOptElseCls = typeCheckStatement(ifStmt->getOptElseClause().value());
        }

        return std::make_shared<AST::If>(newCond, newThenCls, newOptElseCls);
    }
    case AST::NodeType::LabeledStatement:
    {
        auto labeledStmt = std::dynamic_pointer_cast<AST::LabeledStatement>(stmt);
        auto newStmt = typeCheckStatement(labeledStmt->getStatement());

        return std::make_shared<AST::LabeledStatement>(labeledStmt->getLabel(), newStmt);
    }
    case AST::NodeType::Case:
    {
        auto caseStmt = std::dynamic_pointer_cast<AST::Case>(stmt);

        auto newValue = typeCheckExp(caseStmt->getValue());
        auto newBody = typeCheckStatement(caseStmt->getBody());

        return std::make_shared<AST::Case>(newValue, newBody, caseStmt->getId());
    }
    case AST::NodeType::Default:
    {
        auto defaultStmt = std::dynamic_pointer_cast<AST::Default>(stmt);
        auto newBody = typeCheckStatement(defaultStmt->getBody());

        return std::make_shared<AST::Default>(newBody, defaultStmt->getId());
    }
    case AST::NodeType::Switch:
    {
        auto switchStmt = std::dynamic_pointer_cast<AST::Switch>(stmt);
        auto newControl = typeCheckExp(switchStmt->getControl());
        auto newBody = typeCheckStatement(switchStmt->getBody());

        return std::make_shared<AST::Switch>(
            newControl,
            newBody,
            switchStmt->getOptCases(),
            switchStmt->getId());
    }
    case AST::NodeType::Compound:
    {
        auto newBlock = typeCheckBlock(std::dynamic_pointer_cast<AST::Compound>(stmt)->getBlock());

        return std::make_shared<AST::Compound>(newBlock);
    }
    case AST::NodeType::While:
    {
        auto whileStmt = std::dynamic_pointer_cast<AST::While>(stmt);
        auto newCond = typeCheckExp(whileStmt->getCondition());
        auto newBody = typeCheckStatement(whileStmt->getBody());

        return std::make_shared<AST::While>(newCond, newBody, whileStmt->getId());
    }
    case AST::NodeType::DoWhile:
    {
        auto doWhileStmt = std::dynamic_pointer_cast<AST::DoWhile>(stmt);
        auto newBody = typeCheckStatement(doWhileStmt->getBody());
        auto newCond = typeCheckExp(doWhileStmt->getCondition());

        return std::make_shared<AST::DoWhile>(newBody, newCond, doWhileStmt->getId());
    }
    case AST::NodeType::For:
    {
        auto forStmt = std::dynamic_pointer_cast<AST::For>(stmt);

        std::shared_ptr<AST::ForInit> newInit = std::make_shared<AST::InitExp>(std::nullopt);
        if (auto initDecl = std::dynamic_pointer_cast<AST::InitDecl>(forStmt->getInit()))
        {
            newInit = std::make_shared<AST::InitDecl>(typeCheckVarDecl(initDecl->getDecl()));
        }
        else if (auto initExp = std::dynamic_pointer_cast<AST::InitExp>(forStmt->getInit()))
        {
            if (initExp->getOptExp().has_value())
            {
                newInit = std::make_shared<AST::InitExp>(typeCheckExp(initExp->getOptExp().value()));
            }
        }

        auto newCond = std::optional<std::shared_ptr<AST::Expression>>{std::nullopt};
        if (forStmt->getOptCondition().has_value())
        {
            newCond = std::make_optional(typeCheckExp(forStmt->getOptCondition().value()));
        }

        auto newPost = std::optional<std::shared_ptr<AST::Expression>>{std::nullopt};
        if (forStmt->getOptPost().has_value())
        {
            newPost = std::make_optional(typeCheckExp(forStmt->getOptPost().value()));
        }

        auto newBody = typeCheckStatement(forStmt->getBody());

        return std::make_shared<AST::For>(
            newInit,
            newCond,
            newPost,
            newBody,
            forStmt->getId());
    }
    case AST::NodeType::Null:
    case AST::NodeType::Break:
    case AST::NodeType::Continue:
    case AST::NodeType::Goto:
        return stmt;
    default:
        throw std::runtime_error("Internal Error: Unknown type of statement!");
    }
}

std::shared_ptr<AST::Declaration> TypeChecker::typeCheckDeclaration(const std::shared_ptr<AST::Declaration> &decl)
{
    if (auto varDecl = std::dynamic_pointer_cast<AST::VariableDeclaration>(decl))
        return typeCheckVarDecl(varDecl);
    else if (auto funDecl = std::dynamic_pointer_cast<AST::FunctionDeclaration>(decl))
        return typeCheckFunDecl(funDecl);
    else
        throw std::runtime_error("Internal Error: Unknown type of declaration!");
}

std::shared_ptr<AST::VariableDeclaration> TypeChecker::typeCheckVarDecl(const std::shared_ptr<AST::VariableDeclaration> &varDecl)
{
    symbolTable.addVariable(varDecl->getName(), Types::IntType{});
    if (varDecl->getOptInit().has_value())
    {
        auto newInit = typeCheckExp(varDecl->getOptInit().value());
        return std::make_shared<AST::VariableDeclaration>(varDecl->getName(), newInit);
    }

    return varDecl;
}

std::shared_ptr<AST::FunctionDeclaration> TypeChecker::typeCheckFunDecl(const std::shared_ptr<AST::FunctionDeclaration> &funDecl)
{
    auto funType = Types::FunType{static_cast<int>(funDecl->getParams().size())};
    bool hasBody = funDecl->getOptBody().has_value();
    bool alreadyDefined = hasBody;

    if (symbolTable.exists(funDecl->getName()))
    {
        auto oldDecl = symbolTable.get(funDecl->getName());

        if (!isCompatibleType(funType, Types::getFunType(oldDecl.type).value()))
            throw std::runtime_error("Incompatible function declaration!");

        alreadyDefined = oldDecl.isDefined || hasBody;
        if (oldDecl.isDefined && hasBody)
            throw std::runtime_error("Function is defined more than once!");
    }

    symbolTable.addFunction(funDecl->getName(), funType, alreadyDefined);

    if (hasBody)
    {
        for (const auto &param : funDecl->getParams())
        {
            symbolTable.addVariable(param, Types::IntType{});
        }

        auto newBlock = typeCheckBlock(funDecl->getOptBody().value());

        return std::make_shared<AST::FunctionDeclaration>(funDecl->getName(), funDecl->getParams(), newBlock);
    }

    return funDecl;
}

std::shared_ptr<AST::Program> TypeChecker::typeCheck(const std::shared_ptr<AST::Program> &prog)
{
    std::vector<std::shared_ptr<AST::FunctionDeclaration>> checkedFns;
    checkedFns.reserve(prog->getFunctionDeclarations().size());

    for (const auto &fnDecl : prog->getFunctionDeclarations())
    {
        auto checkedFn = typeCheckFunDecl(fnDecl);
        checkedFns.push_back(checkedFn);
    }

    return std::make_shared<AST::Program>(checkedFns);
}
