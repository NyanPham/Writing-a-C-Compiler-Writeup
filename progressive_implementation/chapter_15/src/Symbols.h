#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <string>
#include <unordered_map>
#include <optional>
#include <stdexcept>

#include "Types.h"
#include "Initializers.h"
#include "./utils/VariantHelper.h"

namespace Symbols
{
    /*
    type initial_value = Tentative | Initial(static_init*) | NoInitializer
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
        std::vector<std::shared_ptr<Initializers::StaticInit>> staticInits;

        Initial(std::vector<std::shared_ptr<Initializers::StaticInit>> staticInits) : staticInits{staticInits} {}

        std::string toString() const
        {
            std::string result = "Initial([";
            for (size_t i = 0; i < staticInits.size(); ++i)
            {
                result += Initializers::toString(*staticInits[i]);
                if (i + 1 < staticInits.size())
                    result += ", ";
            }
            result += "])";
            return result;
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
    inline InitialValue makeInitial(const std::vector<std::shared_ptr<Initializers::StaticInit>> &inits) { return Initial{inits}; }
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
        | FunAttr(bool defined, bool global)
        | StaticAttr(initial_value init, bool global)
        | LocalAttr
    */

    struct FunAttr
    {
        bool defined;
        bool global;

        FunAttr() = default;
        FunAttr(bool defined, bool global)
            : defined{defined}, global{global}
        {
        }

        std::string toString() const { return "FunAttr(defined=" + std::to_string(defined) + ", global=" + std::to_string(global) + ")"; }
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

    inline IdentifierAttrs makeFunAttr(bool defined, bool global) { return FunAttr{defined, global}; }
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
                .attrs = makeFunAttr(isDefined, global),
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
                    return attr.global;
                } else if constexpr (std::is_same_v<std::decay_t<decltype(attr)>, FunAttr>) {
                    return attr.global;
                }
                throw std::runtime_error("Unexpected attribute type"); }, attrs);
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