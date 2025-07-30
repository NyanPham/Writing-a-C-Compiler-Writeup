#ifndef VARIANT_HELPER_H
#define VARIANT_HELPER_H

#include <variant>
#include <optional>

template <typename T, typename Variant>
inline std::optional<T> getVariant(const Variant &variant)
{
    if (auto value = std::get_if<T>(&variant))
    {
        return *value;
    }
    return std::nullopt;
}

template <typename T, typename Variant>
inline bool isVariant(const Variant &variant)
{
    return std::holds_alternative<T>(variant);
}

#endif