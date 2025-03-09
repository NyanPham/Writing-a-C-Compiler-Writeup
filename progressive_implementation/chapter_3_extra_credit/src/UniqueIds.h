#ifndef UNIQUE_IDS_H
#define UNIQUE_IDS_H

#include <string>
#include <mutex>

class UniqueIds
{
public:
    static std::string makeTemporary();

private:
    static int _counter;
    static std::mutex _mutex;
};

#endif