#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <string>
#include <unordered_map>
#include <optional>
#include <stdexcept>

#include "Types.h"
#include "./utils/VariantHelper.h"

namespace Symbols
{
    /*
    type initial_value = Tentative | Initial(int) | NoInitializer
    */
    struct Tentative
    {
        std::string toString() const
        {
            return "Tentative";
        }
    };

    struct Initial
    {
        int value;

        Initial(int value) : value{value} {}

        std::string toString() const
        {
            return "Initial(" + std::to_string(value) + ")";
        }
    };

    struct NoInitializer
    {
        std::string toString() const
        {
            return "NoInitializer";
        }
    };

    using InitialValue = std::variant<Tentative, Initial, NoInitializer>;

    inline std::string initialValueToString(const InitialValue &initValue)
    {
        return std::visit(
            [](const auto &t)
            {
                return t.toString();
            },
            initValue);
    }

    inline InitialValue makeTentative() { return Tentative{}; }
    inline InitialValue makeInitial(int value) { return Initial{value}; }
    inline InitialValue makeNoInitializer() { return NoInitializer(); }

    inline std::optional<Tentative> getTentative(const InitialValue &initValue)
    {
        return getVariant<Tentative>(initValue);
    }

    inline std::optional<Initial> getInitial(const InitialValue &initValue)
    {
        return getVariant<Initial>(initValue);
    }

    inline std::optional<NoInitializer> getNoInitializer(const InitialValue &initValue)
    {
        return getVariant<NoInitializer>(initValue);
    }

    inline bool isTentative(const InitialValue &initValue)
    {
        return isVariant<Tentative>(initValue);
    }

    inline bool isInitial(const InitialValue &initValue)
    {
        return isVariant<Initial>(initValue);
    }

    inline bool isNoInitializer(const InitialValue &initValue)
    {
        return isVariant<NoInitializer>(initValue);
    }

    /*
    type identifier_attrs =
        | FunAttr(bool defined, bool global, int stack_frame_size)
        | StaticAttr(initial_value init, bool global)
        | LocalAttr
    */

    struct FunAttr
    {
        bool defined;
        bool global;
        int stackFrameSize;

        FunAttr() = default;
        FunAttr(bool defined, bool global, int stackFrameSize)
            : defined{defined}, global{global}, stackFrameSize{stackFrameSize}
        {
        }

        std::string toString() const { return "FunAttr(defined=" + std::to_string(defined) + ", global=" + std::to_string(global) + ", stackFrameSize=" + std::to_string(stackFrameSize) + ")"; }
    };

    struct StaticAttr
    {
        InitialValue init;
        bool global;

        StaticAttr() = default;
        StaticAttr(InitialValue init, bool global)
            : init{init}, global{global}
        {
        }

        std::string toString() const { return "StaticAttr(init=" + initialValueToString(init) + ", global=" + std::to_string(global) + ")"; }
    };

    struct LocalAttr
    {
        LocalAttr() = default;
        std::string toString() const { return "LocalAttr()"; }
    };

    using IdentifierAttrs = std::variant<FunAttr, StaticAttr, LocalAttr>;

    inline std::string identifierAttrsToString(const IdentifierAttrs &attrs)
    {
        return std::visit([](const auto &t)
                          { return t.toString(); }, attrs);
    }

    inline IdentifierAttrs makeFunAttr(bool defined, bool global, int stackFrameSize) { return FunAttr{defined, global, stackFrameSize}; }
    inline IdentifierAttrs makeStaticAttr(InitialValue init, bool global) { return StaticAttr{init, global}; }
    inline IdentifierAttrs makeLocalAttr() { return LocalAttr{}; }

    inline std::optional<FunAttr> getFunAttr(const IdentifierAttrs &attrs)
    {
        return getVariant<FunAttr>(attrs);
    }

    inline std::optional<StaticAttr> getStaticAttr(const IdentifierAttrs &attrs)
    {
        return getVariant<StaticAttr>(attrs);
    }

    inline std::optional<LocalAttr> getLocalAttr(const IdentifierAttrs &attrs)
    {
        return getVariant<LocalAttr>(attrs);
    }

    inline bool isFunAttr(const IdentifierAttrs &attrs)
    {
        return isVariant<FunAttr>(attrs);
    }

    inline bool isStaticAttr(const IdentifierAttrs &attrs)
    {
        return isVariant<StaticAttr>(attrs);
    }

    inline bool isLocalAttr(const IdentifierAttrs &attrs)
    {
        return isVariant<LocalAttr>(attrs);
    }

    /*
    type entry = {
        t: Types.t,
        attrs: identifier_attrs
    }
    I call it Symbol in my implementation to be relevant with the name SymbolTable
    */
    struct Symbol
    {
        Types::DataType type;
        IdentifierAttrs attrs;

        std::string toString() const
        {
            return "Symbol(type=" + Types::dataTypeToString(type) +
                   ", attrs=" + identifierAttrsToString(attrs) + ")";
        }
    };

    /*
    Symbol table and helper functions
    */

    class SymbolTable
    {
    private:
        std::unordered_map<std::string, Symbol> symbols;

    public:
        void addStaticVar(const std::string &name, const Types::DataType &type, const InitialValue &init, bool global)
        {
            symbols[name] = Symbol{
                .type = type,
                .attrs = makeStaticAttr(init, global),
            };
        }

        void addAutomaticVar(const std::string &name, const Types::DataType &type)
        {
            symbols[name] = Symbol{
                .type = type,
                .attrs = makeLocalAttr(),
            };
        }

        void addFunction(const std::string &name, const Types::DataType &type, bool isDefined, bool global)
        {
            symbols[name] = Symbol{
                .type = type,
                .attrs = makeFunAttr(isDefined, global, 0),
            };
        }

        bool isGlobal(const std::string &name)
        {
            auto attrs = get(name).attrs;

            return std::visit([](const auto &attr) -> bool
                              {
                if constexpr (std::is_same_v<std::decay_t<decltype(attr)>, LocalAttr>) {
                    return false;
                } else if constexpr (std::is_same_v<std::decay_t<decltype(attr)>, StaticAttr>) {
                    return true;
                } else if constexpr (std::is_same_v<std::decay_t<decltype(attr)>, FunAttr>) {
                    return true;
                }
                throw std::runtime_error("Unexpected attribute type"); }, attrs);
        }

        bool isStatic(const std::string &name)
        {
            if (!exists(name))
                return false; // Is not in the symbol table, so the name is generated during later stage (TACKY), so it's false

            auto attrs = get(name).attrs;

            if (isLocalAttr(attrs))
                return false;
            else if (isStaticAttr(attrs))
                return true;
            else if (isFunAttr(attrs))
                throw std::runtime_error("Functions don't have storage duration");
            else
                return false;
        }

        std::optional<Symbol> getOpt(const std::string &name) const
        {
            auto it = symbols.find(name);
            if (it != symbols.end())
                return it->second;
            return std::nullopt;
        }

        Symbol get(const std::string &name) const
        {
            auto optSymbol = getOpt(name);

            if (!optSymbol.has_value())
                throw std::runtime_error("Symbol not found: " + name);

            return optSymbol.value();
        }

        void setStackFrameSize(const std::string &name, int newSize)
        {
            auto it = symbols.find(name);
            if (it == symbols.end())
            {
                throw std::runtime_error("Symbol not found: " + name);
            }

            if (auto funAttrs = getFunAttr(it->second.attrs))
            {
                it->second.attrs = makeFunAttr(funAttrs->defined, funAttrs->global, newSize);
            }
            else
            {
                throw std::runtime_error("Internal error: not a function");
            }
        }

        int getStackFrameSize(const std::string &name)
        {
            auto it = symbols.find(name);

            if (it == symbols.end())
                throw std::runtime_error("Symbol not found: " + name);

            if (auto funAttrs = getFunAttr(it->second.attrs))
                return funAttrs->stackFrameSize;
            else
                throw std::runtime_error("Internal error: not a function");
        }

        bool exists(const std::string &name) const
        {
            return getOpt(name).has_value();
        }

        const std::unordered_map<std::string, Symbol> &getAllSymbols() const
        {
            return symbols;
        }
    };
}

#endif