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

    void collectLabelsFromStatement(std::set<std::string> &definedLabels, std::set<std::string> &usedLabels, const std::shared_ptr<AST::Statement> &stmt);
    void collectLabelsFromBlockItem(std::set<std::string> &definedLabels, std::set<std::string> &usedLabels, const std::shared_ptr<AST::BlockItem> &blockItem);
    void validateLabelsInFun(const std::shared_ptr<AST::FunctionDefinition> &funDef);
    void validateLabels(const std::shared_ptr<AST::Program> &prog);
};

#endif