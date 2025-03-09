#include "Settings.h"
#include <stdexcept>
#include <cstdlib>

Settings::Settings() : currentPlatform(Platform::Linux), isDebug(false) {}

void Settings::getCurrentPlatform()
{
    currentPlatform = Platform::Linux;
}

void Settings::validateExtension(const std::string &filename)
{
    if (filename.size() >= 2 && (filename.compare(filename.size() - 2, 2, ".c") == 0 || filename.compare(filename.size() - 2, 2, ".h") == 0))
    {
        // Valid extension, do nothing
    }
    else
    {
        throw std::invalid_argument("Invalid extension: " + filename);
    }
}

std::string Settings::replaceExtension(const std::string &filename, const std::string &newExt)
{
    std::string base = filename.substr(0, filename.find_last_of("."));
    return base + newExt;
}

void Settings::runCommand(const std::string &cmd, const std::vector<std::string> &args)
{
    std::string fullCmd = cmd;
    for (const auto &arg : args)
    {
        fullCmd += " " + arg;
    }
    system(fullCmd.c_str());
}

bool Settings::getIsDebug() const
{
    return isDebug;
}

void Settings::setIsDebug(bool debug)
{
    isDebug = debug;
}