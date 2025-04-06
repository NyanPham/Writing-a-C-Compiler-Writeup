#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <variant>
#include <iostream>

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
        if (auto intType = std::get_if<IntType>(&type))
        {
            return *intType;
        }

        return std::nullopt;
    }

    inline std::optional<FunType> getFunType(const DataType &type)
    {
        if (auto funType = std::get_if<FunType>(&type))
        {
            return *funType;
        }

        return std::nullopt;
    }

    inline bool isIntType(const DataType &type)
    {
        return std::holds_alternative<IntType>(type);
    }

    inline bool isFunType(const DataType &type)
    {
        return std::holds_alternative<FunType>(type);
    }
}

#endif