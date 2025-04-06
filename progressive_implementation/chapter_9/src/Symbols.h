#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <string>
#include <unordered_map>
#include <optional>
#include <stdexcept>
#include "Types.h"

namespace Symbols
{

    enum class SymbolKind
    {
        Variable,
        Function
    };

    struct Symbol
    {
        Types::DataType type;
        SymbolKind kind;
        bool isDefined;
        int stackFrameSize;

        std::string toString() const
        {
            return "Symbol(kind=" + std::string(kind == SymbolKind::Variable ? "Variable" : "Function") +
                   ", type=" + Types::dataTypeToString(type) +
                   ", isDefined=" + (isDefined ? "true" : "false") +
                   ", stackFrameSize=" + std::to_string(stackFrameSize) + ")";
        }
    };

    class SymbolTable
    {
    private:
        std::unordered_map<std::string, Symbol> symbols;

    public:
        void addVariable(const std::string &name, const Types::DataType &type)
        {
            symbols[name] = Symbol{type, SymbolKind::Variable, false, 0};
        }

        void addFunction(const std::string &name, const Types::DataType &type, bool isDefined)
        {
            symbols[name] = Symbol{type, SymbolKind::Function, isDefined, 0};
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
                throw std::runtime_error("Symbol not found: " + name);
            it->second.stackFrameSize = newSize;
        }

        bool exists(const std::string &name) const
        {
            return symbols.find(name) != symbols.end();
        }
    };

}

#endif