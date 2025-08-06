
#ifndef REGALLOC_H
#define REGALLOC_H

#include <string>
#include <set>
#include <memory>
#include "Assembly.h"
#include "AssemblySymbols.h"

class Settings;
std::shared_ptr<Assembly::Program> allocateRegisters(
    const std::set<std::string> &aliasedPseudos,
    const std::shared_ptr<Assembly::Program> &program,
    AssemblySymbols::AsmSymbolTable &asmSymbol,
    Settings &settings);

#endif
