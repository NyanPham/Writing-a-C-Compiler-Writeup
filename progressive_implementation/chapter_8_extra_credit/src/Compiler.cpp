#include <fstream>
#include <iostream>
#include <stdexcept>

#include "Compiler.h"
#include "Lexer.h"
#include "Parser.h"
#include "UniqueIds.h"
#include "semantic_analysis/VarResolution.h"
#include "semantic_analysis/ValidateLabels.h"
#include "semantic_analysis/LoopLabeling.h"
#include "semantic_analysis/CollectSwitchCases.h"
#include "TackyGen.h"
#include "Emit.h"
#include "backend/CodeGen.h"
#include "backend/ReplacePseudos.h"
#include "backend/InstructionFixup.h"
#include "utils/ASTPrettyPrint.h"
#include "utils/TackyPrettyPrint.h"
#include "utils/CodeGenPrettyPrint.h"

std::string Compiler::preprocess(const std::string &src)
{
    settings.validateExtension(src);
    std::string output = settings.replaceExtension(src, ".i");
    settings.runCommand("gcc", {"-E", "-P", src, "-o", output});
    return output;
}

int Compiler::compile(Stage stage, const std::string &src, bool debugging)
{
    try
    {
        std::string preprocessedFile = preprocess(src);
        std::ifstream file(preprocessedFile);

        if (!file.is_open())
        {
            std::cerr << "Error: Could not open file " << preprocessedFile << std::endl;
            return -1;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        // Clean up the preprocessed file after reading it.
        if (std::remove(preprocessedFile.c_str()) != 0)
        {
            std::cerr << "Warning: Unable to remove temporary file " << preprocessedFile << std::endl;
        }

        auto input = buffer.str();
        ASTPrettyPrint astPrettyPrint;
        TackyPrettyPrint tackyPrettyPrint;
        CodeGenPrettyPrint codeGenPrettyPrint;

        switch (stage)
        {
        case Stage::Lexing:
        {
            auto lexer = Lexer();
            lexer.setInput(input);
            lexer.defineTokenDefs();

            std::vector<Token> tokens = lexer.tokens();

            if (debugging)
            {
                for (Token token : tokens)
                {
                    std::cout << token.toString() << '\n';
                }
            }

            return 0;
        }

        case Stage::Parsing:
        {
            auto parser = Parser();
            auto program = parser.parse(input);

            if (debugging)
                astPrettyPrint.print(*program);

            return 0;
        }

        case Stage::Validate:
        {
            auto parser = Parser();
            auto ast = parser.parse(input);

            auto varResolution = VarResolution();
            auto transformedAst = varResolution.resolve(ast);

            auto validateLabels = ValidateLabels();
            validateLabels.validateLabels(transformedAst);

            auto loopLabeling = LoopLabeling();
            auto labeledAst = loopLabeling.labelLoops(transformedAst);

            auto collectSwitchCases = CollectSwitchCases();
            auto casesCollectedAst = collectSwitchCases.analyzeSwitches(labeledAst);

            if (debugging)
            {
                // std::cout << "AST:" << '\n';
                // astPrettyPrint.print(*ast);

                // std::cout << "Var Resolved:" << '\n';
                // astPrettyPrint.print(*transformedAst);

                // std::cout << "LoopLabeled:" << '\n';
                // astPrettyPrint.print(*labeledAst);

                // std::cout << "Cases Collected:" << '\n';
                astPrettyPrint.print(*casesCollectedAst);
            }

            return 0;
        }

        case Stage::Tacky:
        {
            auto parser = Parser();
            auto ast = parser.parse(input);

            auto varResolution = VarResolution();
            auto transformedAst = varResolution.resolve(ast);

            auto validateLabels = ValidateLabels();
            validateLabels.validateLabels(transformedAst);

            auto loopLabeling = LoopLabeling();
            auto labeledAst = loopLabeling.labelLoops(transformedAst);

            auto collectSwitchCases = CollectSwitchCases();
            auto casesCollectedAst = collectSwitchCases.analyzeSwitches(labeledAst);

            auto tackyGen = std::make_shared<TackyGen>();
            auto tacky = tackyGen->gen(casesCollectedAst);

            if (debugging)
                tackyPrettyPrint.print(*tacky);

            return 0;
        }

        case Stage::CodeGen:
        {
            auto parser = Parser();
            auto ast = parser.parse(input);

            auto varResolution = VarResolution();
            auto transformedAst = varResolution.resolve(ast);

            auto validateLabels = ValidateLabels();
            validateLabels.validateLabels(transformedAst);

            auto loopLabeling = LoopLabeling();
            auto labeledAst = loopLabeling.labelLoops(transformedAst);

            auto collectSwitchCases = CollectSwitchCases();
            auto casesCollectedAst = collectSwitchCases.analyzeSwitches(labeledAst);

            auto tackyGen = TackyGen();
            auto tacky = tackyGen.gen(casesCollectedAst);

            auto codeGen = CodeGen();
            auto asmProg = codeGen.gen(tacky);

            auto replacePseudos = ReplacePseudos();
            auto [replacedAsm, lastStackSlot] = replacePseudos.replacePseudos(asmProg);

            auto instructionFixup = InstructionFixup();
            auto fixedupAsm = instructionFixup.fixupProgram(replacedAsm, lastStackSlot);

            if (debugging)
            {
                std::cout << "======= RAW ASSEMBLY =======" << '\n';
                codeGenPrettyPrint.print(*asmProg);
                std::cout << '\n';

                std::cout << "======= OPERANDS REPLACED ASSEMBLY =======" << '\n';
                codeGenPrettyPrint.print(*replacedAsm);
                std::cout << '\n';

                std::cout << "======= INSTRUCTIONS FIXEDUP ASSEMBLY =======" << '\n';
                codeGenPrettyPrint.print(*fixedupAsm);
                std::cout << '\n';
            }

            return 0;
        }

        case Stage::Emit:
        {
            auto parser = Parser();
            auto ast = parser.parse(input);

            auto varResolution = VarResolution();
            auto transformedAst = varResolution.resolve(ast);

            auto validateLabels = ValidateLabels();
            validateLabels.validateLabels(transformedAst);

            auto loopLabeling = LoopLabeling();
            auto labeledAst = loopLabeling.labelLoops(transformedAst);

            auto collectSwitchCases = CollectSwitchCases();
            auto casesCollectedAst = collectSwitchCases.analyzeSwitches(labeledAst);

            auto tackyGen = TackyGen();
            auto tacky = tackyGen.gen(casesCollectedAst);

            auto codeGen = CodeGen();
            auto asmProg = codeGen.gen(tacky);

            auto replacePseudos = ReplacePseudos();
            auto [replacedAsm, lastStackSlot] = replacePseudos.replacePseudos(asmProg);

            auto instructionFixup = InstructionFixup();
            auto fixedupAsm = instructionFixup.fixupProgram(replacedAsm, lastStackSlot);

            Emit emitter = Emit();

            std::string asmFile = settings.replaceExtension(src, ".s");
            emitter.emit(fixedupAsm, asmFile);

            return 0;
        }

        default:
        {
            auto parser = Parser();
            auto ast = parser.parse(input);

            auto varResolution = VarResolution();
            auto transformedAst = varResolution.resolve(ast);

            auto validateLabels = ValidateLabels();
            validateLabels.validateLabels(transformedAst);

            auto loopLabeling = LoopLabeling();
            auto labeledAst = loopLabeling.labelLoops(transformedAst);

            auto collectSwitchCases = CollectSwitchCases();
            auto casesCollectedAst = collectSwitchCases.analyzeSwitches(labeledAst);

            auto tackyGen = TackyGen();
            auto tacky = tackyGen.gen(casesCollectedAst);

            CodeGen codeGen = CodeGen();
            auto asmProg = codeGen.gen(tacky);

            Emit emitter = Emit();

            std::string asmFile = settings.replaceExtension(src, ".s");
            emitter.emit(asmProg, asmFile);

            assembleAndLink(src, false);

            return 0;
        }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Compilation error: " << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "Unknown compilation error" << std::endl;
        return -1;
    }
}

void Compiler::assembleAndLink(const std::string &src, bool cleanUp)
{
    std::string asmFile = settings.replaceExtension(src, ".s");
    std::string objFile = settings.replaceExtension(src, ".o");
    settings.runCommand("gcc", {asmFile, "-o", objFile});
    if (cleanUp)
    {
        settings.runCommand("rm", {asmFile});
    }
}
