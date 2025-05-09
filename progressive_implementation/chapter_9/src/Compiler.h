#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <vector>
#include "Settings.h"

enum class Stage
{
    Lexing,
    Parsing,
    Validate,
    Tacky,
    CodeGen,
    Emit,
    Object,
    Executable,
};

class Compiler
{
public:
    Compiler() {}

    std::string preprocess(const std::string &src);
    int compile(Stage stage, const std::vector<std::string> &srcFiles, bool debugging = false);
    void assembleAndLink(const std::vector<std::string> &srcFiles, bool link = true, bool cleanUp = true);

private:
    Settings settings;
};

#endif