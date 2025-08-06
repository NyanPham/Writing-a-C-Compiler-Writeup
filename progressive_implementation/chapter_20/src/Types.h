#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <variant>
#include <optional>
#include <iostream>
#include <memory>
#include <vector>

#include "./utils/VariantHelper.h"

namespace TypeTableNS
{
    class TypeTable;
    struct TypeDef;
}

namespace Types
{
    struct CharType;
    struct SCharType;
    struct UCharType;
    struct IntType;
    struct LongType;
    struct UIntType;
    struct ULongType;
    struct DoubleType;
    struct FunType;
    struct PointerType;
    struct VoidType;
    struct ArrayType;
    struct StructureType;
    struct UnionType;

    using DataType = std::variant<
        CharType,
        SCharType,
        UCharType,
        IntType,
        LongType,
        UIntType,
        ULongType,
        DoubleType,
        PointerType,
        VoidType,
        ArrayType,
        StructureType,
        UnionType,
        FunType>;

    struct CharType
    {
        CharType() {}

        int getSize() const { return 1; }
        int getAlignment() const { return 1; }
        bool isSigned() const { return true; }
        std::string toString() const { return "CharType"; }
    };

    struct SCharType
    {
        SCharType() {}

        int getSize() const { return 1; }
        int getAlignment() const { return 1; }
        bool isSigned() const { return true; }
        std::string toString() const { return "SCharType"; }
    };

    struct UCharType
    {
        UCharType() {}

        int getSize() const { return 1; }
        int getAlignment() const { return 1; }
        bool isSigned() const { return false; }
        std::string toString() const { return "UCharType"; }
    };

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

    struct VoidType
    {
        VoidType() {}

        int getSize() const { throw std::runtime_error("Internal error: void type doesn't have size"); }
        int getAlignment() const { throw std::runtime_error("Internal error: void type doesn't have alignment"); }
        bool isSigned() const { throw std::runtime_error("Internal error: signedness doesn't make sense for void type"); }
        std::string toString() const { return "VoidType"; }
    };

    struct PointerType
    {
        std::shared_ptr<DataType> referencedType;

        PointerType(const std::shared_ptr<DataType> &type);

        int getSize() const { return 8; }
        int getAlignment() const { return 8; }
        bool isSigned() const { return false; }

        std::string toString() const;
    };

    struct StructureType
    {
        std::string tag;
        StructureType(const std::string &tag);

        int getSize(const TypeTableNS::TypeTable &typeTable) const;
        int getAlignment(const TypeTableNS::TypeTable &typeTable) const;
        bool isSigned() const { throw std::runtime_error("Internal error: signedness doesn't make sense for structure type"); }

        std::string toString() const;
    };

    struct UnionType
    {
        std::string tag;
        UnionType(const std::string &tag);

        int getSize(const TypeTableNS::TypeTable &typeTable) const;
        int getAlignment(const TypeTableNS::TypeTable &typeTable) const;
        bool isSigned() const { throw std::runtime_error("Internal error: signedness doesn't make sense for union type"); }

        std::string toString() const;
    };

    struct FunType
    {
        std::vector<std::shared_ptr<DataType>> paramTypes;
        std::shared_ptr<DataType> retType;

        FunType(
            std::vector<std::shared_ptr<DataType>> paramTypes,
            std::shared_ptr<DataType> retType);

        int getSize() const;
        int getAlignment() const;
        bool isSigned() const;

        std::string toString() const;
    };

    struct ArrayType
    {
        std::shared_ptr<DataType> elemType;
        int size;

        ArrayType(const std::shared_ptr<DataType> &elemType, int size);

        int getSize(const TypeTableNS::TypeTable &typeTable) const;
        int getAlignment(const TypeTableNS::TypeTable &typeTable) const;
        bool isSigned() const;

        std::string toString() const;
    };

    // --- Inline and utility functions ---

    int getSize(const DataType &type);
    int getSize(const DataType &type, const TypeTableNS::TypeTable &typeTable);
    int getAlignment(const DataType &type);
    int getAlignment(const DataType &type, const TypeTableNS::TypeTable &typeTable);
    bool isSigned(const DataType &type);
    std::string dataTypeToString(const DataType &type);

    inline DataType makeCharType() { return CharType{}; }
    inline DataType makeSCharType() { return SCharType{}; }
    inline DataType makeUCharType() { return UCharType{}; }
    inline DataType makeIntType() { return IntType{}; }
    inline DataType makeLongType() { return LongType{}; }
    inline DataType makeUIntType() { return UIntType{}; }
    inline DataType makeULongType() { return ULongType{}; }
    inline DataType makeDoubleType() { return DoubleType{}; }
    inline DataType makePointerType(const std::shared_ptr<DataType> &referencedType) { return PointerType{referencedType}; }
    inline DataType makeVoidType() { return VoidType{}; }
    inline DataType makeArrayType(const std::shared_ptr<DataType> &elemType, int size) { return ArrayType{elemType, size}; }
    inline DataType makeStructType(const std::string tag) { return StructureType{tag}; }
    inline DataType makeUnionType(const std::string tag) { return UnionType{tag}; }
    inline DataType makeFunType(std::vector<std::shared_ptr<DataType>> paramTypes, const std::shared_ptr<DataType> &retType) { return FunType{paramTypes, retType}; }

    inline std::optional<CharType> getCharType(const DataType &type) { return getVariant<CharType>(type); }
    inline std::optional<SCharType> getSCharType(const DataType &type) { return getVariant<SCharType>(type); }
    inline std::optional<UCharType> getUCharType(const DataType &type) { return getVariant<UCharType>(type); }
    inline std::optional<IntType> getIntType(const DataType &type) { return getVariant<IntType>(type); }
    inline std::optional<LongType> getLongType(const DataType &type) { return getVariant<LongType>(type); }
    inline std::optional<UIntType> getUIntType(const DataType &type) { return getVariant<UIntType>(type); }
    inline std::optional<ULongType> getULongType(const DataType &type) { return getVariant<ULongType>(type); }
    inline std::optional<DoubleType> getDoubleType(const DataType &type) { return getVariant<DoubleType>(type); }
    inline std::optional<PointerType> getPointerType(const DataType &type) { return getVariant<PointerType>(type); }
    inline std::optional<VoidType> getVoidType(const DataType &type) { return getVariant<VoidType>(type); }
    inline std::optional<ArrayType> getArrayType(const DataType &type) { return getVariant<ArrayType>(type); }
    inline std::optional<StructureType> getStructType(const DataType &type) { return getVariant<StructureType>(type); }
    inline std::optional<UnionType> getUnionType(const DataType &type) { return getVariant<UnionType>(type); }
    inline std::optional<FunType> getFunType(const DataType &type) { return getVariant<FunType>(type); }

    inline bool isCharType(const DataType &type) { return isVariant<CharType>(type); }
    inline bool isSCharType(const DataType &type) { return isVariant<SCharType>(type); }
    inline bool isUCharType(const DataType &type) { return isVariant<UCharType>(type); }
    inline bool isIntType(const DataType &type) { return isVariant<IntType>(type); }
    inline bool isLongType(const DataType &type) { return isVariant<LongType>(type); }
    inline bool isUIntType(const DataType &type) { return isVariant<UIntType>(type); }
    inline bool isULongType(const DataType &type) { return isVariant<ULongType>(type); }
    inline bool isDoubleType(const DataType &type) { return isVariant<DoubleType>(type); }
    inline bool isPointerType(const DataType &type) { return isVariant<PointerType>(type); }
    inline bool isVoidType(const DataType &type) { return isVariant<VoidType>(type); }
    inline bool isArrayType(const DataType &type) { return isVariant<ArrayType>(type); }
    inline bool isStructType(const DataType &type) { return isVariant<StructureType>(type); }
    inline bool isUnionType(const DataType &type) { return isVariant<UnionType>(type); }
    inline bool isFunType(const DataType &type) { return isVariant<FunType>(type); }

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
                        if (left.paramTypes.size() != right.paramTypes.size())
                            return false;
                        for (size_t i = 0; i < left.paramTypes.size(); ++i)
                        {
                            if (*left.paramTypes[i] != *right.paramTypes[i])
                                return false;
                        }
                        return static_cast<bool>(*left.retType == *right.retType);
                    }
                    else if constexpr (std::is_same_v<LeftType, Types::PointerType>)
                    {
                        return static_cast<bool>(*left.referencedType == *right.referencedType);
                    }
                    else if constexpr (std::is_same_v<LeftType, Types::ArrayType>)
                    {
                        return static_cast<bool>(left.size == right.size && *left.elemType == *right.elemType);
                    }
                    else if constexpr (std::is_same_v<LeftType, Types::StructureType>)
                    {
                        return static_cast<bool>(left.tag == right.tag);
                    }
                    else if constexpr (std::is_same_v<LeftType, Types::UnionType>)
                    {
                        return static_cast<bool>(left.tag == right.tag);
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

    inline bool isArithmetic(const Types::DataType &type)
    {
        if (
            Types::isIntType(type) ||
            Types::isUIntType(type) ||
            Types::isLongType(type) ||
            Types::isULongType(type) ||
            Types::isDoubleType(type) ||
            Types::isCharType(type) ||
            Types::isSCharType(type) ||
            Types::isUCharType(type))
        {
            return true;
        }

        if (Types::isFunType(type) || Types::isPointerType(type) || Types::isArrayType(type) || Types::isStructType(type) || Types::isUnionType(type))
        {
            return false;
        }

        throw std::runtime_error("Internal error: unknown type");
    }

    inline bool isInteger(const Types::DataType &type)
    {
        if (
            Types::isCharType(type) ||
            Types::isSCharType(type) ||
            Types::isUCharType(type) ||
            Types::isIntType(type) ||
            Types::isUIntType(type) ||
            Types::isLongType(type) ||
            Types::isULongType(type))
        {
            return true;
        }

        return false;
    }

    inline bool isCharacter(const Types::DataType &type)
    {
        return getSize(type) == 1;
    }

    inline bool isScalar(const Types::DataType &type)
    {
        if (Types::isArrayType(type) || Types::isVoidType(type) || Types::isFunType(type) || Types::isStructType(type) || Types::isUnionType(type))
            return false;
        return true;
    }

    bool isComplete(const DataType &type, const TypeTableNS::TypeTable &typeTable);
    bool isCompletePointer(const DataType &type, const TypeTableNS::TypeTable &typeTable);

    const TypeTableNS::TypeDef &getTypeDef(const std::string &tag, const TypeTableNS::TypeTable &typeTable);
}

#endif