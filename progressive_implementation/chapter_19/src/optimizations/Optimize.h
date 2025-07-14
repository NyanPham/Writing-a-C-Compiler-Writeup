#pragma once

#include <string>
#include <memory>
#include "../Tacky.h"
#include "../Symbols.h"
#include "../Settings.h"

// Main optimization pipeline for a TACKY program.
std::shared_ptr<TACKY::Program> optimize(
    const Settings &settings,
    const std::string &srcFile,
    const TACKY::Program &tackyProgram,
    const Symbols::SymbolTable &symbolTable);