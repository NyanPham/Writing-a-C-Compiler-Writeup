#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <vector>
#include <unordered_map>

enum class Platform
{
    Linux,
    OS_X
};

class Settings
{
public:
    Settings();

    void getCurrentPlatform();
    void validateExtension(const std::string &filename);
    std::string removeExtension(const std::string &filename);
    std::string replaceExtension(const std::string &filename, const std::string &newExt);
    void runCommand(const std::string &cmd, const std::vector<std::string> &args);

    bool getIsDebug() const;
    void setIsDebug(bool debug);

    // Optimization-flag interface
    void setOptimizations(const std::unordered_map<std::string, bool> &opts);
    bool isOptimizationEnabled(const std::string &flag) const;

    // RegAlloc options flag interface
    void setRegAllocDebugOptions(const std::unordered_map<std::string, bool> &opts);
    bool isRegAllocDebugOptionEnabled(const std::string &flag) const;

    void setRegAllocLevel(int level);
    int getRegAllocLevel() const;

    // Struct for easy access to all optimization flags
    struct Optimizations
    {
        bool constantFolding = false;
        bool unreachableCodeElimination = false;
        bool copyPropagation = false;
        bool deadStoreElimination = false;
    };
    Optimizations getOptimizations() const;

    // Struct for easy access to the options to debug register alloc
    struct RegAllocDebugOptions
    {
        bool debugMsg = false;
        bool interferenceNcol = false;
        bool interferenceGraphviz = false;
        bool liveness = false;
    };
    RegAllocDebugOptions getRegAllocDebugOptions() const;

private:
    Platform currentPlatform;
    bool isDebug;
    int regAllocLevel = 0;
    std::unordered_map<std::string, bool> optimizations;
    RegAllocDebugOptions regAllocDebugOptions;
};

#endif
