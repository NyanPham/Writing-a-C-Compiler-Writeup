#ifndef ASSEMBLY_SYMBOLS_H
#define ASSEMBLY_SYMBOLS_H

#include <string>
#include <variant>
#include <unordered_map>
#include <memory>
#include <optional>

#include "Assembly.h"

namespace AssemblySymbols
{
    struct Fun
    {
        bool defined;
        int bytesRequired;
        bool returnOnStack;

        Fun();
        Fun(bool defined, int bytesRequired, bool returnOnStack);

        std::string toString() const;
    };

    struct Obj
    {
        Assembly::AsmType asmType;
        bool isStatic;
        bool isConstant;

        Obj();
        Obj(Assembly::AsmType asmType, bool isStatic, bool isConstant);

        std::string toString() const;
    };

    using Entry = std::variant<Fun, Obj>;

    class AsmSymbolTable
    {
    private:
        std::unordered_map<std::string, Entry> symbols;

    public:
        void addFun(const std::string &funName, bool defined, bool returnOnStack);
        void addVar(const std::string &varName, const std::shared_ptr<Assembly::AsmType> &t, bool isStatic);
        void addConstant(const std::string &constName, const std::shared_ptr<Assembly::AsmType> &t);
        void setBytesRequired(const std::string &funName, int bytesRequired);
        int getBytesRequired(const std::string &funName);
        int getSize(const std::string &varName);
        int getAlignment(const std::string &varName);
        bool isDefined(const std::string &funName);
        bool isStatic(const std::string &varName);
        bool isConstant(const std::string &constName);
        Assembly::AsmType getType(const std::string &varName);
        bool returnsOnStack(const std::string &funName);
        bool exists(const std::string &name) const;
        std::optional<Entry> getOpt(const std::string &name) const;
        Entry get(const std::string &name) const;
        const std::unordered_map<std::string, Entry> &getAllSymbols() const;
    };
};

#endif