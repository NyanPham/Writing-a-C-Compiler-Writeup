#include "Settings.h"
#include <stdexcept>
#include <cstdlib>

Settings::Settings()
    : currentPlatform(Platform::Linux),
      isDebug(false),
      optimizations{} {}

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
    optimizations = opts;
}

bool Settings::isOptimizationEnabled(const std::string &flag) const
{
    auto it = optimizations.find(flag);
    return it != optimizations.end() && it->second;
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

void Settings::setRegAllocLevel(int level)
{
    regAllocLevel = level;
    RegAllocDebugOptions opts;
    opts.debugMsg = level >= 1;
    opts.interferenceNcol = level >= 2;
    opts.interferenceGraphviz = level >= 3;
    opts.liveness = level >= 4;
    regAllocDebugOptions = opts;
}

int Settings::getRegAllocLevel() const
{
    return regAllocLevel;
}

Settings::RegAllocDebugOptions Settings::getRegAllocDebugOptions() const
{
    return regAllocDebugOptions;
}

void Settings::setRegAllocDebugOptions(const std::unordered_map<std::string, bool> &opts)
{
    // Set struct fields from opts map
    regAllocDebugOptions.debugMsg = opts.count("debugMsg") ? opts.at("debugMsg") : false;
    regAllocDebugOptions.interferenceNcol = opts.count("interferenceNcol") ? opts.at("interferenceNcol") : false;
    regAllocDebugOptions.interferenceGraphviz = opts.count("interferenceGraphviz") ? opts.at("interferenceGraphviz") : false;
    regAllocDebugOptions.liveness = opts.count("liveness") ? opts.at("liveness") : false;
}

bool Settings::isRegAllocDebugOptionEnabled(const std::string &flag) const
{
    if (flag == "debugMsg")
        return regAllocDebugOptions.debugMsg;
    if (flag == "interferenceNcol")
        return regAllocDebugOptions.interferenceNcol;
    if (flag == "interferenceGraphviz")
        return regAllocDebugOptions.interferenceGraphviz;
    if (flag == "liveness")
        return regAllocDebugOptions.liveness;
    return false;
}