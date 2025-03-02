#include "../TestFramework.h"
#include "Compiler.h"
#include "Settings.h"

#include <fstream>
#include <vector>
#include <string>
#include <iostream>

TEST_CASE(Chapter1ValidLex, "chapter_1", "--lexing")
{
    std::vector<std::string> srcFiles = {
        "tests/chapter_1/valid/multi_digit.c",
        "tests/chapter_1/valid/newlines.c",
        "tests/chapter_1/valid/no_newlines.c",
        "tests/chapter_1/valid/return_0.c",
        "tests/chapter_1/valid/return_2.c",
        "tests/chapter_1/valid/spaces.c",
        "tests/chapter_1/valid/tabs.c",
    };
    Settings settings;

    for (const auto &srcFile : srcFiles)
    {
        Compiler compiler;
        try
        {
            int status = compiler.compile(Stage::Lexing, srcFile);
            ASSERT_TRUE(status == 0);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error compiling file " << srcFile << ": " << e.what() << std::endl;
            throw;
        }
    }
}