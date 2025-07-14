#include "Settings.h"
#include <stdexcept>
#include <cstdlib>

Settings::Settings()
    : currentPlatform(Platform::Linux),
      isDebug(false),
      optimizations_{} {}

void Settings::getCurrentPlatform()
{
    // stub: detect OS if you like
    currentPlatform = Platform::Linux;
}

void Settings::validateExtension(const std::string &filename)
{
    if (filename.size() < 2 ||
        (filename.compare(filename.size() - 2, 2, ".c") != 0 &&
         filename.compare(filename.size() - 2, 2, ".h") != 0))
    {
        throw std::invalid_argument("Invalid extension: " + filename);
    }
}

std::string Settings::removeExtension(const std::string &filename)
{
    return filename.substr(0, filename.find_last_of("."));
}

std::string Settings::replaceExtension(const std::string &filename, const std::string &newExt)
{
    std::string base = filename.substr(0, filename.find_last_of("."));
    return base + newExt;
}

void Settings::runCommand(const std::string &cmd, const std::vector<std::string> &args)
{
    std::string fullCmd = cmd;
    for (auto &arg : args)
    {
        fullCmd += " " + arg;
    }
    std::system(fullCmd.c_str());
}

bool Settings::getIsDebug() const
{
    return isDebug;
}

void Settings::setIsDebug(bool debug)
{
    isDebug = debug;
}

void Settings::setOptimizations(const std::unordered_map<std::string, bool> &opts)
{
    optimizations_ = opts;
}

bool Settings::isOptimizationEnabled(const std::string &flag) const
{
    auto it = optimizations_.find(flag);
    return it != optimizations_.end() && it->second;
}

Settings::Optimizations Settings::getOptimizations() const
{
    Optimizations o;
    o.constantFolding = isOptimizationEnabled("--constant_folding");
    o.unreachableCodeElimination = isOptimizationEnabled("--unreachable_code_elimination");
    o.copyPropagation = isOptimizationEnabled("--copy_propagation");
    o.deadStoreElimination = isOptimizationEnabled("--dead_store_elimination");
    return o;
}