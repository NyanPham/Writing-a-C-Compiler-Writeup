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
        os << "****************************************\n";
        os << "TypeTable Table:\n";
        os << "----------------------------------------\n";

        for (const auto &pair : typeTable.getTypeEntries())
        {
            const std::string &tag = pair.first;
            const TypeTableNS::TypeEntry &entry = pair.second;

            // Print kind (Struct/Union)
            os << (entry.kind == AST::Which::Struct ? "Struct: " : "Union: ") << tag << "\n";

            if (entry.optTypeDef.has_value())
            {
                const auto &typeDef = entry.optTypeDef.value();
                os << "  Alignment: " << typeDef.alignment << "\n";
                os << "  Size: " << typeDef.size << "\n";
                os << "  Members:\n";
                for (const auto &memberPair : typeDef.members)
                {
                    os << "    " << memberPair.first << ": "
                       << memberPair.second.toString() << "\n";
                }
            }
            else
            {
                os << "  <incomplete type>\n";
            }
            os << std::endl;
        }

        os << "****************************************\n";
    }
};

#endif