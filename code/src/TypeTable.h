#ifndef TYPETABLE_H
#define TYPETABLE_H

#include <string>
#include <memory>
#include <map>
#include <vector>

#include "Types.h"

namespace TypeTableNS
{

    struct MemberEntry
    {
        std::shared_ptr<Types::DataType> memberType;
        int offset;

        MemberEntry();
        MemberEntry(const std::shared_ptr<Types::DataType> &memberType, int offset);

        std::string toString() const;
    };

    struct StructEntry
    {
        int alignment;
        int size;
        std::map<std::string, MemberEntry> members;

        StructEntry();
        StructEntry(int alignment, int size, std::map<std::string, MemberEntry> members);

        std::string toString() const;
    };

    class TypeTable
    {
    public:
        void addStructDefinition(const std::string &tag, const StructEntry &entry);
        std::vector<MemberEntry> getMembers(const std::string &tag) const;
        std::vector<Types::DataType> getMemberTypes(const std::string &tag) const;
        const StructEntry &find(const std::string &tag) const;
        bool has(const std::string &tag) const;
        const std::map<std::string, StructEntry> &getStructEntries() const { return _table; }

    private:
        std::map<std::string, StructEntry> _table;
    };

}

#endif