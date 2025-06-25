#ifndef INITIALIZERS_H
#define INITIALIZERS_H

#include <cstdint>
#include <string>
#include <variant>

#include "Types.h"
#include "./utils/VariantHelper.h"

namespace Initializers
{
    struct CharInit
    {
        int8_t val;

        CharInit() = default;
        CharInit(int8_t val) : val{val} {}

        std::string toString() const { return "CharInit(" + std::to_string(val) + ")"; }
    };

    struct UCharInit
    {
        uint8_t val;

        UCharInit() = default;
        UCharInit(uint8_t val) : val{val} {}

        std::string toString() const { return "UCharInit(" + std::to_string(val) + ")"; }
    };

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

    struct ZeroInit
    {
        size_t byteCount;

        ZeroInit() = default;
        ZeroInit(size_t byteCount) : byteCount{byteCount} {}
        std::string toString() const { return "ZeroInit(" + std::to_string(byteCount) + " bytes)"; }
    };

    struct StringInit
    {
        std::string str;
        bool nullTerminated;

        StringInit() = default;
        StringInit(const std::string &str, bool nullTerminated) : str{str}, nullTerminated{nullTerminated} {}
        std::string toString() const { return "StringInit(string=" + str + ", nullTerminated=" + (nullTerminated ? "true" : "false") + ")"; }
    };

    struct PointerInit
    {
        std::string label;

        PointerInit() = default;
        PointerInit(const std::string &label) : label{label} {}
        std::string toString() const { return "PointerInit(label=" + label + ")"; }
    };

    using StaticInit = std::variant<CharInit, UCharInit, IntInit, LongInit, UIntInit, ULongInit, DoubleInit, ZeroInit, StringInit, PointerInit>;

    inline std::string toString(const StaticInit &staticInit)
    {
        return std::visit([](const auto &obj)
                          { return obj.toString(); }, staticInit);
    }

    inline bool isCharInit(const StaticInit &staticInit)
    {
        return isVariant<CharInit>(staticInit);
    }

    inline bool isUCharInit(const StaticInit &staticInit)
    {
        return isVariant<UCharInit>(staticInit);
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

    inline bool isZeroInit(const StaticInit &staticInit)
    {
        return isVariant<ZeroInit>(staticInit);
    }

    inline bool isStringInit(const StaticInit &staticInit)
    {
        return isVariant<StringInit>(staticInit);
    }

    inline bool isPointerInit(const StaticInit &staticInit)
    {
        return isVariant<PointerInit>(staticInit);
    }

    inline std::optional<CharInit> getCharInit(const StaticInit &staticInit)
    {
        return getVariant<CharInit>(staticInit);
    }

    inline std::optional<UCharInit> getUCharInit(const StaticInit &staticInit)
    {
        return getVariant<UCharInit>(staticInit);
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

    inline const std::optional<ZeroInit> getZeroInit(const StaticInit &staticInit)
    {
        return getVariant<ZeroInit>(staticInit);
    }

    inline std::optional<StringInit> getStringInit(const StaticInit &staticInit)
    {
        return getVariant<StringInit>(staticInit);
    }

    inline std::optional<PointerInit> getPointerInit(const StaticInit &staticInit)
    {
        return getVariant<PointerInit>(staticInit);
    }

    inline std::vector<std::shared_ptr<StaticInit>> zero(const Types::DataType &type, const TypeTableNS::TypeTable &typeTable)
    {
        std::vector<std::shared_ptr<StaticInit>> result;
        result.reserve(1);
        result.emplace_back(std::make_shared<StaticInit>(ZeroInit(Types::getSize(type, typeTable))));
        return result;
    }

    inline bool isZero(const StaticInit &staticInit)
    {
        if (auto charInit = getCharInit(staticInit))
            return charInit->val == 0;
        else if (auto ucharInit = getUCharInit(staticInit))
            return ucharInit->val == 0;
        else if (auto intInit = getIntInit(staticInit))
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
        else if (auto zeroInit = getZeroInit(staticInit))
            return true;
        else if (auto pointerInit = getPointerInit(staticInit))
            return false;
        else if (auto stringInit = getStringInit(staticInit))
            return false;
        else
            throw std::runtime_error("Internal error: Invalid static initializer");
    }
};

#endif