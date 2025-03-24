#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include "Settings.h"

enum class Stage
{
    Lexing,
    Parsing,
    Validate,
    Tacky,
    CodeGen,
    Emit,
    Executable,
};

class Compiler
{
public:
    Compiler() {}

    std::string preprocess(const std::string &src);
    int compile(Stage stage, const std::string &src, bool debugging = false);
    void assembleAndLink(const std::string &src, bool cleanUp = true);

private:
    Settings settings;
};

#endif