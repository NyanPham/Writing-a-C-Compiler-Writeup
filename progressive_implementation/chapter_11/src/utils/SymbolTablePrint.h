#ifndef SYMBOL_TABLE_PRINT_H
#define SYMBOL_TABLE_PRINT_H

#include <iostream>
#include <string>
#include "../Symbols.h"
#include "../Types.h"

class SymbolTablePrint
{
public:
    static void print(const Symbols::SymbolTable &symbolTable)
    {
        std::cout << "****************************************\n";
        std::cout << "Symbol Table:\n";
        std::cout << "----------------------------------------\n";

        for (const auto &[name, symbol] : symbolTable.getAllSymbols())
        {
            std::cout << "Name: " << name << "\n";
            std::cout << "Type: " << Types::dataTypeToString(symbol.type) << "\n";
            std::cout << "Attributes: " << Symbols::identifierAttrsToString(symbol.attrs) << "\n";
            std::cout << "----------------------------------------\n";
        }
        std::cout << "****************************************\n";
    }
};

#endif