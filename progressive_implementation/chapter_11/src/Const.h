#ifndef CONST_H
#define CONST_H

#include <variant>
#include <string>
#include <cstdint>
#include <optional>

#include "./utils/VariantHelper.h"

namespace Constants
{

    struct ConstInt
    {
        int32_t val;

        ConstInt(int32_t val) : val{val} {}

        std::string toString() const
        {
            return "ConstInt(" + std::to_string(val) + ")";
        }
    };

    struct ConstLong
    {
        int64_t val;

        ConstLong(int64_t val) : val{val} {}

        std::string toString() const
        {
            return "ConstLong(" + std::to_string(val) + ")";
        }
    };

    using Const = std::variant<ConstInt, ConstLong>;

    inline std::string toString(const Const &c)
    {
        return std::visit([](const auto &obj)
                          { return obj.toString(); }, c);
    }

    inline Const makeConstInt(int32_t val)
    {
        return ConstInt{val};
    }

    inline Const makeConstLong(int64_t val)
    {
        return ConstLong{val};
    }

    inline ConstInt makeIntZero()
    {
        return ConstInt{0};
    }

    inline ConstLong makeLongZero()
    {
        return ConstLong{0};
    }

    inline bool isConstInt(const Const &c)
    {
        return isVariant<ConstInt>(c);
    }

    inline bool isConstLong(const Const &c)
    {
        return isVariant<ConstLong>(c);
    }

    inline std::optional<ConstInt> getConstInt(const Const &c)
    {
        return getVariant<ConstInt>(c);
    }

    inline std::optional<ConstLong> getConstLong(const Const &c)
    {
        return getVariant<ConstLong>(c);
    }
};

#endif