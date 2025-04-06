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
    else if (stageStr == "--object")
    {
        return Stage::Object;
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
        std::cerr << "Usage: " << argv[0] << " <source_file1> <source_file2> ... --<stage> [--debug]\n";
        return 1;
    }

    std::vector<std::string> srcFiles;
    bool debugging = false;
    std::string stageStr;

    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--debug")
        {
            debugging = true;
        }
        else if (argv[i][0] == '-' && argv[i][1] == '-')
        {
            stageStr = argv[i];
        }
        else
        {
            srcFiles.push_back(argv[i]);
        }
    }

    if (stageStr.empty())
    {
        std::cerr << "Error: No stage flag provided (e.g., --lex, --parse, --validate, etc.)\n";
        return 1;
    }

    Stage stage = parseStage(stageStr);

    Settings settings;
    Compiler compiler;

    int status = compiler.compile(stage, srcFiles, debugging);

    if (status != 0)
    {
        return 1;
    }

    return 0;
}