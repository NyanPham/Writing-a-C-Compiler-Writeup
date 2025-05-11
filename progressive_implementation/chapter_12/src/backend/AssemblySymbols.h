#ifndef ASSEMBLY_SYMBOLS_H
#define ASSEMBLY_SYMBOLS_H

#include <string>
#include <variant>
#include <unordered_map>

#include "Assembly.h"

namespace AssemblySymbols
{
    struct Fun
    {
        bool defined;
        int bytesRequired;

        Fun() = default;
        Fun(bool defined, int bytesRequired) : defined{defined}, bytesRequired{bytesRequired} {}

        std::string toString() const
        {
            return "Fun { defined: " + std::string(defined ? "true" : "false") +
                   ", bytesRequired: " + std::to_string(bytesRequired) + " }";
        }
    };

    struct Obj
    {
        Assembly::AsmType asmType;
        bool isStatic;

        Obj() = default;
        Obj(Assembly::AsmType asmType, bool isStatic) : asmType{asmType}, isStatic{isStatic} {}

        std::string toString() const
        {
            return "Obj { asmType: " + Assembly::asmTypeToString(asmType) +
                   ", isStatic: " + std::string(isStatic ? "true" : "false") + " }";
        }
    };

    using Entry = std::variant<Fun, Obj>;

    class AsmSymbolTable
    {
    private:
        std::unordered_map<std::string, Entry> symbols;

    public:
        void addFun(const std::string &funName, bool defined)
        {
            symbols[funName] = Fun(defined, 0);
        }

        void addVar(const std::string &varName, const std::shared_ptr<Assembly::AsmType> &t, bool isStatic)
        {
            symbols[varName] = Obj(*t, isStatic);
        }

        void setBytesRequired(const std::string &funName, int bytesRequired)
        {
            if (auto *fun = std::get_if<Fun>(&symbols[funName]))
                // Note: we only set bytesRequired if function is defined in this translation unit
                fun->bytesRequired = bytesRequired;
            else
                throw std::runtime_error("Internal error: function is not defined");
        }

        int getBytesRequired(const std::string &funName)
        {
            if (auto *fun = std::get_if<Fun>(&symbols[funName]))
                return fun->bytesRequired;
            else
                throw std::runtime_error("Internal error: not a function");
        }

        int getSize(const std::string &varName)
        {
            if (!exists(varName))
                throw std::runtime_error("Internal error: symbol not found");

            auto entry = get(varName);
            if (auto *obj = std::get_if<Obj>(&entry))
            {
                if (Assembly::isAsmLongword(obj->asmType))
                    return 4;
                if (Assembly::isAsmQuadword(obj->asmType))
                    return 8;
            }

            throw std::runtime_error("Internal error: this is a function, not an object");
        }

        int getAlignment(const std::string &varName)
        {
            if (!exists(varName))
                throw std::runtime_error("Internal error: symbol not found");

            auto entry = get(varName);
            if (auto *obj = std::get_if<Obj>(&entry))
            {
                if (Assembly::isAsmLongword(obj->asmType))
                    return 4;
                if (Assembly::isAsmQuadword(obj->asmType))
                    return 8;
            }

            throw std::runtime_error("Internal error: this is a function, not an object");
        }

        bool isDefined(const std::string &funName)
        {
            if (auto *fun = std::get_if<Fun>(&symbols[funName]))
                return fun->defined;
            else
                throw std::runtime_error("Internal error: not a function");
        }

        bool isStatic(const std::string &varName)
        {
            if (!exists(varName))
                throw std::runtime_error("Internal error: symbol not found");

            auto entry = get(varName);
            if (auto *obj = std::get_if<Obj>(&entry))
                return obj->isStatic;
            else
                throw std::runtime_error("Internal error: this is a function, not an object");
        }

        bool exists(const std::string &name) const
        {
            return getOpt(name).has_value();
        }

        std::optional<Entry> getOpt(const std::string &name) const
        {
            auto it = symbols.find(name);
            if (it != symbols.end())
                return it->second;
            return std::nullopt;
        }

        Entry get(const std::string &name) const
        {
            auto optSymbol = getOpt(name);

            if (!optSymbol.has_value())
                throw std::runtime_error("Symbol not found: " + name);

            return optSymbol.value();
        }

        const std::unordered_map<std::string, Entry> &getAllSymbols() const
        {
            return symbols;
        }
    };
};

#endif