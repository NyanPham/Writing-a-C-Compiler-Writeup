#include "TypeTable.h"
#include "Types.h"
#include <algorithm>
#include <stdexcept>

namespace TypeTableNS
{

    MemberEntry::MemberEntry() = default;

    MemberEntry::MemberEntry(const std::shared_ptr<Types::DataType> &memberType, int offset)
        : memberType{memberType}, offset{offset} {}

    std::string MemberEntry::toString() const
    {
        return "MemberEntry(Type: " + Types::dataTypeToString(*memberType) + ", Offset: " + std::to_string(offset) + ")";
    }

    StructEntry::StructEntry() = default;

    StructEntry::StructEntry(int alignment, int size, std::map<std::string, MemberEntry> members)
        : alignment{alignment}, size{size}, members{members} {}

    std::string StructEntry::toString() const
    {
        std::string membersStr = "{";
        bool first = true;
        for (const auto &[name, entry] : members)
        {
            if (!first)
                membersStr += ", ";
            membersStr += "\"" + name + "\": " + entry.toString();
            first = false;
        }
        membersStr += "}";
        return "StructEntry(Alignment: " + std::to_string(alignment) +
               ", Size: " + std::to_string(size) +
               ", Members: " + membersStr + ")";
    }

    void TypeTable::addStructDefinition(const std::string &tag, const StructEntry &entry)
    {
        _table[tag] = entry;
    }

    std::vector<MemberEntry> TypeTable::getMembers(const std::string &tag) const
    {
        auto it = _table.find(tag);
        if (it == _table.end())
            throw std::runtime_error("Struct tag not found: " + tag);
        std::vector<MemberEntry> members;
        for (const auto &[name, entry] : it->second.members)
            members.push_back(entry);
        std::sort(members.begin(), members.end(), [](const MemberEntry &a, const MemberEntry &b)
                  { return a.offset < b.offset; });
        return members;
    }

    std::vector<Types::DataType> TypeTable::getMemberTypes(const std::string &tag) const
    {
        std::vector<Types::DataType> types;
        auto members = getMembers(tag);
        for (const auto &member : members)
            types.push_back(*member.memberType);
        return types;
    }

    const StructEntry &TypeTable::find(const std::string &tag) const
    {
        auto it = _table.find(tag);
        if (it != _table.end())
            return it->second;
        throw std::runtime_error("Struct not defined: " + tag);
    }

    bool TypeTable::has(const std::string &tag) const
    {
        return _table.find(tag) != _table.end();
    }

}