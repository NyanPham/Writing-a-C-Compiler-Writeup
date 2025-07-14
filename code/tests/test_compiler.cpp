#include "TestFramework.h"
#include <iostream>
#include <string>
#include <cstring>

bool g_redirectOutput = false;
std::string g_redirectToFile;

int main(int argc, char *argv[])
{
    std::string chapterFilter;
    std::string stageFilter;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--redirect-output") {
            g_redirectOutput = true;
        } else if (arg.rfind("--redirect-to-file=", 0) == 0) {
            g_redirectToFile = arg.substr(strlen("--redirect-to-file="));
        } else if (chapterFilter.empty()) {
            chapterFilter = arg;
        } else if (stageFilter.empty()) {
            stageFilter = arg;
        }
    }

    TestFramework::runTests(chapterFilter, stageFilter);
    return 0;
}