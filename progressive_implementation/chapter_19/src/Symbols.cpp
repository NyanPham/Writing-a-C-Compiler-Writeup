#include "Symbols.h"

// Member function implementations (constructors, toString, etc.) can remain outside the namespace block if they are already in the Symbols namespace via the class definition.

namespace Symbols
{

    // Tentative
    std::string Tentative::toString() const { return "Tentative"; }

    // Initial
    Initial::Initial(std::vector<std::shared_ptr<Initializers::StaticInit>> staticInits)
        : staticInits{staticInits} {}

    std::string Initial::toString() const
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

    // NoInitializer
    std::string NoInitializer::toString() const { return "NoInitializer"; }

    // InitialValue helpers
    std::string initialValueToString(const InitialValue &initValue)
    {
        return std::visit([](const auto &t)
                          { return t.toString(); }, initValue);
    }

    InitialValue makeTentative() { return Tentative{}; }
    InitialValue makeInitial(const std::vector<std::shared_ptr<Initializers::StaticInit>> &inits) { return Initial{inits}; }
    InitialValue makeNoInitializer() { return NoInitializer(); }

    std::optional<Tentative> getTentative(const InitialValue &initValue) { return getVariant<Tentative>(initValue); }
    std::optional<Initial> getInitial(const InitialValue &initValue) { return getVariant<Initial>(initValue); }
    std::optional<NoInitializer> getNoInitializer(const InitialValue &initValue) { return getVariant<NoInitializer>(initValue); }

    bool isTentative(const InitialValue &initValue) { return isVariant<Tentative>(initValue); }
    bool isInitial(const InitialValue &initValue) { return isVariant<Initial>(initValue); }
    bool isNoInitializer(const InitialValue &initValue) { return isVariant<NoInitializer>(initValue); }

    // FunAttr
    FunAttr::FunAttr() = default;
    FunAttr::FunAttr(bool defined, bool global) : defined{defined}, global{global} {}
    std::string FunAttr::toString() const { return "FunAttr(defined=" + std::to_string(defined) + ", global=" + std::to_string(global) + ")"; }

    // StaticAttr
    StaticAttr::StaticAttr() = default;
    StaticAttr::StaticAttr(const InitialValue &init, bool global) : init{init}, global{global} {}
    std::string StaticAttr::toString() const { return "StaticAttr(init=" + initialValueToString(init) + ", global=" + std::to_string(global) + ")"; }

    // ConstAttr
    ConstAttr::ConstAttr() = default;
    ConstAttr::ConstAttr(const std::shared_ptr<Initializers::StaticInit> &init) : init{init} {}
    std::string ConstAttr::toString() const { return "ConstAttr(init=" + Initializers::toString(*init) + ")"; }

    // LocalAttr
    LocalAttr::LocalAttr() = default;
    std::string LocalAttr::toString() const { return "LocalAttr()"; }

    // IdentifierAttrs helpers
    std::string identifierAttrsToString(const IdentifierAttrs &attrs)
    {
        return std::visit([](const auto &t)
                          { return t.toString(); }, attrs);
    }

    IdentifierAttrs makeFunAttr(bool defined, bool global) { return FunAttr{defined, global}; }
    IdentifierAttrs makeStaticAttr(const InitialValue &init, bool global) { return StaticAttr{init, global}; }
    IdentifierAttrs makeConstAttr(const std::shared_ptr<Initializers::StaticInit> &init) { return ConstAttr{init}; }
    IdentifierAttrs makeLocalAttr() { return LocalAttr{}; }

    std::optional<FunAttr> getFunAttr(const IdentifierAttrs &attrs) { return getVariant<FunAttr>(attrs); }
    std::optional<StaticAttr> getStaticAttr(const IdentifierAttrs &attrs) { return getVariant<StaticAttr>(attrs); }
    std::optional<ConstAttr> getConstAttr(const IdentifierAttrs &attrs) { return getVariant<ConstAttr>(attrs); }
    std::optional<LocalAttr> getLocalAttr(const IdentifierAttrs &attrs) { return getVariant<LocalAttr>(attrs); }

    bool isFunAttr(const IdentifierAttrs &attrs) { return isVariant<FunAttr>(attrs); }
    bool isStaticAttr(const IdentifierAttrs &attrs) { return isVariant<StaticAttr>(attrs); }
    bool isConstAttr(const IdentifierAttrs &attrs) { return isVariant<ConstAttr>(attrs); }
    bool isLocalAttr(const IdentifierAttrs &attrs) { return isVariant<LocalAttr>(attrs); }

    // Symbol
    std::string Symbol::toString() const
    {
        return "Symbol(type=" + Types::dataTypeToString(type) +
               ", attrs=" + identifierAttrsToString(attrs) + ")";
    }

    // SymbolTable
    void SymbolTable::addStaticVar(const std::string &name, const Types::DataType &type, const InitialValue &init, bool global)
    {
        symbols[name] = Symbol{
            .type = type,
            .attrs = makeStaticAttr(init, global),
        };
    }

    void SymbolTable::addAutomaticVar(const std::string &name, const Types::DataType &type)
    {
        symbols[name] = Symbol{
            .type = type,
            .attrs = makeLocalAttr(),
        };
    }

    void SymbolTable::addFunction(const std::string &name, const Types::DataType &type, bool isDefined, bool global)
    {
        symbols[name] = Symbol{
            .type = type,
            .attrs = makeFunAttr(isDefined, global),
        };
    }

    std::string SymbolTable::addString(const std::string &str)
    {
        auto strId = UniqueIds::makeNamedTemporary("string");
        auto type = Types::makeArrayType(
            std::make_shared<Types::DataType>(Types::makeCharType()),
            str.size() + 1);

        symbols[strId] = Symbol{
            .type = type,
            .attrs = makeConstAttr(std::make_shared<Initializers::StaticInit>(Initializers::StringInit(str, true))),
        };

        return strId;
    }

    bool SymbolTable::isGlobal(const std::string &name)
    {
        auto attrs = get(name).attrs;

        return std::visit([](const auto &attr) -> bool
                          {
        if constexpr (std::is_same_v<std::decay_t<decltype(attr)>, LocalAttr> ||
                      std::is_same_v<std::decay_t<decltype(attr)>, ConstAttr>) {
            return false;
        } else if constexpr (std::is_same_v<std::decay_t<decltype(attr)>, StaticAttr>) {
            return attr.global;
        } else if constexpr (std::is_same_v<std::decay_t<decltype(attr)>, FunAttr>) {
            return attr.global;
        }
        throw std::runtime_error("Unexpected attribute type"); }, attrs);
    }

    std::optional<Symbol> SymbolTable::getOpt(const std::string &name) const
    {
        auto it = symbols.find(name);
        if (it != symbols.end())
            return it->second;
        return std::nullopt;
    }

    Symbol SymbolTable::get(const std::string &name) const
    {
        auto optSymbol = getOpt(name);

        if (!optSymbol.has_value())
            throw std::runtime_error("Symbol not found: " + name);

        return optSymbol.value();
    }

    bool SymbolTable::exists(const std::string &name) const
    {
        return getOpt(name).has_value();
    }

    const std::unordered_map<std::string, Symbol> &SymbolTable::getAllSymbols() const
    {
        return symbols;
    }

}