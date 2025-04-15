#ifndef VALIDATE_LABELS_H
#define VALIDATE_LABELS_H

#include <optional>
#include <memory>
#include <string>
#include <set>

#include "AST.h"

class ValidateLabels
{
public:
    ValidateLabels() = default;

    std::shared_ptr<AST::Statement> collectLabelsFromStatement(std::set<std::string> &definedLabels, std::set<std::string> &usedLabels, const std::shared_ptr<AST::Statement> &stmt, std::function<std::string(const std::string &)> transformLabel);
    std::shared_ptr<AST::BlockItem> collectLabelsFromBlockItem(std::set<std::string> &definedLabels, std::set<std::string> &usedLabels, const std::shared_ptr<AST::BlockItem> &blockItem, std::function<std::string(const std::string &)> transformLabel);
    std::shared_ptr<AST::FunctionDeclaration> validateLabelsInFun(const std::shared_ptr<AST::FunctionDeclaration> &fnDecl);
    std::shared_ptr<AST::Declaration> validateLabelsInDecl(const std::shared_ptr<AST::Declaration> &decl);
    std::shared_ptr<AST::Program> validateLabels(const std::shared_ptr<AST::Program> &prog);
};

#endif