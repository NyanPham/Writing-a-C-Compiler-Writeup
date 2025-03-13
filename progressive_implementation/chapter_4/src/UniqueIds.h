#ifndef UNIQUE_IDS_H
#define UNIQUE_IDS_H

#include <string>
#include <mutex>

class UniqueIds
{
public:
    static std::string makeTemporary();
    static std::string makeLabel(const std::string& prefix);

private:
    static int _counter;
    static std::mutex _mutex;
};

#endif