#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <variant>
#include <optional>
#include <iostream>

#include "./utils/VariantHelper.h"

namespace Types
{
    struct IntType
    {
        std::string toString() const
        {
            return "Int";
        }
    };

    struct FunType
    {
        int paramCount;

        FunType(int paramCount) : paramCount(paramCount) {}

        std::string toString() const
        {
            return "FunType(" + std::to_string(paramCount) + ")";
        }
    };

    using DataType = std::variant<IntType, FunType>;

    inline std::string dataTypeToString(const DataType &type)
    {
        return std::visit([](const auto &t)
                          { return t.toString(); }, type);
    }

    inline DataType makeInt()
    {
        return IntType{};
    }

    inline DataType makeFunType(int paramCount)
    {
        return FunType{paramCount};
    }

    inline std::optional<IntType> getIntType(const DataType &type)
    {
        return getVariant<IntType>(type);
    }

    inline std::optional<FunType> getFunType(const DataType &type)
    {
        return getVariant<FunType>(type);
    }

    inline bool isIntType(const DataType &type)
    {
        return isVariant<IntType>(type);
    }

    inline bool isFunType(const DataType &type)
    {
        return isVariant<FunType>(type);
    }
}

#endif