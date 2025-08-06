#include "AssemblySymbols.h"
#include <stdexcept>

using namespace AssemblySymbols;

Fun::Fun() = default;
Fun::Fun(bool defined, int bytesRequired, bool returnOnStack,
         const std::vector<Assembly::RegName> &paramRegs,
         const std::vector<Assembly::RegName> &returnRegs)
    : defined{defined},
      bytesRequired{bytesRequired},
      returnOnStack{returnOnStack},
      paramRegs{paramRegs},
      returnRegs{returnRegs},
      calleeSavedRegsUsed{} {}

std::string Fun::toString() const
{
    return "Fun { defined: " + std::string(defined ? "true" : "false") +
           ", bytesRequired: " + std::to_string(bytesRequired) +
           ", returnOnStack: " + std::string(returnOnStack ? "true" : "false") +
           ", paramRegs: " + std::to_string(paramRegs.size()) +
           ", returnRegs: " + std::to_string(returnRegs.size()) +
           ", calleeSavedRegsUsed: " + std::to_string(calleeSavedRegsUsed.size()) +
           " }";
}

Obj::Obj() = default;
Obj::Obj(Assembly::AsmType asmType, bool isStatic, bool isConstant)
    : asmType{asmType}, isStatic{isStatic}, isConstant{isConstant} {}

std::string Obj::toString() const
{
    return "Obj { asmType: " + Assembly::asmTypeToString(asmType) +
           ", isStatic: " + std::string(isStatic ? "true" : "false") +
           ", isConstant: " + std::string(isConstant ? "true" : "false") + " }";
}

// AsmSymbolTable methods

void AsmSymbolTable::addFun(const std::string &funName, bool defined, bool returnOnStack,
                            const std::vector<Assembly::RegName> &paramRegs,
                            const std::vector<Assembly::RegName> &returnRegs)
{
    symbols[funName] = Fun(defined, 0, returnOnStack, paramRegs, returnRegs);
}

void AsmSymbolTable::addVar(const std::string &varName, const std::shared_ptr<Assembly::AsmType> &t, bool isStatic)
{
    symbols[varName] = Obj(*t, isStatic, false);
}

void AsmSymbolTable::addConstant(const std::string &constName, const std::shared_ptr<Assembly::AsmType> &t)
{
    symbols[constName] = Obj(*t, true, true);
}

void AsmSymbolTable::setBytesRequired(const std::string &funName, int bytesRequired)
{
    if (auto *fun = std::get_if<Fun>(&symbols[funName]))
        fun->bytesRequired = bytesRequired;
    else
        throw std::runtime_error("Internal error: function is not defined");
}

int AsmSymbolTable::getBytesRequired(const std::string &funName)
{
    if (auto *fun = std::get_if<Fun>(&symbols[funName]))
        return fun->bytesRequired;
    else
        throw std::runtime_error("Internal error: not a function");
}

void AsmSymbolTable::addCalleeSavedRegsUsed(const std::string &funName, const std::set<Assembly::RegName> &regs)
{
    if (auto *fun = std::get_if<Fun>(&symbols[funName]))
        fun->calleeSavedRegsUsed.insert(regs.begin(), regs.end());
    else
        throw std::runtime_error("Internal error: not a function");
}

std::set<Assembly::RegName> AsmSymbolTable::getCalleeSavedRegsUsed(const std::string &funName)
{
    if (auto *fun = std::get_if<Fun>(&symbols[funName]))
        return fun->calleeSavedRegsUsed;
    else
        throw std::runtime_error("Internal error: not a function");
}

int AsmSymbolTable::getSize(const std::string &varName)
{
    if (!exists(varName))
        throw std::runtime_error("Internal error: symbol not found");

    auto entry = get(varName);
    if (auto *obj = std::get_if<Obj>(&entry))
    {
        if (Assembly::isAsmByte(obj->asmType))
            return 1;
        if (Assembly::isAsmLongword(obj->asmType))
            return 4;
        if (Assembly::isAsmQuadword(obj->asmType) || Assembly::isAsmDouble(obj->asmType))
            return 8;
        if (Assembly::isAsmByteArray(obj->asmType))
            return std::get<Assembly::ByteArray>(obj->asmType).size;
    }

    throw std::runtime_error("Internal error: this is a function, not an object");
}

int AsmSymbolTable::getAlignment(const std::string &varName)
{
    if (!exists(varName))
        throw std::runtime_error("Internal error: symbol not found");

    auto entry = get(varName);
    if (auto *obj = std::get_if<Obj>(&entry))
    {
        if (Assembly::isAsmByte(obj->asmType))
            return 1;
        if (Assembly::isAsmLongword(obj->asmType))
            return 4;
        if (Assembly::isAsmQuadword(obj->asmType) || Assembly::isAsmDouble(obj->asmType))
            return 8;
        if (Assembly::isAsmByteArray(obj->asmType))
            return std::get<Assembly::ByteArray>(obj->asmType).alignment;
    }

    throw std::runtime_error("Internal error: this is a function, not an object");
}

bool AsmSymbolTable::isDefined(const std::string &funName)
{
    if (auto *fun = std::get_if<Fun>(&symbols[funName]))
        return fun->defined;
    else
        throw std::runtime_error("Internal error: not a function");
}

bool AsmSymbolTable::isStatic(const std::string &varName) const
{
    if (!exists(varName))
        throw std::runtime_error("Internal error: symbol not found");

    auto entry = get(varName);
    if (auto *obj = std::get_if<Obj>(&entry))
        return obj->isStatic;
    else
        throw std::runtime_error("Internal error: this is a function, not an object");
}

bool AsmSymbolTable::isConstant(const std::string &constName)
{
    if (!exists(constName))
        throw std::runtime_error("Internal error: symbol not found");

    auto entry = get(constName);
    if (auto *obj = std::get_if<Obj>(&entry))
        return obj->isConstant;
    else
        throw std::runtime_error("Internal error: this is a function, not an object");
}

Assembly::AsmType AsmSymbolTable::getType(const std::string &varName)
{
    auto entry = get(varName);

    if (auto *obj = std::get_if<Obj>(&entry))
        return obj->asmType;
    else
        throw std::runtime_error("Internal error: this is a function, not an object");
}

bool AsmSymbolTable::returnsOnStack(const std::string &funName)
{
    auto entry = get(funName);
    if (auto *fun = std::get_if<Fun>(&entry))
        return fun->returnOnStack;
    else
        throw std::runtime_error("Internal error: this is an object, not a function");
}

std::vector<Assembly::RegName> AsmSymbolTable::paramRegsUsed(const std::string &funName)
{
    auto entry = get(funName);
    if (auto *fun = std::get_if<Fun>(&entry))
        return fun->paramRegs;
    else
        throw std::runtime_error("Internal error: not a function");
}

std::vector<Assembly::RegName> AsmSymbolTable::returnRegsUsed(const std::string &funName)
{
    auto entry = get(funName);
    if (auto *fun = std::get_if<Fun>(&entry))
        return fun->returnRegs;
    else
        throw std::runtime_error("Internal error: not a function");
}

bool AsmSymbolTable::exists(const std::string &name) const
{
    return getOpt(name).has_value();
}

std::optional<Entry> AsmSymbolTable::getOpt(const std::string &name) const
{
    auto it = symbols.find(name);
    if (it != symbols.end())
        return it->second;
    return std::nullopt;
}

Entry AsmSymbolTable::get(const std::string &name) const
{
    auto optSymbol = getOpt(name);

    if (!optSymbol.has_value())
        throw std::runtime_error("Symbol not found: " + name);

    return optSymbol.value();
}

const std::unordered_map<std::string, Entry> &AsmSymbolTable::getAllSymbols() const
{
    return symbols;
}