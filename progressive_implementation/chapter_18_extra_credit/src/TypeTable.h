#ifndef TYPETABLE_H
#define TYPETABLE_H

#include <string>
#include <memory>
#include <map>
#include <vector>

#include "Types.h"
#include "AST.h"

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

    struct TypeDef
    {
        int alignment;
        int size;
        std::map<std::string, MemberEntry> members;

        TypeDef();
        TypeDef(int alignment, int size, std::map<std::string, MemberEntry> members);

        std::string toString() const;
    };

    struct TypeEntry 
    {
        AST::Which kind;
        std::optional<TypeDef> optTypeDef;

        TypeEntry();
        TypeEntry(AST::Which kind, std::optional<TypeDef> optTypeDef);

        std::string toString() const;
    };

    class TypeTable
    {
    public:
        void addTypeDefinition(const std::string &tag, const TypeEntry &entry);
        std::map<std::string, TypeTableNS::MemberEntry> getMembers(const std::string &tag) const;
        std::vector<Types::DataType> getMemberTypes(const std::string &tag) const;
        std::vector<MemberEntry> getFlattenMembers(const std::string &tag) const;
        const TypeEntry &find(const std::string &tag) const;
        bool has(const std::string &tag) const;
        const std::map<std::string, TypeEntry> &getTypeEntries() const { return _table; }
        std::optional<TypeEntry> findOpt(const std::string &tag) const;
        int getSize(const std::string &tag) const;
        Types::DataType getType(const std::string &tag) const;

    private:
        std::map<std::string, TypeEntry> _table;
    };

}

#endif