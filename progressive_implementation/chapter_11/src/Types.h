#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <variant>
#include <optional>
#include <iostream>
#include <memory>

#include "./utils/VariantHelper.h"

namespace Types
{
    struct IntType;
    struct LongType;
    struct FunType;

    using DataType = std::variant<IntType, LongType, FunType>;

    struct IntType
    {
        IntType() {}

        int getAlignment() const
        {
            return 4;
        }

        std::string toString() const
        {
            return "Int";
        }
    };

    struct LongType
    {
        LongType() {}

        int getAlignment() const
        {
            return 8;
        }

        std::string toString() const
        {
            return "Long";
        }
    };

    inline std::string dataTypeToString(const DataType &type)
    {
        return std::visit([](const auto &t)
                          { return t.toString(); }, type);
    }

    inline int getAlignment(const DataType &type)
    {
        return std::visit([](const auto &t)
                          { return t.getAlignment(); }, type);
    }

    struct FunType
    {
        std::vector<std::shared_ptr<DataType>> paramTypes;
        std::shared_ptr<DataType> retType;

        FunType(
            std::vector<std::shared_ptr<DataType>> paramTypes,
            std::shared_ptr<DataType> retType) : paramTypes(std::move(paramTypes)), retType(std::move(retType)) {}

        int getAlignment() const
        {
            throw std::runtime_error("Internal error: function type doesn't have alignment");
        }

        std::string toString() const
        {
            std::string result = "FunType(";
            result += '[';
            for (size_t i = 0; i < paramTypes.size(); ++i)
            {
                result += dataTypeToString(*paramTypes[i]);
                if (i < paramTypes.size() - 1)
                    result += ", ";
            }
            result += ']';

            result += " -> " + dataTypeToString(*retType) + ")";
            return result;
        }
    };

    using DataType = std::variant<IntType, LongType, FunType>;

    inline DataType makeIntType()
    {
        return IntType{};
    }

    inline DataType makeLongType()
    {
        return LongType{};
    }

    inline DataType makeFunType(std::vector<std::shared_ptr<DataType>> paramTypes,
                                std::shared_ptr<DataType> retType)
    {
        return FunType{paramTypes, retType};
    }

    inline std::optional<IntType> getIntType(const DataType &type)
    {
        return getVariant<IntType>(type);
    }

    inline std::optional<LongType> getLongType(const DataType &type)
    {
        return getVariant<LongType>(type);
    }

    inline std::optional<FunType> getFunType(const DataType &type)
    {
        return getVariant<FunType>(type);
    }

    inline bool isIntType(const DataType &type)
    {
        return isVariant<IntType>(type);
    }

    inline bool isLongType(const DataType &type)
    {
        return isVariant<LongType>(type);
    }

    inline bool isFunType(const DataType &type)
    {
        return isVariant<FunType>(type);
    }

    inline bool operator==(const Types::DataType &lhs, const Types::DataType &rhs)
    {
        return std::visit(
            [](const auto &left, const auto &right) -> bool
            {
                using LeftType = std::decay_t<decltype(left)>;
                using RightType = std::decay_t<decltype(right)>;
                if constexpr (std::is_same_v<LeftType, RightType>)
                {
                    if constexpr (std::is_same_v<LeftType, Types::FunType>)
                    {
                        // Compare FunType (parameter types and return type)
                        if (left.paramTypes.size() != right.paramTypes.size())
                            return false;
                        for (size_t i = 0; i < left.paramTypes.size(); ++i)
                        {
                            if (*left.paramTypes[i] != *right.paramTypes[i])
                                return false;
                        }
                        return *left.retType == *right.retType;
                    }
                    else
                    {
                        // Compare IntType or LongType (no additional fields to compare)
                        return true;
                    }
                }
                else
                {
                    return false; // Different types
                }
            },
            lhs, rhs);
    }
}

#endif