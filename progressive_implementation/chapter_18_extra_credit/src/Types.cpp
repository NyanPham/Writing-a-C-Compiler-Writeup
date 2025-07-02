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
        return getTypeDef(tag, typeTable).size;
    }

    int StructureType::getAlignment(const TypeTableNS::TypeTable &typeTable) const
    {
        return getTypeDef(tag, typeTable).alignment;
    }

    std::string StructureType::toString() const
    {
        return "StructureType(tag: " + tag + ")";
    }

    UnionType::UnionType(const std::string &tag)
        : tag(tag) {}

    int UnionType::getSize(const TypeTableNS::TypeTable &typeTable) const
    {
        return getTypeDef(tag, typeTable).size;
    }

    int UnionType::getAlignment(const TypeTableNS::TypeTable &typeTable) const
    {
        return getTypeDef(tag, typeTable).alignment;
    }

    std::string UnionType::toString() const
    {
        return "UnionType(tag: " + tag + ")";
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
            else if constexpr (std::is_same_v<T, UnionType>)
                throw std::runtime_error("UnionType::getSize() requires a TypeTable argument");
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
            else if constexpr (std::is_same_v<T, UnionType>)
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
            else if constexpr (std::is_same_v<T, UnionType>)
                throw std::runtime_error("UnionType::getAlignment() requires a TypeTable argument");
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
            else if constexpr (std::is_same_v<T, UnionType>)
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

    /* Helper to check whether tag has type table entry w/ member info*/
    bool isComplete(const DataType &type, const TypeTableNS::TypeTable &typeTable)
    {
        auto tagComplete = [&](const std::string &tag) -> bool
        {
            auto optEntry = typeTable.findOpt(tag);
            if (optEntry.has_value() && optEntry->optTypeDef.has_value())
                return true;
            else
                // Otherwise, either type isn't in type table, or it's only declared, not defined
                return false;
        };

        if (Types::isVoidType(type))
            return false;
        if (auto strctType = Types::getStructType(type))
            return tagComplete(strctType->tag);
        if (auto unionType = Types::getUnionType(type))
            return tagComplete(unionType->tag);
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

    /* Helper to get definition from type table by tag, or throw error if not defined */
    const TypeTableNS::TypeDef &getTypeDef(const std::string &tag, const TypeTableNS::TypeTable &typeTable)
    {
        std::optional<const TypeTableNS::TypeEntry> optTypeEntry = typeTable.findOpt(tag);
        if (!optTypeEntry.has_value() || !optTypeEntry->optTypeDef.has_value())
            throw std::runtime_error("No definition found for " + tag);
        else // type_def.kind and type_entry.type_def are not null
            return optTypeEntry->optTypeDef.value();
    }
}