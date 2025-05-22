#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <vector>

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

private:
    Platform currentPlatform;
    bool isDebug;
};

#endif