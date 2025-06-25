#include "Types.h"
#include "./TypeTable.h"

namespace Types
{

    PointerType::PointerType(const std::shared_ptr<DataType> &type)
        : referencedType(type) {}

    std::string PointerType::toString() const
    {
        std::string result = "PointerType(referencedType: ";
        result += dataTypeToString(*referencedType);
        result += ")";
        return result;
    }

    StructureType::StructureType(const std::string &tag)
        : tag(tag) {}

    int StructureType::getSize(const TypeTableNS::TypeTable &typeTable) const
    {
        return typeTable.find(tag).size;
    }

    int StructureType::getAlignment(const TypeTableNS::TypeTable &typeTable) const
    {
        return typeTable.find(tag).alignment;
    }

    std::string StructureType::toString() const
    {
        return "StructureType(tag: " + tag + ")";
    }

    FunType::FunType(
        std::vector<std::shared_ptr<DataType>> paramTypes,
        std::shared_ptr<DataType> retType)
        : paramTypes(std::move(paramTypes)), retType(std::move(retType)) {}

    int FunType::getSize() const
    {
        throw std::runtime_error("Internal error: function type doesn't have size");
    }

    int FunType::getAlignment() const
    {
        throw std::runtime_error("Internal error: function type doesn't have alignment");
    }

    bool FunType::isSigned() const
    {
        throw std::runtime_error("Internal error: signedness doesn't make sense for function type");
    }

    std::string FunType::toString() const
    {
        std::string result = "FunType([";
        for (size_t i = 0; i < paramTypes.size(); ++i)
        {
            result += dataTypeToString(*paramTypes[i]);
            if (i < paramTypes.size() - 1)
                result += ", ";
        }
        result += "] -> ";
        result += dataTypeToString(*retType);
        result += ")";
        return result;
    }

    ArrayType::ArrayType(const std::shared_ptr<DataType> &elemType, int size)
        : elemType(elemType), size(size) {}

    int ArrayType::getSize(const TypeTableNS::TypeTable &typeTable) const
    {
        return size * Types::getSize(*elemType, typeTable);
    }

    int ArrayType::getAlignment(const TypeTableNS::TypeTable &typeTable) const
    {
        return Types::getAlignment(*elemType, typeTable);
    }

    bool ArrayType::isSigned() const
    {
        throw std::runtime_error("Internal error: signedness doesn't make sense for array type");
    }

    std::string ArrayType::toString() const
    {
        std::string result = "ArrayType(elemType: ";
        result += dataTypeToString(*elemType);
        result += ", size: ";
        result += std::to_string(size);
        result += ")";
        return result;
    }

    // Utility functions

    int getSize(const DataType &type)
    {
        return std::visit([](const auto &t) -> int
                          {
            using T = std::decay_t<decltype(t)>;
            if constexpr (std::is_same_v<T, StructureType>)
                throw std::runtime_error("StructureType::getSize() requires a TypeTable argument");
            else if constexpr (std::is_same_v<T, PointerType>)
                throw std::runtime_error("PointerType::getSize() requires a TypeTable argument");
            else if constexpr (std::is_same_v<T, ArrayType>)
                throw std::runtime_error("ArrayType::getSize() requires a TypeTable argument");
            else
                return t.getSize(); }, type);
    }

    int getSize(const DataType &type, const TypeTableNS::TypeTable &typeTable)
    {
        return std::visit([&typeTable](const auto &t) -> int
                          {
            using T = std::decay_t<decltype(t)>;
            if constexpr (std::is_same_v<T, StructureType>)
                return t.getSize(typeTable);
            else if constexpr (std::is_same_v<T, ArrayType>)
                return t.getSize(typeTable);
            else
                return t.getSize(); }, type);
    }

    int getAlignment(const DataType &type)
    {
        return std::visit([](const auto &t) -> int
                          {
            using T = std::decay_t<decltype(t)>;
            if constexpr (std::is_same_v<T, StructureType>)
                throw std::runtime_error("StructureType::getAlignment() requires a TypeTable argument");
            else if constexpr (std::is_same_v<T, PointerType>)
                throw std::runtime_error("PointerType::getAlignment() requires a TypeTable argument");
            else if constexpr (std::is_same_v<T, ArrayType>)
                throw std::runtime_error("ArrayType::getAlignment() requires a TypeTable argument");
            else
                return t.getAlignment(); }, type);
    }

    int getAlignment(const DataType &type, const TypeTableNS::TypeTable &typeTable)
    {
        return std::visit([&typeTable](const auto &t) -> int
                          {
            using T = std::decay_t<decltype(t)>;
            if constexpr (std::is_same_v<T, StructureType>)
                return t.getAlignment(typeTable);
            else if constexpr (std::is_same_v<T, ArrayType>)
                return t.getAlignment(typeTable);
            else
                return t.getAlignment(); }, type);
    }

    bool isSigned(const DataType &type)
    {
        return std::visit([](const auto &t)
                          { return t.isSigned(); }, type);
    }

    std::string dataTypeToString(const DataType &type)
    {
        return std::visit([](const auto &t)
                          { return t.toString(); }, type);
    }

    bool isComplete(const DataType &type, const TypeTableNS::TypeTable &typeTable)
    {
        if (isVoidType(type))
            return false;

        if (auto strctType = getStructType(type))
            return typeTable.has(strctType->tag);

        return true;
    }

    bool isCompletePointer(const DataType &type, const TypeTableNS::TypeTable &typeTable)
    {
        if (auto pointerType = getPointerType(type))
        {
            return isComplete(*pointerType->referencedType, typeTable);
        }

        return false;
    }
}