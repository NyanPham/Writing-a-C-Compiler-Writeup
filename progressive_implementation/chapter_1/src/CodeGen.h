#ifndef CODEGEN_H
#define CODEGEN_H

#include <string>
#include <memory>
#include <vector>

#include "AST.h"
#include "Assembly.h"

class CodeGen
{
public:
    CodeGen() {}
    std::shared_ptr<Assembly::Operand> convertExp(std::shared_ptr<AST::Expression> exp);
    std::vector<std::shared_ptr<Assembly::Instruction>> convertReturn(std::shared_ptr<AST::Return> stmt);
    std::vector<std::shared_ptr<Assembly::Instruction>> convertStmt(std::shared_ptr<AST::Statement> stmt);
    std::shared_ptr<Assembly::Function> convertFunc(std::shared_ptr<AST::FunctionDefinition> funDef);
    std::shared_ptr<Assembly::Program> gen(std::shared_ptr<AST::Program> program);
};

#endif