#ifndef DISJOINT_SET_H
#define DISJOINT_SET_H

#include <map>
#include <stdexcept>

template <typename T>
class DisjointSet
{
public:
    DisjointSet() = default;

    bool isEmpty() const
    {
        return _parent.empty();
    }

    void unite(const T &x, const T &y)
    {
        T xRoot = find(x);
        T yRoot = find(y);
        if (xRoot == yRoot)
            return;
        _parent[xRoot] = yRoot;
    }

    T find(const T &x) const
    {
        auto it = _parent.find(x);
        if (it == _parent.end())
            return x;
        if (it->second != x)
            _parent[x] = find(it->second);
        return _parent[x];
    }

    std::map<T, T> getSet() const
    {
        return _parent;
    }

private:
    mutable std::map<T, T> _parent;
};

#endif