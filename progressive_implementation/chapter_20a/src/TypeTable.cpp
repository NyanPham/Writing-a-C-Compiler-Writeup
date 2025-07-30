#include "TypeTable.h"
#include "Types.h"
#include <algorithm>
#include <stdexcept>
#include <optional>

namespace TypeTableNS
{

    MemberEntry::MemberEntry() = default;

    MemberEntry::MemberEntry(const std::shared_ptr<Types::DataType> &memberType, int offset)
        : memberType{memberType}, offset{offset} {}

    std::string MemberEntry::toString() const
    {
        return "MemberEntry(Type: " + Types::dataTypeToString(*memberType) + ", Offset: " + std::to_string(offset) + ")";
    }

    TypeDef::TypeDef() = default;

    TypeDef::TypeDef(int alignment, int size, std::map<std::string, MemberEntry> members, std::vector<std::string> memberOrder)
        : alignment{alignment}, size{size}, members{members}, memberOrder{memberOrder} {}

    std::string TypeDef::toString() const
    {
        std::string membersStr = "{";
        bool first = true;
        for (const auto &name : memberOrder)
        {
            if (!first)
                membersStr += ", ";
            membersStr += "\"" + name + "\": " + members.at(name).toString();
            first = false;
        }
        membersStr += "}";
        return "TypeDef(Alignment: " + std::to_string(alignment) +
               ", Size: " + std::to_string(size) +
               ", Members: " + membersStr + ")";
    }

    TypeEntry::TypeEntry() = default;

    TypeEntry::TypeEntry(AST::Which kind, std::optional<TypeDef> optTypeDef)
        : kind{kind}, optTypeDef{optTypeDef} {}

    std::string TypeEntry::toString() const
    {
        std::string result = "TypeEntry(Kind: " + std::to_string(static_cast<int>(kind));
        if (optTypeDef.has_value())
        {
            result += ", " + optTypeDef->toString();
        }
        else
        {
            result += ", <no TypeDef>";
        }
        result += ")";
        return result;
    }

    void TypeTable::addTypeDefinition(const std::string &tag, const TypeEntry &entry)
    {
        _table[tag] = entry;
    }

    std::map<std::string, TypeTableNS::MemberEntry> TypeTable::getMembers(const std::string &tag) const
    {
        auto it = _table.find(tag);
        if (it == _table.end())
            throw std::runtime_error("Type tag not found: " + tag);
        const auto &optTypeDef = it->second.optTypeDef;
        if (!optTypeDef.has_value())
            throw std::runtime_error("Type does not have members: " + tag);

        return optTypeDef->members;
    }

    std::vector<Types::DataType> TypeTable::getMemberTypes(const std::string &tag) const
    {
        std::vector<Types::DataType> types{};
        auto it = _table.find(tag);
        if (it == _table.end())
            throw std::runtime_error("Type tag not found: " + tag);
        const auto &optTypeDef = it->second.optTypeDef;
        if (!optTypeDef.has_value())
            throw std::runtime_error("Type does not have members: " + tag);

        for (const auto &name : optTypeDef->memberOrder)
        {
            types.push_back(*optTypeDef->members.at(name).memberType);
        }
        return types;
    }

    std::vector<MemberEntry> TypeTable::getFlattenMembers(const std::string &tag) const
    {
        auto it = _table.find(tag);
        if (it == _table.end())
            throw std::runtime_error("Type tag not found: " + tag);
        const auto &optTypeDef = it->second.optTypeDef;
        if (!optTypeDef.has_value())
            throw std::runtime_error("Type does not have members: " + tag);
        std::vector<MemberEntry> members;
        for (const auto &[name, entry] : optTypeDef->members)
            members.push_back(entry);
        std::sort(members.begin(), members.end(), [](const MemberEntry &a, const MemberEntry &b)
                  { return a.offset < b.offset; });
        return members;
    }

    const TypeEntry &TypeTable::find(const std::string &tag) const
    {
        auto it = _table.find(tag);
        if (it != _table.end())
            return it->second;
        throw std::runtime_error("Type not defined: " + tag);
    }

    bool TypeTable::has(const std::string &tag) const
    {
        return _table.find(tag) != _table.end();
    }

    std::optional<TypeEntry> TypeTable::findOpt(const std::string &tag) const
    {
        auto it = _table.find(tag);
        if (it != _table.end())
            return std::make_optional(it->second);
        return std::nullopt;
    }

    int TypeTable::getSize(const std::string &tag) const
    {
        const auto &entry = find(tag);
        if (!entry.optTypeDef.has_value())
            throw std::runtime_error("Incomplete type: " + tag);
        return entry.optTypeDef->size;
    }

    /* Helper function to reconstruct Types.t from a tag */
    Types::DataType TypeTable::getType(const std::string &tag) const
    {
        auto entry = find(tag);
        switch (entry.kind)
        {
        case AST::Which::Struct:
            return Types::makeStructType(tag);
        case AST::Which::Union:
            return Types::makeUnionType(tag);
        default:
            throw std::runtime_error("Unknown type: " + tag);
        }
    }
}