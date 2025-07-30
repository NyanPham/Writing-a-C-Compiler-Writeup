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
#include "optimizations/Optimize.h"
#include "optimizations/AddressTaken.h"
#include "backend/CodeGen.h"
#include "backend/RegAlloc.h"
#include "backend/ReplacePseudos.h"
#include "backend/InstructionFixup.h"
#include "Emit.h"
#include "utils/ASTPrettyPrint.h"
#include "utils/SymbolTablePrint.h"
#include "utils/TypeTablePrint.h"
#include "utils/TackyPrettyPrint.h"
#include "utils/CodeGenPrettyPrint.h"
#include "utils/AssemblySymbolTablePrint.h"

std::string Compiler::preprocess(const std::string &src)
{
    _settings.validateExtension(src);
    std::string output = _settings.replaceExtension(src, ".i");
    _settings.runCommand("gcc", {"-E", "-P", src, "-o", output});
    return output;
}

// Helper to extract all instructions from a TACKY::Program
std::vector<std::shared_ptr<TACKY::Instruction>> getAllInstructions(const std::shared_ptr<TACKY::Program> &program)
{
    std::vector<std::shared_ptr<TACKY::Instruction>> result;
    for (const auto &topLevel : program->getTopLevels())
    {
        if (auto func = std::dynamic_pointer_cast<TACKY::Function>(topLevel))
        {
            const auto &instrs = func->getInstructions();
            result.insert(result.end(), instrs.begin(), instrs.end());
        }
    }
    return result;
}

int Compiler::compile(Stage stage, const std::vector<std::string> &srcFiles)
{
    bool debugging = _settings.getIsDebug();
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
            TypeTablePrint typeTablePrint;
            TackyPrettyPrint tackyPrettyPrint;
            CodeGenPrettyPrint codeGenPrettyPrint;
            AssemblySymbolTablePrint asmSymbolTablePrint;

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

                auto typeChecker = TypeChecker();
                auto typeCheckedAst = typeChecker.typeCheck(labeledAst);

                auto switchCasesCollector = CollectSwitchCases();
                auto casesCollectedAst = switchCasesCollector.analyzeSwitches(typeCheckedAst);

                if (debugging)
                {
                    std::cout << "AST:" << '\n';
                    astPrettyPrint.print(*ast);

                    std::cout << "ID Resolved:" << '\n';
                    astPrettyPrint.print(*transformedAst);

                    // std::cout << "Validated labels:" << '\n';
                    // astPrettyPrint.print(*validatedASt);

                    // std::cout << "LoopLabeled:" << '\n';
                    // astPrettyPrint.print(*labeledAst);

                    symbolTablePrint.print(typeChecker.getSymbolTable());
                    typeTablePrint.print(typeChecker.getTypeTable());
                    std::cout << "TypeChecked:" << '\n';
                    astPrettyPrint.print(*typeCheckedAst);

                    // std::cout << "Cases Collected:" << '\n';
                    // astPrettyPrint.print(*casesCollectedAst);
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

                auto typeChecker = TypeChecker();
                auto typeCheckedAst = typeChecker.typeCheck(labeledAst);

                auto switchCasesCollector = CollectSwitchCases();
                auto casesCollectedAst = switchCasesCollector.analyzeSwitches(typeCheckedAst);

                auto tackyGen = TackyGen(typeChecker.getSymbolTable(), typeChecker.getTypeTable());
                auto tacky = tackyGen.gen(casesCollectedAst);

                if (debugging)
                {
                    symbolTablePrint.print(typeChecker.getSymbolTable());
                    typeTablePrint.print(typeChecker.getTypeTable());
                    std::cout << "TACKY:" << '\n';
                    tackyPrettyPrint.print(*tacky);
                }

                auto optimizedTacky = optimize(_settings, file, *tacky, typeChecker.getSymbolTable());

                if (debugging)
                {
                    std::cout << "Optimized TACKY:" << '\n';
                    tackyPrettyPrint.print(*optimizedTacky);
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

                auto typeChecker = TypeChecker();
                auto typeCheckedAst = typeChecker.typeCheck(labeledAst);

                auto switchCasesCollector = CollectSwitchCases();
                auto casesCollectedAst = switchCasesCollector.analyzeSwitches(typeCheckedAst);

                auto tackyGen = TackyGen(typeChecker.getSymbolTable(), typeChecker.getTypeTable());
                auto tacky = tackyGen.gen(casesCollectedAst);

                auto optimizedTacky = optimize(_settings, file, *tacky, typeChecker.getSymbolTable());

                auto codeGen = CodeGen(typeChecker.getSymbolTable(), typeChecker.getTypeTable());
                auto asmProg = codeGen.gen(optimizedTacky);

                auto aliasedPseudos = analyzeProgram(optimizedTacky->getTopLevels());
                auto regAllocedAsmProg = allocateRegisters(aliasedPseudos, asmProg, codeGen.getAsmSymbolTable(), _settings);

                auto replacePseudos = ReplacePseudos(codeGen.getAsmSymbolTable());
                auto replacedAsm = replacePseudos.replacePseudos(regAllocedAsmProg);

                auto instructionFixup = InstructionFixup(codeGen.getAsmSymbolTable());
                auto fixedupAsm = instructionFixup.fixupProgram(replacedAsm);

                if (debugging)
                {
                    symbolTablePrint.print(typeChecker.getSymbolTable());
                    std::cout << "======= RAW ASSEMBLY =======" << '\n';
                    codeGenPrettyPrint.print(*asmProg);
                    std::cout << '\n';

                    asmSymbolTablePrint.print(codeGen.getAsmSymbolTable());

                    symbolTablePrint.print(typeChecker.getSymbolTable());
                    std::cout << "======= REG ALLOCATED ASSEMBLY =======" << '\n';
                    codeGenPrettyPrint.print(*regAllocedAsmProg);
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

                auto typeChecker = TypeChecker();
                auto typeCheckedAst = typeChecker.typeCheck(labeledAst);

                auto switchCasesCollector = CollectSwitchCases();
                auto casesCollectedAst = switchCasesCollector.analyzeSwitches(typeCheckedAst);

                auto tackyGen = TackyGen(typeChecker.getSymbolTable(), typeChecker.getTypeTable());
                auto tacky = tackyGen.gen(casesCollectedAst);

                auto optimizedTacky = optimize(_settings, file, *tacky, typeChecker.getSymbolTable());

                auto codeGen = CodeGen(typeChecker.getSymbolTable(), typeChecker.getTypeTable());
                auto asmProg = codeGen.gen(optimizedTacky);

                auto aliasedPseudos = analyzeProgram(optimizedTacky->getTopLevels());
                auto regAllocedAsmProg = allocateRegisters(aliasedPseudos, asmProg, codeGen.getAsmSymbolTable(), _settings);

                auto replacePseudos = ReplacePseudos(codeGen.getAsmSymbolTable());
                auto replacedAsm = replacePseudos.replacePseudos(regAllocedAsmProg);

                auto instructionFixup = InstructionFixup(codeGen.getAsmSymbolTable());
                auto fixedupAsm = instructionFixup.fixupProgram(replacedAsm);

                Emit emitter = Emit(codeGen.getAsmSymbolTable());

                std::string asmFile = _settings.replaceExtension(file, ".s");
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

                auto typeChecker = TypeChecker();
                auto typeCheckedAst = typeChecker.typeCheck(labeledAst);

                auto switchCasesCollector = CollectSwitchCases();
                auto casesCollectedAst = switchCasesCollector.analyzeSwitches(typeCheckedAst);

                auto tackyGen = TackyGen(typeChecker.getSymbolTable(), typeChecker.getTypeTable());
                auto tacky = tackyGen.gen(casesCollectedAst);

                auto optimizedTacky = optimize(_settings, file, *tacky, typeChecker.getSymbolTable());

                auto codeGen = CodeGen(typeChecker.getSymbolTable(), typeChecker.getTypeTable());
                auto asmProg = codeGen.gen(optimizedTacky);

                auto aliasedPseudos = analyzeProgram(optimizedTacky->getTopLevels());
                auto regAllocedAsmProg = allocateRegisters(aliasedPseudos, asmProg, codeGen.getAsmSymbolTable(), _settings);

                auto replacePseudos = ReplacePseudos(codeGen.getAsmSymbolTable());
                auto replacedAsm = replacePseudos.replacePseudos(regAllocedAsmProg);

                auto instructionFixup = InstructionFixup(codeGen.getAsmSymbolTable());
                auto fixedupAsm = instructionFixup.fixupProgram(replacedAsm);

                Emit emitter = Emit(codeGen.getAsmSymbolTable());

                std::string asmFile = _settings.replaceExtension(file, ".s");
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

                auto typeChecker = TypeChecker();
                auto typeCheckedAst = typeChecker.typeCheck(labeledAst);

                auto switchCasesCollector = CollectSwitchCases();
                auto casesCollectedAst = switchCasesCollector.analyzeSwitches(typeCheckedAst);

                auto tackyGen = TackyGen(typeChecker.getSymbolTable(), typeChecker.getTypeTable());
                auto tacky = tackyGen.gen(casesCollectedAst);

                auto optimizedTacky = optimize(_settings, file, *tacky, typeChecker.getSymbolTable());

                auto codeGen = CodeGen(typeChecker.getSymbolTable(), typeChecker.getTypeTable());
                auto asmProg = codeGen.gen(optimizedTacky);

                auto aliasedPseudos = analyzeProgram(optimizedTacky->getTopLevels());
                auto regAllocedAsmProg = allocateRegisters(aliasedPseudos, asmProg, codeGen.getAsmSymbolTable(), _settings);

                auto replacePseudos = ReplacePseudos(codeGen.getAsmSymbolTable());
                auto replacedAsm = replacePseudos.replacePseudos(regAllocedAsmProg);

                auto instructionFixup = InstructionFixup(codeGen.getAsmSymbolTable());
                auto fixedupAsm = instructionFixup.fixupProgram(replacedAsm);

                Emit emitter = Emit(codeGen.getAsmSymbolTable());

                std::string asmFile = _settings.replaceExtension(file, ".s");
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
        std::string asmFile = _settings.replaceExtension(src, ".s");
        std::string objFile = _settings.replaceExtension(src, ".o");
        objFiles.push_back(objFile);

        // Assemble each source file into an object file
        _settings.runCommand("gcc", {asmFile, "-c", "-o", objFile});

        if (cleanUp)
        {
            _settings.runCommand("rm", {asmFile});
        }
    }

    if (link)
    {
        // Link all object files into a single executable
        std::string outputFile = _settings.removeExtension(srcFiles[0]); // Use the first file's name for the executable
        std::vector<std::string> gccArgs = objFiles;
        gccArgs.push_back("-o");
        gccArgs.push_back(outputFile);

        _settings.runCommand("gcc", gccArgs);

        if (cleanUp)
        {
            for (const auto &objFile : objFiles)
            {
                _settings.runCommand("rm", {objFile});
            }
        }
    }
}
