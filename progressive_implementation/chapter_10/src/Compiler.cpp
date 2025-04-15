#include <fstream>
#include <iostream>
#include <stdexcept>

#include "Compiler.h"
#include "Lexer.h"
#include "Parser.h"
#include "UniqueIds.h"
#include "Symbols.h"
#include "semantic_analysis/IdentifierResolution.h"
#include "semantic_analysis/ValidateLabels.h"
#include "semantic_analysis/LoopLabeling.h"
#include "semantic_analysis/CollectSwitchCases.h"
#include "semantic_analysis/TypeChecker.h"
#include "TackyGen.h"
#include "backend/CodeGen.h"
#include "backend/ReplacePseudos.h"
#include "backend/InstructionFixup.h"
#include "Emit.h"
#include "utils/ASTPrettyPrint.h"
#include "utils/SymbolTablePrint.h"
#include "utils/TackyPrettyPrint.h"
#include "utils/CodeGenPrettyPrint.h"

std::string Compiler::preprocess(const std::string &src)
{
    settings.validateExtension(src);
    std::string output = settings.replaceExtension(src, ".i");
    settings.runCommand("gcc", {"-E", "-P", src, "-o", output});
    return output;
}

int Compiler::compile(Stage stage, const std::vector<std::string> &srcFiles, bool debugging)
{
    try
    {
        std::vector<std::string> preprocessedFiles;

        for (const auto &src : srcFiles)
        {
            std::string preprocessedFile = preprocess(src);
            preprocessedFiles.push_back(preprocessedFile);
        }

        // Process each file based on the stage
        for (const auto &file : preprocessedFiles)
        {
            std::ifstream inputFile(file);
            if (!inputFile.is_open())
            {
                std::cerr << "Error: Could not open file " << file << std::endl;
                return -1;
            }

            std::stringstream buffer;
            buffer << inputFile.rdbuf();
            inputFile.close();

            if (std::remove(file.c_str()) != 0)
            {
                std::cerr << "Warning: Unable to remove temporary file " << file << std::endl;
            }

            auto input = buffer.str();
            ASTPrettyPrint astPrettyPrint;
            SymbolTablePrint symbolTablePrint;
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

                break;
            }

            case Stage::Parsing:
            {
                auto parser = Parser();
                auto program = parser.parse(input);

                if (debugging)
                    astPrettyPrint.print(*program);

                break;
            }

            case Stage::Validate:
            {
                auto parser = Parser();
                auto ast = parser.parse(input);

                auto IdResover = IdentifierResolution();
                auto transformedAst = IdResover.resolve(ast);

                auto validateLabels = ValidateLabels();
                auto validatedASt = validateLabels.validateLabels(transformedAst);

                auto loopLabeler = LoopLabeling();
                auto labeledAst = loopLabeler.labelLoops(transformedAst);

                auto switchCasesCollector = CollectSwitchCases();
                auto casesCollectedAst = switchCasesCollector.analyzeSwitches(labeledAst);

                auto typeChecker = TypeChecker();
                auto typeCheckedAst = typeChecker.typeCheck(casesCollectedAst);

                if (debugging)
                {
                    std::cout << "AST:" << '\n';
                    astPrettyPrint.print(*ast);

                    std::cout << "ID Resolved:" << '\n';
                    astPrettyPrint.print(*transformedAst);

                    std::cout << "Validated labels:" << '\n';
                    astPrettyPrint.print(*validatedASt);

                    std::cout << "LoopLabeled:" << '\n';
                    astPrettyPrint.print(*labeledAst);

                    std::cout << "Cases Collected:" << '\n';
                    astPrettyPrint.print(*casesCollectedAst);

                    symbolTablePrint.print(typeChecker.getSymbolTable());
                    std::cout << "TypeChecked:" << '\n';
                    astPrettyPrint.print(*typeCheckedAst);
                }

                break;
            }

            case Stage::Tacky:
            {
                auto parser = Parser();
                auto ast = parser.parse(input);

                auto IdResover = IdentifierResolution();
                auto transformedAst = IdResover.resolve(ast);

                auto validateLabels = ValidateLabels();
                auto validatedASt = validateLabels.validateLabels(transformedAst);

                auto loopLabeler = LoopLabeling();
                auto labeledAst = loopLabeler.labelLoops(transformedAst);

                auto switchCasesCollector = CollectSwitchCases();
                auto casesCollectedAst = switchCasesCollector.analyzeSwitches(labeledAst);

                auto typeChecker = TypeChecker();
                auto typeCheckedAst = typeChecker.typeCheck(casesCollectedAst);
                auto symbolTable = typeChecker.getSymbolTable();

                auto tackyGen = TackyGen(symbolTable);
                auto tacky = tackyGen.gen(typeCheckedAst);

                if (debugging)
                {
                    symbolTablePrint.print(symbolTable);
                    std::cout << "TACKY:" << '\n';
                    tackyPrettyPrint.print(*tacky);
                }

                break;
            }

            case Stage::CodeGen:
            {
                auto parser = Parser();
                auto ast = parser.parse(input);

                auto IdResover = IdentifierResolution();
                auto transformedAst = IdResover.resolve(ast);

                auto validateLabels = ValidateLabels();
                auto validatedASt = validateLabels.validateLabels(transformedAst);

                auto loopLabeler = LoopLabeling();
                auto labeledAst = loopLabeler.labelLoops(transformedAst);

                auto switchCasesCollector = CollectSwitchCases();
                auto casesCollectedAst = switchCasesCollector.analyzeSwitches(labeledAst);

                auto typeChecker = TypeChecker();
                auto typeCheckedAst = typeChecker.typeCheck(casesCollectedAst);
                auto symbolTable = typeChecker.getSymbolTable();

                auto tackyGen = TackyGen(symbolTable);
                auto tacky = tackyGen.gen(typeCheckedAst);

                auto codeGen = CodeGen(symbolTable);
                auto asmProg = codeGen.gen(tacky);

                auto replacePseudos = ReplacePseudos(symbolTable);
                auto replacedAsm = replacePseudos.replacePseudos(asmProg);

                auto instructionFixup = InstructionFixup(symbolTable);
                auto fixedupAsm = instructionFixup.fixupProgram(replacedAsm);

                if (debugging)
                {
                    symbolTablePrint.print(symbolTable);
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

                break;
            }

            case Stage::Emit:
            {
                auto parser = Parser();
                auto ast = parser.parse(input);

                auto IdResover = IdentifierResolution();
                auto transformedAst = IdResover.resolve(ast);

                auto validateLabels = ValidateLabels();
                auto validatedASt = validateLabels.validateLabels(transformedAst);

                auto loopLabeler = LoopLabeling();
                auto labeledAst = loopLabeler.labelLoops(transformedAst);

                auto switchCasesCollector = CollectSwitchCases();
                auto casesCollectedAst = switchCasesCollector.analyzeSwitches(labeledAst);

                auto typeChecker = TypeChecker();
                auto typeCheckedAst = typeChecker.typeCheck(casesCollectedAst);
                auto symbolTable = typeChecker.getSymbolTable();

                auto tackyGen = TackyGen(symbolTable);
                auto tacky = tackyGen.gen(typeCheckedAst);

                auto codeGen = CodeGen(symbolTable);
                auto asmProg = codeGen.gen(tacky);

                auto replacePseudos = ReplacePseudos(symbolTable);
                auto replacedAsm = replacePseudos.replacePseudos(asmProg);

                auto instructionFixup = InstructionFixup(symbolTable);
                auto fixedupAsm = instructionFixup.fixupProgram(replacedAsm);

                Emit emitter = Emit();

                std::string asmFile = settings.replaceExtension(file, ".s");
                emitter.emit(fixedupAsm, asmFile);

                break;
            }

            case Stage::Object:
            {
                auto parser = Parser();
                auto ast = parser.parse(input);

                auto IdResover = IdentifierResolution();
                auto transformedAst = IdResover.resolve(ast);

                auto validateLabels = ValidateLabels();
                auto validatedASt = validateLabels.validateLabels(transformedAst);

                auto loopLabeler = LoopLabeling();
                auto labeledAst = loopLabeler.labelLoops(transformedAst);

                auto switchCasesCollector = CollectSwitchCases();
                auto casesCollectedAst = switchCasesCollector.analyzeSwitches(labeledAst);

                auto typeChecker = TypeChecker();
                auto typeCheckedAst = typeChecker.typeCheck(casesCollectedAst);
                auto symbolTable = typeChecker.getSymbolTable();

                auto tackyGen = TackyGen(symbolTable);
                auto tacky = tackyGen.gen(typeCheckedAst);

                auto codeGen = CodeGen(symbolTable);
                auto asmProg = codeGen.gen(tacky);

                auto replacePseudos = ReplacePseudos(symbolTable);
                auto replacedAsm = replacePseudos.replacePseudos(asmProg);

                auto instructionFixup = InstructionFixup(symbolTable);
                auto fixedupAsm = instructionFixup.fixupProgram(replacedAsm);

                Emit emitter = Emit();

                std::string asmFile = settings.replaceExtension(file, ".s");
                emitter.emit(fixedupAsm, asmFile);

                assembleAndLink(srcFiles, false, !debugging);
                break;
            }

            default:
            {
                auto parser = Parser();
                auto ast = parser.parse(input);

                auto IdResover = IdentifierResolution();
                auto transformedAst = IdResover.resolve(ast);

                auto validateLabels = ValidateLabels();
                auto validatedASt = validateLabels.validateLabels(transformedAst);

                auto loopLabeler = LoopLabeling();
                auto labeledAst = loopLabeler.labelLoops(transformedAst);

                auto switchCasesCollector = CollectSwitchCases();
                auto casesCollectedAst = switchCasesCollector.analyzeSwitches(labeledAst);

                auto typeChecker = TypeChecker();
                auto typeCheckedAst = typeChecker.typeCheck(casesCollectedAst);
                auto symbolTable = typeChecker.getSymbolTable();

                auto tackyGen = TackyGen(symbolTable);
                auto tacky = tackyGen.gen(typeCheckedAst);

                auto codeGen = CodeGen(symbolTable);
                auto asmProg = codeGen.gen(tacky);

                auto replacePseudos = ReplacePseudos(symbolTable);
                auto replacedAsm = replacePseudos.replacePseudos(asmProg);

                auto instructionFixup = InstructionFixup(symbolTable);
                auto fixedupAsm = instructionFixup.fixupProgram(replacedAsm);

                Emit emitter = Emit();

                std::string asmFile = settings.replaceExtension(file, ".s");
                emitter.emit(fixedupAsm, asmFile);

                assembleAndLink(srcFiles, true, !debugging);
                break;
            }
            }
        }

        return 0;
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

void Compiler::assembleAndLink(const std::vector<std::string> &srcFiles, bool link, bool cleanUp)
{
    std::vector<std::string> objFiles;

    for (const auto &src : srcFiles)
    {
        std::string asmFile = settings.replaceExtension(src, ".s");
        std::string objFile = settings.replaceExtension(src, ".o");
        objFiles.push_back(objFile);

        // Assemble each source file into an object file
        settings.runCommand("gcc", {asmFile, "-c", "-o", objFile});

        if (cleanUp)
        {
            settings.runCommand("rm", {asmFile});
        }
    }

    if (link)
    {
        // Link all object files into a single executable
        std::string outputFile = settings.removeExtension(srcFiles[0]); // Use the first file's name for the executable
        std::vector<std::string> gccArgs = objFiles;
        gccArgs.push_back("-o");
        gccArgs.push_back(outputFile);

        settings.runCommand("gcc", gccArgs);

        if (cleanUp)
        {
            for (const auto &objFile : objFiles)
            {
                settings.runCommand("rm", {objFile});
            }
        }
    }
}
