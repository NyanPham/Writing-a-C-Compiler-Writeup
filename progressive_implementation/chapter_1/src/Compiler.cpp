#include <fstream>
#include <iostream>
#include <stdexcept>

#include "Compiler.h"
#include "Lexer.h"
#include "Parser.h"
#include "CodeGen.h"
#include "Emit.h"
#include "utils/PrettyPrint.h"

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

        auto input = buffer.str();
        auto prettyPrint = PrettyPrint();

        switch (stage)
        {
        case Stage::Lexing:
        {
            Lexer lexer = Lexer();
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
            Parser parser = Parser();
            auto program = parser.parse(input);

            if (debugging)
                prettyPrint.print(*program);

            return 0;
        }

        case Stage::CodeGen:
        {
            Parser parser = Parser();
            auto ast = parser.parse(input);

            CodeGen codeGen = CodeGen();
            auto asmProg = codeGen.gen(ast);

            if (debugging)
                prettyPrint.print(*asmProg);

            return 0;
        }

        case Stage::Emit:
        {
            Parser parser = Parser();
            auto ast = parser.parse(input);

            CodeGen codeGen = CodeGen();
            auto asmProg = codeGen.gen(ast);

            Emit emitter = Emit();

            std::string asmFile = settings.replaceExtension(src, ".s");
            emitter.emit(asmProg, asmFile);

            return 0;
        }

        default:
        {
            Parser parser = Parser();
            auto ast = parser.parse(input);

            CodeGen codeGen = CodeGen();
            auto asmProg = codeGen.gen(ast);

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
