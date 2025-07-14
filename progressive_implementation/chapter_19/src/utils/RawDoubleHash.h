#ifndef RAW_DOUBLE_HASH_H
#define RAW_DOUBLE_HASH_H

#include <bit>
#include <cstdint>
#include <functional>

// A hasher that reinterprets the double’s bits as uint64_t
struct RawDoubleHash
{
    size_t operator()(double d) const noexcept
    {
        auto u = std::bit_cast<std::uint64_t>(d);
        return std::hash<std::uint64_t>{}(u);
    }
};

// An equality comparator that does a bit-wise compare
struct RawDoubleEq
{
    bool operator()(double a, double b) const noexcept
    {
        return std::bit_cast<std::uint64_t>(a) == std::bit_cast<std::uint64_t>(b);
    }
};

#endif
