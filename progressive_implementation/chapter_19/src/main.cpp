#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>

#include "Settings.h"
#include "Compiler.h"

Stage parseStage(const std::string &stageStr)
{
    if (stageStr == "--lex")
        return Stage::Lexing;
    else if (stageStr == "--parse")
        return Stage::Parsing;
    else if (stageStr == "--validate")
        return Stage::Validate;
    else if (stageStr == "--tacky")
        return Stage::Tacky;
    else if (stageStr == "--codegen")
        return Stage::CodeGen;
    else if (stageStr == "--emit")
        return Stage::Emit;
    else if (stageStr == "--object")
        return Stage::Object;
    else
        return Stage::Executable;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <src1> <src2> ... --<stage> [--debug]"
                     " [--constant_folding] [--dead_store_elimination] ...\n";
        return 1;
    }

    // 1. Prepare containers
    std::vector<std::string> srcFiles;
    bool debugging = false;
    std::string stageStr;

    // 2. Define all of your optimization flags
    std::unordered_map<std::string, bool> optimizations = {
        {"--constant_folding", false},
        {"--dead_store_elimination", false},
        {"--copy_propagation", false},
        {"--unreachable_code_elimination", false},
        {"--optimize", false}};

    // 3. Parse each argument
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        // Debug flag
        if (arg == "--debug")
        {
            debugging = true;
        }
        // Any double-dash flag
        else if (arg.rfind("--", 0) == 0)
        {
            // If it's one of our known optimizations, mark it
            if (optimizations.count(arg))
            {
                optimizations[arg] = true;
            }
            // Otherwise assume it's the stage flag
            else
            {
                stageStr = arg;
            }
        }
        // Otherwise treat as a source file
        else
        {
            srcFiles.push_back(arg);
        }
    }

    // 4. Ensure a stage was provided
    if (stageStr.empty())
    {
        std::cerr << "Error: missing stage flag (e.g., --lex, --parse, etc.)\n";
        return 1;
    }

    // 5. If --optimize is set, enable all other optimization passes
    if (optimizations["--optimize"])
    {
        for (auto &kv : optimizations)
        {
            if (kv.first != "--optimize")
            {
                kv.second = true;
            }
        }
    }

    // 6. Configure Settings
    // Convert optimization flags (strip "--")
    std::unordered_map<std::string, bool> strippedOptimizations;
    for (const auto &kv : optimizations)
    {
        std::string key = kv.first;
        if (key.rfind("--", 0) == 0)
        {
            key = key.substr(2); // Remove leading "--"
        }
        strippedOptimizations[key] = kv.second;
    }

    Settings settings;
    settings.setIsDebug(debugging);
    settings.setOptimizations(strippedOptimizations);

    // 7. Invoke the compiler
    Stage stage = parseStage(stageStr);
    Compiler compiler(settings);
    int status = compiler.compile(stage, srcFiles);

    return (status == 0 ? 0 : 1);
}
