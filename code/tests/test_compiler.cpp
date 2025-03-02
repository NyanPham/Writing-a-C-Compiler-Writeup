#include "TestFramework.h"
#include "Compiler.h"
#include "Settings.h"

#include <iostream>

int main(int argc, char *argv[])
{
    std::string chapterFilter;
    std::string stageFilter;

    if (argc > 1)
    {
        chapterFilter = argv[1];
    }
    if (argc > 2)
    {
        stageFilter = argv[2];
    }

    TestFramework::runTests(chapterFilter, stageFilter);
    return 0;
}