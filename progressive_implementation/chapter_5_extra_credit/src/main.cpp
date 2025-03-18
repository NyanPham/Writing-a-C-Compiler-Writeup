#include <vector>
#include <string>
#include <iostream>
#include "Settings.h"
#include "Compiler.h"

Stage parseStage(const std::string &stageStr)
{
    if (stageStr == "--lex")
    {
        return Stage::Lexing;
    }
    else if (stageStr == "--parse")
    {
        return Stage::Parsing;
    }
    else if (stageStr == "--validate")
    {
        return Stage::Validate;
    }
    else if (stageStr == "--tacky")
    {
        return Stage::Tacky;
    }
    else if (stageStr == "--codegen")
    {
        return Stage::CodeGen;
    }
    else if (stageStr == "--emit")
    {
        return Stage::Emit;
    }
    else
    {
        return Stage::Executable;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << " --<stage>\n";
        return 1;
    }

    std::string srcFile{argv[1]};
    Stage stage{parseStage(argv[2])};

    bool debugging = argv[3] && std::string(argv[3]) == "--debug";

    Settings settings;
    Compiler compiler;

    int status = compiler.compile(stage, srcFile, debugging);

    if (status != 0)
    {
        return 1;
    }

    return 0;
}