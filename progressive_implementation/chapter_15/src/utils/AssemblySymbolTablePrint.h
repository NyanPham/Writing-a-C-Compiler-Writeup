#ifndef ASSEMBLY_SYMBOL_TABLE_PRINT_H
#define ASSEMBLY_SYMBOL_TABLE_PRINT_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>
#include "../backend/AssemblySymbols.h"

class AssemblySymbolTablePrint
{
public:
    static void print(const AssemblySymbols::AsmSymbolTable &symbolTable)
    {
        std::cout << "****************************************\n";
        std::cout << "Assembly Symbol Table:\n";
        std::cout << "----------------------------------------\n";

        const auto &symbols = symbolTable.getAllSymbols();
        for (const auto &[name, entry] : symbols)
        {
            std::cout << "Name: " << name << "\n";

            // Handle the variant type (Fun or Obj)
            std::visit(
                [](const auto &value)
                {
                    using T = std::decay_t<decltype(value)>;
                    if constexpr (std::is_same_v<T, AssemblySymbols::Fun>)
                    {
                        std::cout << "Type: Function\n";
                        std::cout << "Defined: " << (value.defined ? "true" : "false") << "\n";
                        std::cout << "Bytes Required: " << value.bytesRequired << "\n";
                    }
                    else if constexpr (std::is_same_v<T, AssemblySymbols::Obj>)
                    {
                        std::cout << "Type: Object\n";
                        std::cout << "AsmType: " << Assembly::asmTypeToString(value.asmType) << "\n";
                        std::cout << "Static: " << (value.isStatic ? "true" : "false") << "\n";
                    }
                },
                entry);

            std::cout << "----------------------------------------\n";
        }

        std::cout << "****************************************\n";
    }
};

#endif