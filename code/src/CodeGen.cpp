#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

#include "CodeGen.h"
#include "AST.h"
#include "Assembly.h"

std::shared_ptr<Assembly::Operand> CodeGen::convertExp(std::shared_ptr<AST::Expression> exp)
{
    switch (exp->getType())
    {
    case AST::NodeType::Constant:
    {
        auto constant = std::dynamic_pointer_cast<AST::Constant>(exp);
        return std::make_shared<Assembly::Imm>(constant->getValue());
    }
    default:
    {
        throw std::runtime_error("Unknown expression type");
    }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Assembly::Instruction>> CodeGen::convertStmt(std::shared_ptr<AST::Statement> stmt)
{
    switch (stmt->getType())
    {
    case AST::NodeType::Return:
    {
        return convertReturn(std::dynamic_pointer_cast<AST::Return>(stmt));
    }
    default:
    {
        throw std::runtime_error("Unknown statement type");
    }
    }
}

std::vector<std::shared_ptr<Assembly::Instruction>> CodeGen::convertReturn(std::shared_ptr<AST::Return> returnStmt)
{
    auto value = convertExp(returnStmt->getValue());
    std::vector<std::shared_ptr<Assembly::Instruction>> instructions{
        std::make_shared<Assembly::Mov>(value, std::make_shared<Assembly::Register>()),
        std::make_shared<Assembly::Ret>(),
    };

    return instructions;
}

std::shared_ptr<Assembly::Function> CodeGen::convertFunc(std::shared_ptr<AST::FunctionDefinition> funDef)
{
    auto insts = convertStmt(funDef->getBody());
    auto name = funDef->getName();

    return std::make_shared<Assembly::Function>(std::move(name), std::move(insts));
}

std::shared_ptr<Assembly::Program> CodeGen::gen(std::shared_ptr<AST::Program> program)
{
    auto asmFun = convertFunc(program->getFunctionDefinition());
    return std::make_shared<Assembly::Program>(std::move(asmFun));
}
