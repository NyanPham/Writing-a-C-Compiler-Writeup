#include <string>
#include "UniqueIds.h"

int UniqueIds::_counter = 0;
std::mutex UniqueIds::_mutex;

std::string UniqueIds::makeTemporary()
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::string name = "tmp." + std::to_string(_counter);
    _counter++;

    return std::move(name);
}

std::string UniqueIds::makeLabel(const std::string &prefix)
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::string name = prefix + "." + std::to_string(_counter);
    _counter++;

    return std::move(name);
}

std::string UniqueIds::makeNamedTemporary(const std::string &name)
{
    return makeLabel(name);
}
