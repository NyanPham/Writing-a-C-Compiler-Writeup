#ifndef TYPE_TABLE_PRINT_H
#define TYPE_TABLE_PRINT_H

#include <iostream>
#include <string>
#include "../TypeTable.h"
#include "../Types.h"

class TypeTablePrint
{
public:
    // Print the entire type table to the given output stream.
    static void print(const TypeTableNS::TypeTable &typeTable, std::ostream &os = std::cout)
    {
        std::cout << "****************************************\n";
        std::cout << "TypeTable Table:\n";
        std::cout << "----------------------------------------\n";

        for (const auto &pair : typeTable.getStructEntries())
        {
            const std::string &tag = pair.first;
            const TypeTableNS::StructEntry &entry = pair.second;
            os << "Struct: " << tag << "\n";
            os << "  Alignment: " << entry.alignment << "\n";
            os << "  Size: " << entry.size << "\n";
            os << "  Members:\n";
            for (const auto &memberPair : entry.members)
            {
                os << "    " << memberPair.first << ": "
                   << memberPair.second.toString() << "\n";
            }
            os << std::endl;
        }

        std::cout << "****************************************\n";
    }
};

#endif