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
    struct UIntType;
    struct ULongType;
    struct DoubleType;
    struct FunType;
    struct PointerType;

    using DataType = std::variant<IntType, LongType, UIntType, ULongType, DoubleType, PointerType, FunType>;

    struct IntType
    {
        IntType() {}

        int getSize() const { return 4; }
        int getAlignment() const { return 4; }
        bool isSigned() const { return true; }
        std::string toString() const { return "IntType"; }
    };

    struct LongType
    {
        LongType() {}

        int getSize() const { return 8; }
        int getAlignment() const { return 8; }
        bool isSigned() const { return true; }
        std::string toString() const { return "LongType"; }
    };

    struct UIntType
    {
        UIntType() {}

        int getSize() const { return 4; }
        int getAlignment() const { return 4; }
        bool isSigned() const { return false; }
        std::string toString() const { return "UIntType"; }
    };

    struct ULongType
    {
        ULongType() {}

        int getSize() const { return 8; }
        int getAlignment() const { return 8; }
        bool isSigned() const { return false; }
        std::string toString() const { return "ULongType"; }
    };

    struct DoubleType
    {
        DoubleType() {}

        int getSize() const { return 8; }
        int getAlignment() const { return 8; }
        bool isSigned() const { throw std::runtime_error("Internal error: signedness doens't make sense for double type"); }
        std::string toString() const { return "DoubleType"; }
    };

    inline int getSize(const DataType &type)
    {
        return std::visit([](const auto &t)
                          { return t.getSize(); }, type);
    }

    inline int getAlignment(const DataType &type)
    {
        return std::visit([](const auto &t)
                          { return t.getAlignment(); }, type);
    }

    inline bool isSigned(const DataType &type)
    {
        return std::visit([](const auto &t)
                          { return t.isSigned(); }, type);
    }

    std::string dataTypeToString(const DataType &type);

    struct PointerType
    {
        std::shared_ptr<DataType> referencedType;

        PointerType(const std::shared_ptr<DataType> &type) : referencedType(std::move(type)) {}

        int getSize() const { return 8; }
        int getAlignment() const { return 8; }
        bool isSigned() const { return false; }

        std::string toString() const;
    };

    struct FunType
    {
        std::vector<std::shared_ptr<DataType>> paramTypes;
        std::shared_ptr<DataType> retType;

        FunType(
            std::vector<std::shared_ptr<DataType>> paramTypes,
            std::shared_ptr<DataType> retType) : paramTypes(std::move(paramTypes)), retType(std::move(retType)) {}

        int getSize() const
        {
            throw std::runtime_error("Internal error: function type doesn't have size");
        }

        int getAlignment() const
        {
            throw std::runtime_error("Internal error: function type doesn't have alignment");
        }

        bool isSigned() const
        {
            throw std::runtime_error("Internal error: signedness doesn't make sense for function type");
        }

        std::string toString() const;
    };

    inline std::string PointerType::toString() const
    {
        std::string result = "PointerType(referencedType: ";
        result += dataTypeToString(*referencedType);
        result += ")";
        return result;
    }

    inline std::string FunType::toString() const
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

    inline std::string dataTypeToString(const DataType &type)
    {
        return std::visit([](const auto &t)
                          { return t.toString(); }, type);
    }

    inline DataType makeIntType()
    {
        return IntType{};
    }

    inline DataType makeLongType()
    {
        return LongType{};
    }

    inline DataType makeUIntType()
    {
        return UIntType{};
    }

    inline DataType makeULongType()
    {
        return ULongType{};
    }

    inline DataType makeDoubleType()
    {
        return DoubleType{};
    }

    inline DataType makePointerType(const std::shared_ptr<DataType> &referencedType)
    {
        return PointerType{referencedType};
    }

    inline DataType makeFunType(std::vector<std::shared_ptr<DataType>> paramTypes,
                                const std::shared_ptr<DataType> &retType)
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

    inline std::optional<UIntType> getUIntType(const DataType &type)
    {
        return getVariant<UIntType>(type);
    }

    inline std::optional<ULongType> getULongType(const DataType &type)
    {
        return getVariant<ULongType>(type);
    }

    inline std::optional<DoubleType> getDoubleType(const DataType &type)
    {
        return getVariant<DoubleType>(type);
    }

    inline std::optional<PointerType> getPointerType(const DataType &type)
    {
        return getVariant<PointerType>(type);
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

    inline bool isUIntType(const DataType &type)
    {
        return isVariant<UIntType>(type);
    }

    inline bool isULongType(const DataType &type)
    {
        return isVariant<ULongType>(type);
    }

    inline bool isDoubleType(const DataType &type)
    {
        return isVariant<DoubleType>(type);
    }

    inline bool isPointerType(const DataType &type)
    {
        return isVariant<PointerType>(type);
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
                    else if constexpr (std::is_same_v<LeftType, Types::PointerType>)
                    {
                        return static_cast<bool>(*left.referencedType == *right.referencedType);
                    }
                    else
                    {
                        return true;
                    }
                }
                else
                {
                    return false;
                }
            },
            lhs, rhs);
    }
}

#endif