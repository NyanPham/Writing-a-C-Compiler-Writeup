#ifndef INITIALIZERS_H
#define INITIALIZERS_H

#include <cstdint>
#include <string>
#include <variant>

#include "Types.h"
#include "./utils/VariantHelper.h"

namespace Initializers
{
    struct IntInit
    {
        int32_t val;

        IntInit() = default;
        IntInit(int32_t val) : val{val} {}

        std::string toString() const { return "IntInit(" + std::to_string(val) + ")"; }
    };

    struct LongInit
    {
        int64_t val;

        LongInit() = default;
        LongInit(int64_t val) : val{val} {}

        std::string toString() const { return "LongInit(" + std::to_string(val) + ")"; }
    };

    struct UIntInit
    {
        uint32_t val;

        UIntInit() = default;
        UIntInit(uint32_t val) : val{val} {}

        std::string toString() const { return "UIntInit(" + std::to_string(val) + ")"; }
    };

    struct ULongInit
    {
        uint64_t val;

        ULongInit() = default;
        ULongInit(uint64_t val) : val{val} {}

        std::string toString() const { return "ULongInit(" + std::to_string(val) + ")"; }
    };

    struct DoubleInit
    {
        double val;

        DoubleInit() = default;
        DoubleInit(double val) : val{val} {}

        std::string toString() const { return "DoubleInit(" + std::to_string(val) + ")"; }
    };

    using StaticInit = std::variant<IntInit, LongInit, UIntInit, ULongInit, DoubleInit>;

    inline std::string toString(const StaticInit &staticInit)
    {
        return std::visit([](const auto &obj)
                          { return obj.toString(); }, staticInit);
    }

    inline bool isIntInit(const StaticInit &staticInit)
    {
        return isVariant<IntInit>(staticInit);
    }

    inline bool isLongInit(const StaticInit &staticInit)
    {
        return isVariant<LongInit>(staticInit);
    }

    inline bool isUIntInit(const StaticInit &staticInit)
    {
        return isVariant<UIntInit>(staticInit);
    }

    inline bool isULongInit(const StaticInit &staticInit)
    {
        return isVariant<ULongInit>(staticInit);
    }

    inline bool isDoubleInit(const StaticInit &staticInit)
    {
        return isVariant<DoubleInit>(staticInit);
    }

    inline const std::optional<IntInit> getIntInit(const StaticInit &staticInit)
    {
        return getVariant<IntInit>(staticInit);
    }

    inline const std::optional<LongInit> getLongInit(const StaticInit &staticInit)
    {
        return getVariant<LongInit>(staticInit);
    }

    inline const std::optional<UIntInit> getUIntInit(const StaticInit &staticInit)
    {
        return getVariant<UIntInit>(staticInit);
    }

    inline const std::optional<ULongInit> getULongInit(const StaticInit &staticInit)
    {
        return getVariant<ULongInit>(staticInit);
    }

    inline const std::optional<DoubleInit> getDoubleInit(const StaticInit &staticInit)
    {
        return getVariant<DoubleInit>(staticInit);
    }

    inline StaticInit zero(const Types::DataType &type)
    {
        if (Types::isIntType(type))
            return IntInit{0};
        else if (Types::isLongType(type))
            return LongInit{0};
        else if (Types::isUIntType(type))
            return UIntInit{0};
        else if (Types::isULongType(type) || Types::isPointerType(type))
            return ULongInit{0};
        else if (Types::isDoubleType(type))
            return DoubleInit{0.0};
        else
            throw std::runtime_error("Internal error: Zero doesn't make sense for function type");
    }

    inline bool isZero(const StaticInit &staticInit)
    {
        if (auto intInit = getIntInit(staticInit))
            return intInit->val == 0;
        else if (auto longInit = getLongInit(staticInit))
            return longInit->val == 0;
        else if (auto uintInit = getUIntInit(staticInit))
            return uintInit->val == 0;
        else if (auto ulongInit = getULongInit(staticInit))
            return ulongInit->val == 0;
        else if (auto doubleInit = getDoubleInit(staticInit))
            // Note: consider all double non-zero since we don't know if it's zero or negative-zero
            return false;
        else
            throw std::runtime_error("Internal error: Invalid static initializer");
    }
};

#endif