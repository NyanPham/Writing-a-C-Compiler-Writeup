#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <string>
#include <unordered_map>
#include <optional>
#include <stdexcept>
#include <vector>
#include <memory>

#include "Types.h"
#include "Initializers.h"
#include "UniqueIds.h"
#include "./utils/VariantHelper.h"

namespace Symbols
{
    struct Tentative
    {
        std::string toString() const;
    };

    struct Initial
    {
        std::vector<std::shared_ptr<Initializers::StaticInit>> staticInits;

        Initial(std::vector<std::shared_ptr<Initializers::StaticInit>> staticInits);

        std::string toString() const;
    };

    struct NoInitializer
    {
        std::string toString() const;
    };

    using InitialValue = std::variant<Tentative, Initial, NoInitializer>;

    std::string initialValueToString(const InitialValue &initValue);

    InitialValue makeTentative();
    InitialValue makeInitial(const std::vector<std::shared_ptr<Initializers::StaticInit>> &inits);
    InitialValue makeNoInitializer();

    std::optional<Tentative> getTentative(const InitialValue &initValue);
    std::optional<Initial> getInitial(const InitialValue &initValue);
    std::optional<NoInitializer> getNoInitializer(const InitialValue &initValue);

    bool isTentative(const InitialValue &initValue);
    bool isInitial(const InitialValue &initValue);
    bool isNoInitializer(const InitialValue &initValue);

    struct FunAttr
    {
        bool defined;
        bool global;

        FunAttr();
        FunAttr(bool defined, bool global);

        std::string toString() const;
    };

    struct StaticAttr
    {
        InitialValue init;
        bool global;

        StaticAttr();
        StaticAttr(const InitialValue &init, bool global);

        std::string toString() const;
    };

    struct ConstAttr
    {
        std::shared_ptr<Initializers::StaticInit> init;

        ConstAttr();
        ConstAttr(const std::shared_ptr<Initializers::StaticInit> &init);

        std::string toString() const;
    };

    struct LocalAttr
    {
        LocalAttr();
        std::string toString() const;
    };

    using IdentifierAttrs = std::variant<FunAttr, StaticAttr, ConstAttr, LocalAttr>;

    std::string identifierAttrsToString(const IdentifierAttrs &attrs);

    IdentifierAttrs makeFunAttr(bool defined, bool global);
    IdentifierAttrs makeStaticAttr(const InitialValue &init, bool global);
    IdentifierAttrs makeConstAttr(const std::shared_ptr<Initializers::StaticInit> &init);
    IdentifierAttrs makeLocalAttr();

    std::optional<FunAttr> getFunAttr(const IdentifierAttrs &attrs);
    std::optional<StaticAttr> getStaticAttr(const IdentifierAttrs &attrs);
    std::optional<ConstAttr> getConstAttr(const IdentifierAttrs &attrs);
    std::optional<LocalAttr> getLocalAttr(const IdentifierAttrs &attrs);

    bool isFunAttr(const IdentifierAttrs &attrs);
    bool isStaticAttr(const IdentifierAttrs &attrs);
    bool isConstAttr(const IdentifierAttrs &attrs);
    bool isLocalAttr(const IdentifierAttrs &attrs);

    struct Symbol
    {
        Types::DataType type;
        IdentifierAttrs attrs;

        std::string toString() const;
    };

    class SymbolTable
    {
    private:
        std::unordered_map<std::string, Symbol> symbols;

    public:
        void addStaticVar(const std::string &name, const Types::DataType &type, const InitialValue &init, bool global);
        void addAutomaticVar(const std::string &name, const Types::DataType &type);
        void addFunction(const std::string &name, const Types::DataType &type, bool isDefined, bool global);
        std::string addString(const std::string &str);
        bool isGlobal(const std::string &name);
        std::optional<Symbol> getOpt(const std::string &name) const;
        Symbol get(const std::string &name) const;
        bool exists(const std::string &name) const;
        const std::unordered_map<std::string, Symbol> &getAllSymbols() const;
    };
}

#endif