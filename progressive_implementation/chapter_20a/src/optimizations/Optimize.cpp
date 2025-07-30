#include "Optimize.h"
#include "ConstantFolding.h"
#include "AddressTaken.h"
#include "UnreachableCodeElim.h"
#include "CopyPropa.h"
#include "DeadStoreElim.h"
#include "../CFG.h"
#include "../Tacky.h"
#include "../utils/TackyPrettyPrint.h"
#include <algorithm>

namespace
{

    using InstructionList = std::vector<std::shared_ptr<TACKY::Instruction>>;

    // Recursively optimize a function body until convergence
    InstructionList optimizeFun(
        const std::string &debugLabel,
        const Settings::Optimizations &opts,
        const InstructionList &instructions,
        const Symbols::SymbolTable &symbolTable,
        bool debug)
    {
        if (instructions.empty())
            return {};

        // Analyze address-taken variables
        auto aliasedVars = analyzeAddressTaken(instructions);

        // Print which optimization settings are enabled
        if (debug)
        {
            std::cout << "Optimizations enabled: ";
            bool any = false;
            if (opts.constantFolding)
            {
                std::cout << "[Constant Folding] ";
                any = true;
            }
            if (opts.unreachableCodeElimination)
            {
                std::cout << "[Unreachable Code Elimination] ";
                any = true;
            }
            if (opts.copyPropagation)
            {
                std::cout << "[Copy Propagation] ";
                any = true;
            }
            if (opts.deadStoreElimination)
            {
                std::cout << "[Dead Store Elimination] ";
                any = true;
            }
            if (!any)
            {
                std::cout << "None";
            }
            std::cout << '\n';
        }

        // Constant folding
        InstructionList constantFolded = instructions;
        if (opts.constantFolding)
        {
            constantFolded = constantFold(debugLabel, instructions, symbolTable, debug); // pass debug
        }

        // Build CFG
        auto cfg = cfg::instructionsToCFG<TACKY::Instruction>(debugLabel, constantFolded);

        // Unreachable code elimination
        cfg::Graph<std::monostate, TACKY::Instruction> cfg1 = cfg;
        if (opts.unreachableCodeElimination)
        {
            cfg1 = eliminateUnreachableCode(cfg, debug); // pass debug
        }

        // Copy propagation
        cfg::Graph<std::monostate, TACKY::Instruction> cfg2 = cfg1;
        if (opts.copyPropagation)
        {
            cfg2 = CopyPropa::optimize(aliasedVars, cfg1, symbolTable, debug); // pass debug
        }

        // Dead store elimination
        cfg::Graph<std::monostate, TACKY::Instruction> cfg3 = cfg2;
        if (opts.deadStoreElimination)
        {
            cfg3 = eliminateDeadStores(aliasedVars, cfg2, symbolTable, debug); // pass debug
        }

        // Convert back to instruction list
        InstructionList optimizedInstructions = cfg::cfgToInstructions<TACKY::Instruction>(cfg3);

        // If no change, return; else, repeat
        if (optimizedInstructions.size() == instructions.size() &&
            std::equal(
                optimizedInstructions.begin(), optimizedInstructions.end(),
                instructions.begin(),
                [](const auto &a, const auto &b)
                { return TACKY::equalInstructions(*a, *b); }))
        {
            return optimizedInstructions;
        }
        else
        {
            return optimizeFun(debugLabel, opts, optimizedInstructions, symbolTable, debug);
        }
    }

    // Helper to extract optimization flags from Settings
    Settings::Optimizations getOptimizationFlags(const Settings &settings)
    {
        Settings::Optimizations opts;
        opts.constantFolding = settings.isOptimizationEnabled("constant_folding");
        opts.unreachableCodeElimination = settings.isOptimizationEnabled("unreachable_code_elimination");
        opts.copyPropagation = settings.isOptimizationEnabled("copy_propagation");
        opts.deadStoreElimination = settings.isOptimizationEnabled("dead_store_elimination");
        return opts;
    }

}

std::shared_ptr<TACKY::Program> optimize(
    const Settings &settings,
    const std::string &srcFile,
    const TACKY::Program &tackyProgram,
    const Symbols::SymbolTable &symbolTable)
{
    using namespace TACKY;
    auto opts = getOptimizationFlags(settings);
    bool debug = settings.getIsDebug();
    std::vector<std::shared_ptr<TopLevel>> newTopLevels;
    for (const auto &tl : tackyProgram.getTopLevels())
    {
        if (auto fn = std::dynamic_pointer_cast<Function>(tl))
        {
            auto optimizedBody = optimizeFun(fn->getName(), opts, fn->getInstructions(), symbolTable, debug);
            fn->setInstructions(optimizedBody);
            newTopLevels.push_back(fn);
        }
        else
        {
            newTopLevels.push_back(tl);
        }
    }
    return std::make_shared<Program>(newTopLevels);
}