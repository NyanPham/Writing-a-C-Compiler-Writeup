#ifndef CONST_H
#define CONST_H

#include <variant>
#include <string>
#include <cstdint>
#include <optional>

#include "Types.h"
#include "./utils/VariantHelper.h"

namespace Constants
{
    struct ConstInt
    {
        int32_t val;

        ConstInt(int32_t val) : val{val} {}
        std::string toString() const { return "ConstInt(" + std::to_string(val) + ")"; }
    };

    struct ConstLong
    {
        int64_t val;

        ConstLong(int64_t val) : val{val} {}
        std::string toString() const { return "ConstLong(" + std::to_string(val) + ")"; }
    };

    struct ConstUInt
    {
        uint32_t val;

        ConstUInt(uint32_t val) : val{val} {}
        std::string toString() const { return "ConstUInt(" + std::to_string(val) + ")"; }
    };

    struct ConstULong
    {
        uint64_t val;

        ConstULong(uint64_t val) : val{val} {}
        std::string toString() const { return "ConstULong(" + std::to_string(val) + ")"; }
    };

    struct ConstDouble
    {
        double val;

        ConstDouble(double val) : val{val} {}
        std::string toString() const { return "ConstDouble(" + std::to_string(val) + ")"; }
    };

    using Const = std::variant<ConstInt, ConstLong, ConstUInt, ConstULong, ConstDouble>;

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

    inline Const makeConstUInt(uint32_t val)
    {
        return ConstUInt{val};
    }

    inline Const makeConstULong(uint64_t val)
    {
        return ConstULong{val};
    }

    inline Const makeConstDouble(double val)
    {
        return ConstDouble{val};
    }

    inline ConstInt makeIntZero()
    {
        return ConstInt{0};
    }

    inline ConstLong makeLongZero()
    {
        return ConstLong{0};
    }

    inline ConstUInt makeUIntZero()
    {
        return ConstUInt{0};
    }

    inline ConstULong makeULongZero()
    {
        return ConstULong{0};
    }

    inline bool isConstInt(const Const &c)
    {
        return isVariant<ConstInt>(c);
    }

    inline bool isConstLong(const Const &c)
    {
        return isVariant<ConstLong>(c);
    }

    inline bool isConstUInt(const Const &c)
    {
        return isVariant<ConstUInt>(c);
    }

    inline bool isConstULong(const Const &c)
    {
        return isVariant<ConstULong>(c);
    }

    inline bool isConstDouble(const Const &c)
    {
        return isVariant<ConstDouble>(c);
    }

    inline std::optional<ConstInt> getConstInt(const Const &c)
    {
        return getVariant<ConstInt>(c);
    }

    inline std::optional<ConstLong> getConstLong(const Const &c)
    {
        return getVariant<ConstLong>(c);
    }

    inline std::optional<ConstUInt> getConstUInt(const Const &c)
    {
        return getVariant<ConstUInt>(c);
    }

    inline std::optional<ConstULong> getConstULong(const Const &c)
    {
        return getVariant<ConstULong>(c);
    }

    inline std::optional<ConstDouble> getConstDouble(const Const &c)
    {
        return getVariant<ConstDouble>(c);
    }

    inline Types::DataType typeOfConst(const Const &c)
    {
        if (isConstInt(c))
            return Types::makeIntType();
        if (isConstLong(c))
            return Types::makeLongType();
        if (isConstUInt(c))
            return Types::makeUIntType();
        if (isConstULong(c))
            return Types::makeULongType();
        if (isConstDouble(c))
            return Types::makeDoubleType();
        throw std::runtime_error("Internal error: unknown const type");
    }
};

#endif