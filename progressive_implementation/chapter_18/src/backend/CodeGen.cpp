#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <tuple>
#include <functional>

#include "CodeGen.h"
#include "TACKY.h"
#include "Const.h"
#include "Assembly.h"
#include "AssemblySymbols.h"
#include "UniqueIds.h"
#include "TypeTable.h"

std::shared_ptr<Assembly::Imm> zero()
{
    return std::make_shared<Assembly::Imm>(0);
}

/*
    Get the operand type we should use to move an eightbyte of a struct.
    If it contains exactly 8, 4, or 1 bytes, use the corresponding type (note that all but the last
    eightbyte of a struct are exactly 8 bytes). If it's an ueven size s
*/
Assembly::AsmType
CodeGen::getEightbyteType(size_t eightbyteIdx, size_t totalVarSize)
{
    auto bytesLeft = totalVarSize - (eightbyteIdx * 8);
    if (bytesLeft >= 8)
    {
        return Assembly::Quadword();
    }
    else if (bytesLeft == 4)
    {
        return Assembly::Longword();
    }
    else if (bytesLeft == 1)
    {
        return Assembly::Byte();
    }
    else
    {
        return Assembly::ByteArray(bytesLeft, 8); // alignment is dummy as we never use them in this case
    }
}

std::shared_ptr<Assembly::Operand>
CodeGen::addOffset(int n, std::shared_ptr<Assembly::Operand> operand)
{
    if (auto pseudoMem = std::dynamic_pointer_cast<Assembly::PseudoMem>(operand))
        return std::make_shared<Assembly::PseudoMem>(pseudoMem->getBase(), pseudoMem->getOffset() + n);

    if (auto memory = std::dynamic_pointer_cast<Assembly::Memory>(operand))
        return std::make_shared<Assembly::Memory>(memory->getReg(), memory->getOffset() + n);

    // you could do pointer arithmetic w/ indexed or data operands but we don't need to
    throw std::runtime_error("Internal error: trying to copy data to or from non-memory operand");
}

std::vector<std::shared_ptr<Assembly::Instruction>>
CodeGen::copyBytes(std::shared_ptr<Assembly::Operand> srcVal, std::shared_ptr<Assembly::Operand> dstVal, size_t byteCount)
{
    auto insts = std::vector<std::shared_ptr<Assembly::Instruction>>{};

    if (byteCount == 0)
        return insts;

    while (byteCount > 0)
    {
        Assembly::AsmType operandType;
        int operandSize;

        if (byteCount < 4)
        {
            operandType = Assembly::Byte();
            operandSize = 1;
        }
        else if (byteCount < 8)
        {
            operandType = Assembly::Longword();
            operandSize = 4;
        }
        else
        {
            operandType = Assembly::Quadword();
            operandSize = 8;
        }

        insts.push_back(std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(operandType), srcVal, dstVal));
        srcVal = addOffset(operandSize, srcVal);
        dstVal = addOffset(operandSize, dstVal);
        byteCount -= operandSize;
    }

    return insts;
}

/*
    copy an uneven, smaller-than-quadword eightbyte from memory into a register:
    repeatedly copy byte into register and shift left, starting w/ highest byte and working down to lowest
*/
std::vector<std::shared_ptr<Assembly::Instruction>>
CodeGen::copyBytesToReg(std::shared_ptr<Assembly::Operand> srcVal, std::shared_ptr<Assembly::Reg> dstReg, int byteCount)
{
    auto insts = std::vector<std::shared_ptr<Assembly::Instruction>>{};

    for (int i = byteCount - 1; i >= 0; i--)
    {
        auto mv = std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Byte()), addOffset(i, srcVal), dstReg);
        insts.push_back(mv);

        if (i != 0)
            insts.push_back(std::make_shared<Assembly::Binary>(Assembly::BinaryOp::Shl, std::make_shared<Assembly::AsmType>(Assembly::Quadword()), std::make_shared<Assembly::Imm>(8), dstReg));
    }

    return insts;
}

/*
    copy an uneven, smaller-than-quadword eightbyte from a register into memory;
    repeatedly copy byte into register and shift right, starting w/ byte 0  and working up
*/
std::vector<std::shared_ptr<Assembly::Instruction>>
CodeGen::copyBytesFromReg(std::shared_ptr<Assembly::Reg> srcReg, std::shared_ptr<Assembly::Operand> dstVal, int byteCount)
{
    auto insts = std::vector<std::shared_ptr<Assembly::Instruction>>{};

    for (int i = 0; i <= byteCount - 1; i++)
    {
        auto mv = std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Byte()), srcReg, addOffset(i, dstVal));
        insts.push_back(mv);

        if (i < byteCount - 1)
            insts.push_back(std::make_shared<Assembly::Binary>(Assembly::BinaryOp::Shr, std::make_shared<Assembly::AsmType>(Assembly::Quadword()), std::make_shared<Assembly::Imm>(8), srcReg));
    }

    return insts;
}

std::vector<CLS>
CodeGen::classifyNewStructure(const std::string &tag)
{
    auto size = _typeTable.find(tag).size;

    if (size > 16)
    {
        auto eightbyteCount = (size / 8) + (size % 8 == 0 ? 0 : 1);
        // Return a vector of CLS::Mem with size equal to eightbyteCount
        return std::vector<CLS>(eightbyteCount, CLS::Mem);
    }

    std::function<std::vector<Types::DataType>(const Types::DataType &)> processType;
    processType = [&](const Types::DataType &t) -> std::vector<Types::DataType>
    {
        if (auto strctType = Types::getStructType(t))
        {
            auto memberTypes = _typeTable.getMemberTypes(strctType->tag);
            auto processedTypes = std::vector<Types::DataType>{};
            for (auto &membType : memberTypes)
            {
                auto ts = processType(membType);
                processedTypes.insert(processedTypes.end(), ts.begin(), ts.end());
            }
            return processedTypes;
        }
        else if (auto arrType = Types::getArrayType(t))
        {
            auto processedTypes = std::vector<Types::DataType>{};
            for (size_t i = 0; i < arrType->size; ++i)
            {
                auto elemTypes = processType(*arrType->elemType);
                processedTypes.insert(processedTypes.end(), elemTypes.begin(), elemTypes.end());
            }
            return processedTypes;
        }
        else
        {
            return {
                t,
            };
        }
    };

    auto scalarTypes = processType(Types::makeStructType(tag));
    auto first = scalarTypes[0];
    auto last = scalarTypes[scalarTypes.size() - 1];

    if (size > 8)
    {
        auto firstClass = Types::isDoubleType(first) ? CLS::SSE : CLS::INTEGER;
        auto lastClass = Types::isDoubleType(last) ? CLS::SSE : CLS::INTEGER;
        return {firstClass, lastClass};
    }
    else if (Types::isDoubleType(first))
    {
        return {CLS::SSE};
    }
    else
    {
        return {CLS::INTEGER};
    }
}

std::vector<CLS>
CodeGen::classifyStructure(const std::string &tag)
{
    if (_classifiedStructures.find(tag) != _classifiedStructures.end())
    {
        return _classifiedStructures[tag];
    }
    else
    {
        auto classes = classifyNewStructure(tag);
        _classifiedStructures[tag] = classes;
        return classes;
    }
}

std::vector<CLS>
CodeGen::classifyTackyVal(std::shared_ptr<TACKY::Val> val)
{
    if (auto strctType = Types::getStructType(*tackyType(val)))
        return classifyStructure(strctType->tag);
    else
        throw std::runtime_error("Internal error: trying to classify non-structure type");
}

std::tuple<
    std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>>,
    std::vector<std::shared_ptr<Assembly::Operand>>,
    bool>
CodeGen::classifyReturnVal(std::shared_ptr<TACKY::Val> retVal)
{
    auto retvalType = tackyType(retVal);

    if (auto strctType = Types::getStructType(*retvalType))
    {
        auto classes = classifyStructure(strctType->tag);

        std::string varName;
        if (auto var = std::dynamic_pointer_cast<TACKY::Var>(retVal))
            varName = var->getName();
        else
            throw std::runtime_error("Internal error: constant with structure type");

        if (classes[0] == CLS::Mem)
        {
            return std::make_tuple(
                std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>>{},
                std::vector<std::shared_ptr<Assembly::Operand>>{},
                true);
        }
        else
        {
            // return in registers, can move everything w/ quadword operands
            auto intRetvals = std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>>{};
            auto dblRetvals = std::vector<std::shared_ptr<Assembly::Operand>>{};

            for (size_t i = 0; i < classes.size(); ++i)
            {
                auto cls = classes[i];
                auto operand = std::make_shared<Assembly::PseudoMem>(varName, i * 8);

                if (cls == CLS::SSE)
                {
                    dblRetvals.push_back(operand);
                }
                else if (cls == CLS::INTEGER)
                {
                    auto eightbyteType = std::make_shared<Assembly::AsmType>(getEightbyteType(i, Types::getSize(*retvalType, _typeTable)));
                    intRetvals.push_back(std::make_pair(eightbyteType, operand));
                }
                else // is Mem
                {
                    throw std::runtime_error("Internal error: found eightbyte in Mem class unexpectedly");
                }
            }

            return std::make_tuple(
                intRetvals,
                dblRetvals,
                false);
        }
    }
    else if (Types::isDoubleType(*retvalType))
    {
        auto asmVal = convertVal(retVal);
        return std::make_tuple(
            std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>>{},
            std::vector<std::shared_ptr<Assembly::Operand>>{asmVal},
            false);
    }
    else
    {
        auto typedOperand = std::make_pair(
            getAsmType(retVal),
            convertVal(retVal));
        return std::make_tuple(
            std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>>{typedOperand},
            std::vector<std::shared_ptr<Assembly::Operand>>{},
            false);
    }
}

std::vector<std::shared_ptr<Assembly::Instruction>>
CodeGen::convertReturnInstruction(const std::optional<std::shared_ptr<TACKY::Val>> &retVal)
{
    if (!retVal.has_value())
        return {
            std::make_shared<Assembly::Ret>(),
        };

    auto [intRetvals, dblRetvals, returnOnStack] = classifyReturnVal(retVal.value());

    if (returnOnStack)
    {
        auto byteCount = Types::getSize(*tackyType(retVal.value()), _typeTable);
        auto getPtr = std::make_shared<Assembly::Mov>(
            std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
            std::make_shared<Assembly::Memory>(std::make_shared<Assembly::Reg>(Assembly::RegName::BP), -8),
            std::make_shared<Assembly::Reg>(Assembly::RegName::AX));
        auto copyIntoPtr = copyBytes(convertVal(retVal.value()), std::make_shared<Assembly::Memory>(std::make_shared<Assembly::Reg>(Assembly::RegName::AX), 0), byteCount);

        auto insts = std::vector<std::shared_ptr<Assembly::Instruction>>{};
        insts.push_back(getPtr);
        insts.insert(insts.end(), copyIntoPtr.begin(), copyIntoPtr.end());
        insts.push_back(std::make_shared<Assembly::Ret>());

        return insts;
    }
    else
    {
        auto returnInts = std::vector<std::shared_ptr<Assembly::Instruction>>{};
        for (size_t i = 0; i < intRetvals.size(); i++)
        {
            auto [t, op] = intRetvals[i];
            auto regName = i == 0 ? Assembly::RegName::AX : Assembly::RegName::DX;
            auto dstReg = std::make_shared<Assembly::Reg>(regName);
            if (auto byteArrType = Assembly::getByteArray(*t))
            {
                auto copyInsts = copyBytes(op, dstReg, byteArrType->size);
                returnInts.insert(returnInts.end(), copyInsts.begin(), copyInsts.end());
            }
            else
            {
                returnInts.push_back(std::make_shared<Assembly::Mov>(t, op, dstReg));
            }
        }

        auto returnDbls = std::vector<std::shared_ptr<Assembly::Instruction>>{};
        for (size_t i = 0; i < dblRetvals.size(); i++)
        {
            auto op = dblRetvals[i];
            auto regName = i == 0 ? Assembly::RegName::XMM0 : Assembly::RegName::XMM1;
            auto dstReg = std::make_shared<Assembly::Reg>(regName);
            returnDbls.push_back(std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Double()), op, dstReg));
        }

        returnInts.insert(returnInts.end(), returnDbls.begin(), returnDbls.end());
        returnInts.push_back(std::make_shared<Assembly::Ret>());

        return returnInts;
    }
}

int CodeGen::getVarAlignment(const Types::DataType &type)
{
    if (Types::isArrayType(type) && Types::getSize(type, _typeTable) >= 16)
        return 16;
    else
        return Types::getAlignment(type, _typeTable);
}

std::shared_ptr<Assembly::AsmType>
CodeGen::convertVarType(const Types::DataType &type)
{
    if (Types::isArrayType(type))
        return std::make_shared<Assembly::AsmType>(Assembly::ByteArray(Types::getSize(type, _typeTable), Types::getAlignment(type, _typeTable)));
    else
        return convertType(type);
}

// Helper function for double comparisons w/ support for NaN
std::vector<std::shared_ptr<Assembly::Instruction>>
CodeGen::convertDblComparison(TACKY::BinaryOp op, const std::shared_ptr<Assembly::AsmType> &dstType, std::shared_ptr<Assembly::Operand> &asmSrc1, std::shared_ptr<Assembly::Operand> &asmSrc2, const std::shared_ptr<Assembly::Operand> &asmDst)
{
    auto condCode = convertCondCode(op, false);
    /*
        If op is A or AE, can perform usual comparisons;
            these are true if only some flags are 0, so they'll be false for unordered results.
            If op is B or BE, just flip operands and use A or AE instead.
        If op is E or NE, need to check for parity afterwards.
    */
    switch (condCode)
    {
    case Assembly::CondCode::B:
    {
        condCode = Assembly::CondCode::A;
        std::swap(asmSrc1, asmSrc2);
        break;
    }
    case Assembly::CondCode::BE:
    {
        condCode = Assembly::CondCode::AE;
        std::swap(asmSrc1, asmSrc2);
        break;
    }
    default:
        break;
    }

    auto insts = std::vector<std::shared_ptr<Assembly::Instruction>>{
        std::make_shared<Assembly::Cmp>(std::make_shared<Assembly::AsmType>(Assembly::Double()), asmSrc2, asmSrc1),
        std::make_shared<Assembly::Mov>(dstType, zero(), asmDst),
        std::make_shared<Assembly::SetCC>(condCode, asmDst),
    };

    auto parityInsts = std::vector<std::shared_ptr<Assembly::Instruction>>{};
    if (condCode == Assembly::CondCode::E)
    {
        /*
            zero out destination if parity flag is set,
            indicating unordered result
        */
        parityInsts = std::vector<std::shared_ptr<Assembly::Instruction>>{
            std::make_shared<Assembly::Mov>(dstType, zero(), std::make_shared<Assembly::Reg>(Assembly::RegName::R9)),
            std::make_shared<Assembly::SetCC>(Assembly::CondCode::NP, std::make_shared<Assembly::Reg>(Assembly::RegName::R9)),
            std::make_shared<Assembly::Binary>(Assembly::BinaryOp::And, dstType, std::make_shared<Assembly::Reg>(Assembly::RegName::R9), asmDst),
        };
    }
    else if (condCode == Assembly::CondCode::NE)
    {
        // set destination to 1 if parity flag is set, indicating ordered result
        parityInsts = std::vector<std::shared_ptr<Assembly::Instruction>>{
            std::make_shared<Assembly::Mov>(dstType, zero(), std::make_shared<Assembly::Reg>(Assembly::RegName::R9)),
            std::make_shared<Assembly::SetCC>(Assembly::CondCode::P, std::make_shared<Assembly::Reg>(Assembly::RegName::R9)),
            std::make_shared<Assembly::Binary>(Assembly::BinaryOp::Or, dstType, std::make_shared<Assembly::Reg>(Assembly::RegName::R9), asmDst),
        };
    }

    insts.insert(insts.end(), parityInsts.begin(), parityInsts.end());
    return insts;
}

std::tuple<
    std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>>,
    std::vector<std::shared_ptr<Assembly::Operand>>,
    std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>>>
CodeGen::classifyParameters(const std::vector<std::shared_ptr<TACKY::Val>> &tackyVals, bool returnOnStack)
{
    size_t intRegsAvailable = returnOnStack ? INT_PARAM_PASSING_REGS.size() - 1 : INT_PARAM_PASSING_REGS.size();

    std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>> intRegArgs{};
    std::vector<std::shared_ptr<Assembly::Operand>> dblRegArgs{};
    std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>> stackArgs{};

    for (const auto &v : tackyVals)
    {
        auto operand = convertVal(v);
        auto asmType = getAsmType(v);
        auto typedOperand = std::make_pair(asmType, operand);

        if (Assembly::isAsmDouble(*asmType))
        {
            if (dblRegArgs.size() < DBL_PARAM_PASSING_REGS.size())
            {
                dblRegArgs.push_back(operand);
            }
            else
            {
                stackArgs.push_back(typedOperand);
            }
        }
        else if (Assembly::isAsmByte(*asmType) || Assembly::isAsmLongword(*asmType) || Assembly::isAsmQuadword(*asmType))
        {
            if (intRegArgs.size() < intRegsAvailable)
            {
                intRegArgs.push_back(typedOperand);
            }
            else
            {
                stackArgs.push_back(typedOperand);
            }
        }
        else if (auto byteArrType = Assembly::getByteArray(*asmType))
        {
            // it's a structure
            std::string varName;
            if (auto tackyVar = std::dynamic_pointer_cast<TACKY::Var>(v))
                varName = tackyVar->getName();
            else
                throw std::runtime_error("Internal error: constant byte array");

            auto varSize = Types::getSize(tackyType(v), _typeTable);
            auto classes = classifyTackyVal(v);
            bool useStack = true;

            if (classes[0] == CLS::Mem)
            {
                // all eightbytes go on the stack
                useStack = true;
            }
            else
            {
                // tentative assign eigthbytes to registers
                auto tentativeInts = std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>>{};
                auto tentativeDbls = std::vector<std::shared_ptr<Assembly::Operand>>{};

                for (size_t i = 0; i < classes.size(); i++)
                {
                    auto cls = classes[i];
                    auto operand = std::make_shared<Assembly::PseudoMem>(varName, i * 8);

                    if (cls == CLS::SSE)
                    {
                        tentativeDbls.push_back(operand);
                    }
                    else if (cls == CLS::INTEGER)
                    {
                        auto eightByteType = getEightbyteType(i, varSize);
                        tentativeInts.push_back(std::make_pair(asmType, operand));
                    }
                    else // is Mem
                    {
                        throw std::runtime_error("Internal error: found eightbyte in Mem class, but first eightbyte wasn't Mem");
                    }
                }

                if (tentativeInts.size() <= intRegsAvailable && tentativeDbls.size() <= DBL_PARAM_PASSING_REGS.size())
                {
                    intRegArgs = tentativeInts;
                    dblRegArgs = tentativeDbls;
                    useStack = false;
                }
                else
                {
                    useStack = true;
                }
            }

            if (useStack)
            {
                for (size_t i = 0; i < classes.size(); i++)
                {
                    auto eightbyteType = std::make_shared<Assembly::AsmType>(getEightbyteType(i, varSize));
                    stackArgs.push_back(std::make_pair(eightbyteType, std::make_shared<Assembly::PseudoMem>(varName, i * 8)));
                }
            }
        }
    }

    return std::make_tuple(intRegArgs, dblRegArgs, stackArgs);
}

std::string CodeGen::addConstant(double dbl, size_t alignment)
{
    // We if we've defined this double already
    if (_constants.find(dbl) != _constants.end())
    {
        const auto &[name, oldAlignment] = _constants.find(dbl)->second;
        // Update alignment to max of current and new
        _constants.at(dbl).second = std::max(alignment, oldAlignment);
        return name;
    }

    // We haven't defined it yet, add it to the table
    auto name = UniqueIds::makeLabel("dbl");
    _constants.emplace(dbl, std::make_pair(name, alignment));
    return name;
}

std::shared_ptr<Types::DataType> // note: this reports the type of ConstChar as SChar instead of Char, doesn't matter in this context
CodeGen::tackyType(const std::shared_ptr<TACKY::Val> &operand)
{
    if (auto constant = std::dynamic_pointer_cast<TACKY::Constant>(operand))
    {
        return std::make_shared<Types::DataType>(Constants::typeOfConst(*constant->getConst()));
    }
    else if (auto var = std::dynamic_pointer_cast<TACKY::Var>(operand))
    {
        auto entry = _symbolTable.get(var->getName());
        return std::make_shared<Types::DataType>(entry.type);
    }
    else
        throw std::runtime_error("Internal error: invalid operand to get tacky type");
}

std::shared_ptr<Assembly::AsmType>
CodeGen::convertType(const Types::DataType &type)
{
    if (Types::isIntType(type) || Types::isUIntType(type))
        return std::make_shared<Assembly::AsmType>(Assembly::Longword());
    else if (Types::isLongType(type) || Types::isULongType(type) || Types::isPointerType(type))
        return std::make_shared<Assembly::AsmType>(Assembly::Quadword());
    else if (Types::isCharType(type) || Types::isSCharType(type) || Types::isUCharType(type))
        return std::make_shared<Assembly::AsmType>(Assembly::Byte());
    else if (Types::isDoubleType(type))
        return std::make_shared<Assembly::AsmType>(Assembly::Double());
    else if (Types::isArrayType(type) || Types::isStructType(type))
        return std::make_shared<Assembly::AsmType>(Assembly::ByteArray(
            Types::getSize(type, _typeTable),
            Types::getAlignment(type, _typeTable)));
    else
        throw std::runtime_error("Internal error: converting type to assembly");
}

std::shared_ptr<Assembly::AsmType>
CodeGen::getAsmType(const std::shared_ptr<TACKY::Val> &operand)
{
    return convertType(*tackyType(operand));
}

std::vector<std::shared_ptr<Assembly::Instruction>>
CodeGen::passParams(const std::vector<std::shared_ptr<TACKY::Val>> &params, bool returnOnStack)
{
    auto [intRegParams, dblRegParams, stackParams] = classifyParameters(params, returnOnStack);
    std::vector<std::shared_ptr<Assembly::Instruction>> insts{};

    auto remainingIntRegs = INT_PARAM_PASSING_REGS;
    if (returnOnStack)
    {
        // Copy dst ptr
        insts.push_back(std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), std::make_shared<Assembly::Reg>(Assembly::RegName::DI), std::make_shared<Assembly::Memory>(std::make_shared<Assembly::Reg>(Assembly::RegName::BP), -8)));
        remainingIntRegs.erase(remainingIntRegs.begin());
    }

    // pass params in INTEGER regsiters
    for (int i = 0; i < intRegParams.size(); i++)
    {
        auto [paramType, param] = intRegParams[i];
        auto r = INT_PARAM_PASSING_REGS[i];
        if (auto byteArrType = Assembly::getByteArray(*paramType))
        {
            auto copyInsts = copyBytesFromReg(std::make_shared<Assembly::Reg>(r), param, byteArrType->size);
            insts.insert(insts.end(), copyInsts.begin(), copyInsts.end());
        }
        else
        {
            insts.push_back(std::make_shared<Assembly::Mov>(paramType, std::make_shared<Assembly::Reg>(r), param));
        }
    }

    // pass params in DOUBLE regsiters
    for (int i = 0; i < dblRegParams.size(); i++)
    {
        auto param = dblRegParams[i];
        auto r = DBL_PARAM_PASSING_REGS[i];
        insts.push_back(std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Double()), std::make_shared<Assembly::Reg>(r), param));
    }

    // pass params on the stack
    // first param passed on stack has index 0 and is passed at Stack(16)
    for (int i = 0; i < stackParams.size(); i++)
    {
        auto [paramType, param] = stackParams[i];
        auto stack = std::make_shared<Assembly::Memory>(std::make_shared<Assembly::Reg>(Assembly::RegName::BP), 16 + 8 * i);
        if (auto byteArrType = Assembly::getByteArray(*paramType))
        {
            auto copyInsts = copyBytes(stack, param, byteArrType->size);
            insts.insert(insts.end(), copyInsts.begin(), copyInsts.end());
        }
        else
        {
            insts.push_back(std::make_shared<Assembly::Mov>(paramType, stack, param));
        }
    }

    return insts;
}

bool CodeGen::returnsOnStack(const std::string &fnName)
{
    auto type = _symbolTable.get(fnName).type;
    if (auto funType = Types::getFunType(type))
    {
        if (auto strctType = Types::getStructType(*funType->retType))
        {
            auto classes = classifyStructure(strctType->tag);
            return classes[0] == CLS::Mem;
        }
        else
        {
            return false;
        }
    }
    else
    {
        throw std::runtime_error("Internal error: not a function name");
    }
}

std::shared_ptr<Assembly::Operand>
CodeGen::convertVal(const std::shared_ptr<TACKY::Val> &val)
{
    if (auto constant = std::dynamic_pointer_cast<TACKY::Constant>(val))
    {
        if (auto constChar = Constants::getConstChar(*constant->getConst()))
            return std::make_shared<Assembly::Imm>(constChar->val);
        else if (auto constUChar = Constants::getConstUChar(*constant->getConst()))
            return std::make_shared<Assembly::Imm>(constUChar->val);
        else if (auto constInt = Constants::getConstInt(*constant->getConst()))
            return std::make_shared<Assembly::Imm>(constInt->val);
        else if (auto constLong = Constants::getConstLong(*constant->getConst()))
            return std::make_shared<Assembly::Imm>(constLong->val);
        else if (auto constUInt = Constants::getConstUInt(*constant->getConst()))
            return std::make_shared<Assembly::Imm>(constUInt->val);
        else if (auto constULong = Constants::getConstULong(*constant->getConst()))
            return std::make_shared<Assembly::Imm>(constULong->val);
        else if (auto constDouble = Constants::getConstDouble(*constant->getConst()))
            return std::make_shared<Assembly::Data>(addConstant(constDouble->val, 8), 0);
        else
            throw std::runtime_error("Internal error: Invalid constant to convert to assembly");
    }
    else if (auto var = std::dynamic_pointer_cast<TACKY::Var>(val))
    {
        if (Types::isScalar(_symbolTable.get(var->getName()).type))
            return std::make_shared<Assembly::Pseudo>(var->getName());
        return std::make_shared<Assembly::PseudoMem>(var->getName(), 0);
    }
    else
        throw std::runtime_error("Internal error: Invalid value to convert to assembly");
}

Assembly::UnaryOp
CodeGen::convertUnop(const TACKY::UnaryOp op)
{
    switch (op)
    {
    case TACKY::UnaryOp::Complement:
    {
        return Assembly::UnaryOp::Not;
    }
    case TACKY::UnaryOp::Negate:
    {
        return Assembly::UnaryOp::Neg;
    }
    case TACKY::UnaryOp::Not:
        throw std::runtime_error("Internal Error: Cannot convert NOT operator directly from TACKY to Assembly!");
    default:
    {
        throw std::runtime_error("Invalid unary operator");
    }
    }
}

Assembly::BinaryOp
CodeGen::convertBinop(const TACKY::BinaryOp op)
{
    switch (op)
    {
    case TACKY::BinaryOp::Add:
        return Assembly::BinaryOp::Add;
    case TACKY::BinaryOp::Subtract:
        return Assembly::BinaryOp::Sub;
    case TACKY::BinaryOp::Multiply:
        return Assembly::BinaryOp::Mult;
    case TACKY::BinaryOp::Divide:
        return Assembly::BinaryOp::DivDouble; // NB should only be called for operands on double
    case TACKY::BinaryOp::BitwiseAnd:
        return Assembly::BinaryOp::And;
    case TACKY::BinaryOp::BitwiseOr:
        return Assembly::BinaryOp::Or;
    case TACKY::BinaryOp::BitwiseXor:
        return Assembly::BinaryOp::Xor;
    case TACKY::BinaryOp::Remainder:
    case TACKY::BinaryOp::Equal:
    case TACKY::BinaryOp::NotEqual:
    case TACKY::BinaryOp::LessThan:
    case TACKY::BinaryOp::LessOrEqual:
    case TACKY::BinaryOp::GreaterThan:
    case TACKY::BinaryOp::GreaterOrEqual:
    case TACKY::BinaryOp::BitShiftLeft:
    case TACKY::BinaryOp::BitShiftRight:
        throw std::runtime_error("Internal Error: Shouldn't handle like other binary operators!");
    default:
        throw std::runtime_error("Internal Error: Unknown Binary Operators!");
    }
}

Assembly::BinaryOp
CodeGen::convertShiftOp(const TACKY::BinaryOp op, bool isSigned)
{
    /*
        NOTE: Sal/Shl are actually the same operations;
        we use different mnemonics for symmetry with Sar/Shr, which are distinct.
    */
    if (op == TACKY::BinaryOp::BitShiftLeft)
    {
        if (isSigned)
            return Assembly::BinaryOp::Sal;
        else
            return Assembly::BinaryOp::Shl;
    }

    if (op == TACKY::BinaryOp::BitShiftRight)
    {
        if (isSigned)
            return Assembly::BinaryOp::Sar;
        else
            return Assembly::BinaryOp::Shr;
    }

    throw std::runtime_error("Internal error: Not a bitwise shift operation");
}

Assembly::CondCode
CodeGen::convertCondCode(const TACKY::BinaryOp op, bool isSigned)
{
    switch (op)
    {
    case TACKY::BinaryOp::Equal:
        return Assembly::CondCode::E;
    case TACKY::BinaryOp::NotEqual:
        return Assembly::CondCode::NE;
    case TACKY::BinaryOp::LessThan:
        return isSigned ? Assembly::CondCode::L : Assembly::CondCode::B;
    case TACKY::BinaryOp::LessOrEqual:
        return isSigned ? Assembly::CondCode::LE : Assembly::CondCode::BE;
    case TACKY::BinaryOp::GreaterThan:
        return isSigned ? Assembly::CondCode::G : Assembly::CondCode::A;
    case TACKY::BinaryOp::GreaterOrEqual:
        return isSigned ? Assembly::CondCode::GE : Assembly::CondCode::AE;
    default:
        throw std::runtime_error("Internal Error: Unknown binary to cond_code!");
    }
}

std::vector<std::shared_ptr<Assembly::Instruction>>
CodeGen::convertFunCall(const std::shared_ptr<TACKY::FunCall> &fnCall)
{

    auto intRetvals = std::vector<std::pair<std::shared_ptr<Assembly::AsmType>, std::shared_ptr<Assembly::Operand>>>{};
    auto dblRetvals = std::vector<std::shared_ptr<Assembly::Operand>>{};
    bool returnOnStack = false;

    if (fnCall->getOptDst().has_value())
    {
        auto [_intRetvals, _dblRetvals, _retOnStack] = classifyReturnVal(fnCall->getOptDst().value());
        intRetvals = _intRetvals;
        dblRetvals = _dblRetvals;
        returnOnStack = _retOnStack;
    }

    // load address of dest into DI
    size_t firstIntRegIdx = 0;
    std::shared_ptr<Assembly::Instruction> loadDstInst = nullptr;

    if (returnOnStack)
    {
        firstIntRegIdx = 1;
        loadDstInst = std::make_shared<Assembly::Lea>(
            convertVal(fnCall->getOptDst().value()),
            std::make_shared<Assembly::Reg>(Assembly::RegName::DI));
    }

    auto [intRegArgs, dblRegArgs, stackArgs] = classifyParameters(fnCall->getArgs(), returnOnStack);
    std::vector<std::shared_ptr<Assembly::Instruction>> insts{};

    // adjust stack alignment
    int stackPadding = stackArgs.size() % 2 == 0 ? 0 : 8;
    if (stackPadding != 0)
        insts.push_back(
            std::make_shared<Assembly::Binary>(
                Assembly::BinaryOp::Sub,
                std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                std::make_shared<Assembly::Imm>(stackPadding),
                std::make_shared<Assembly::Reg>(Assembly::RegName::SP)));

    // pass arguments in INTEGER registers
    for (size_t i{0}; i < intRegArgs.size(); i++)
    {
        auto [asmType, asmArg] = intRegArgs[i];
        auto r = INT_PARAM_PASSING_REGS[i + firstIntRegIdx];
        if (auto byteArrType = Assembly::getByteArray(*asmType))
        {
            auto copyInsts = copyBytesToReg(asmArg, std::make_shared<Assembly::Reg>(r), byteArrType->size);
            insts.insert(insts.end(), copyInsts.begin(), copyInsts.end());
        }
        else
        {
            insts.push_back(std::make_shared<Assembly::Mov>(asmType, asmArg, std::make_shared<Assembly::Reg>(r)));
        }
    }

    // pass arguments in DOUBLE registers
    for (size_t i{0}; i < dblRegArgs.size(); i++)
    {
        auto r = DBL_PARAM_PASSING_REGS[i];
        insts.push_back(std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Double()), dblRegArgs[i], std::make_shared<Assembly::Reg>(r)));
    }

    // pass arguments on the stack
    std::reverse(stackArgs.begin(), stackArgs.end());
    for (const auto &[asmType, asmArg] : stackArgs)
    {
        if (
            asmArg->getType() == Assembly::NodeType::Reg ||
            asmArg->getType() == Assembly::NodeType::Imm ||
            Assembly::isAsmQuadword(*asmType) ||
            Assembly::isAsmDouble(*asmType))
        {
            insts.push_back(std::make_shared<Assembly::Push>(asmArg));
        }
        else if (auto byteArrayType = Assembly::getByteArray(*asmType))
        {
            insts.push_back(std::make_shared<Assembly::Binary>(
                Assembly::BinaryOp::Sub,
                std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                std::make_shared<Assembly::Imm>(8),
                std::make_shared<Assembly::Reg>(Assembly::RegName::SP)));

            auto copyInsts = copyBytes(
                asmArg,
                std::make_shared<Assembly::Memory>(std::make_shared<Assembly::Reg>(Assembly::RegName::SP), 0),
                byteArrayType->size);
        }
        else
        {
            // Copy into a register before pushing
            insts.push_back(std::make_shared<Assembly::Mov>(asmType, asmArg, std::make_shared<Assembly::Reg>(Assembly::RegName::AX)));
            insts.push_back(std::make_shared<Assembly::Push>(std::make_shared<Assembly::Reg>(Assembly::RegName::AX)));
        }
    }

    // emit call function
    insts.push_back(std::make_shared<Assembly::Call>(fnCall->getFnName()));

    // adjust stack pointer
    auto bytesToRemove = 8 * (stackArgs.size()) + stackPadding;
    if (bytesToRemove != 0)
        insts.push_back(
            std::make_shared<Assembly::Binary>(
                Assembly::BinaryOp::Add,
                std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                std::make_shared<Assembly::Imm>(bytesToRemove),
                std::make_shared<Assembly::Reg>(Assembly::RegName::SP)));

    // retrieve return value
    if (fnCall->getOptDst().has_value() && !returnOnStack)
    {
        auto intRetRegs = std::vector<Assembly::RegName>{Assembly::RegName::AX, Assembly::RegName::DX};
        auto dblRetRegs = std::vector<Assembly::RegName>{Assembly::RegName::XMM0, Assembly::RegName::XMM1};

        for (size_t i = 0; i < intRetvals.size(); i++)
        {
            auto r = intRetRegs[i];
            auto [t, op] = intRetvals[i];

            if (auto byteArrType = Assembly::getByteArray(*t))
            {
                auto copyInsts = copyBytesFromReg(std::make_shared<Assembly::Reg>(r), op, byteArrType->size);
                insts.insert(insts.end(), copyInsts.begin(), copyInsts.end());
            }
            else
            {
                insts.push_back(std::make_shared<Assembly::Mov>(t, std::make_shared<Assembly::Reg>(r), op));
            }
        }

        for (size_t i = 0; i < dblRetvals.size(); i++)
        {
            auto r = dblRetRegs[i];
            auto op = dblRetvals[i];
            insts.push_back(std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Double()), std::make_shared<Assembly::Reg>(r), op));
        }
    }

    return insts;
}

std::vector<std::shared_ptr<Assembly::Instruction>>
CodeGen::convertInstruction(const std::shared_ptr<TACKY::Instruction> &inst)
{
    switch (inst->getType())
    {
    case TACKY::NodeType::Copy:
    {
        auto copyInst = std::dynamic_pointer_cast<TACKY::Copy>(inst);

        if (Types::isScalar(*tackyType(copyInst->getSrc())))
        {
            auto asmType = getAsmType(copyInst->getSrc());
            auto asmSrc = convertVal(copyInst->getSrc());
            auto asmDst = convertVal(copyInst->getDst());

            return {
                std::make_shared<Assembly::Mov>(asmType, asmSrc, asmDst),
            };
        }
        else
        {
            auto asmSrc = convertVal(copyInst->getSrc());
            auto asmDst = convertVal(copyInst->getDst());
            auto byteCount = Types::getSize(*tackyType(copyInst->getSrc()), _typeTable);

            return copyBytes(asmSrc, asmDst, byteCount);
        }
    }
    case TACKY::NodeType::Return:
    {
        auto returnInst = std::dynamic_pointer_cast<TACKY::Return>(inst);
        return convertReturnInstruction(returnInst->getOptValue());
    }
    case TACKY::NodeType::Unary:
    {
        auto unaryInst = std::dynamic_pointer_cast<TACKY::Unary>(inst);

        if (unaryInst->getOp() == TACKY::UnaryOp::Not)
        {
            auto srcType = getAsmType(unaryInst->getSrc());
            auto dstType = getAsmType(unaryInst->getDst());
            auto asmSrc = convertVal(unaryInst->getSrc());
            auto asmDst = convertVal(unaryInst->getDst());

            if (Assembly::isAsmDouble(*srcType))
            {
                return {
                    std::make_shared<Assembly::Binary>(
                        Assembly::BinaryOp::Xor,
                        std::make_shared<Assembly::AsmType>(Assembly::Double()),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0)),
                    std::make_shared<Assembly::Cmp>(srcType, asmSrc, std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0)),
                    std::make_shared<Assembly::Mov>(dstType, zero(), asmDst),
                    std::make_shared<Assembly::SetCC>(Assembly::CondCode::E, asmDst),

                    // cmp with NaN sets both ZF and PF, but !NaN should evaluate to 0,
                    // so we'll calculate:
                    // !x = ZF && !PF

                    std::make_shared<Assembly::SetCC>(Assembly::CondCode::NP, std::make_shared<Assembly::Reg>(Assembly::RegName::R9)),
                    std::make_shared<Assembly::Binary>(Assembly::BinaryOp::And, dstType, std::make_shared<Assembly::Reg>(Assembly::RegName::R9), asmDst),
                };
            }
            else
            {
                return {
                    std::make_shared<Assembly::Cmp>(srcType, zero(), asmSrc),
                    std::make_shared<Assembly::Mov>(dstType, zero(), asmDst),
                    std::make_shared<Assembly::SetCC>(Assembly::CondCode::E, asmDst),
                };
            }
        }
        else if (unaryInst->getOp() == TACKY::UnaryOp::Negate && Types::isDoubleType(*tackyType(unaryInst->getSrc())))
        {
            auto asmSrc = convertVal(unaryInst->getSrc());
            auto asmDst = convertVal(unaryInst->getDst());
            auto negativeZero = addConstant(-0.0, 16);

            return {
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Double()), asmSrc, asmDst),
                std::make_shared<Assembly::Binary>(
                    Assembly::BinaryOp::Xor,
                    std::make_shared<Assembly::AsmType>(Assembly::Double()),
                    std::make_shared<Assembly::Data>(negativeZero, 0),
                    asmDst),
            };
        }
        else
        {
            auto asmType = getAsmType(unaryInst->getDst());
            auto asmOp = convertUnop(unaryInst->getOp());
            auto asmSrc = convertVal(unaryInst->getSrc());
            auto asmDst = convertVal(unaryInst->getDst());

            return {
                std::make_shared<Assembly::Mov>(asmType, asmSrc, asmDst),
                std::make_shared<Assembly::Unary>(asmOp, asmType, asmDst),
            };
        }
    }
    case TACKY::NodeType::Binary:
    {
        auto binaryInst = std::dynamic_pointer_cast<TACKY::Binary>(inst);

        auto srcType = getAsmType(binaryInst->getSrc1());
        auto dstType = getAsmType(binaryInst->getDst());
        auto asmSrc1 = convertVal(binaryInst->getSrc1());
        auto asmSrc2 = convertVal(binaryInst->getSrc2());
        auto asmDst = convertVal(binaryInst->getDst());

        switch (binaryInst->getOp())
        {
        // Relational Operators
        case TACKY::BinaryOp::Equal:
        case TACKY::BinaryOp::NotEqual:
        case TACKY::BinaryOp::LessThan:
        case TACKY::BinaryOp::LessOrEqual:
        case TACKY::BinaryOp::GreaterThan:
        case TACKY::BinaryOp::GreaterOrEqual:
        {
            if (Assembly::isAsmDouble(*srcType))
            {
                return convertDblComparison(binaryInst->getOp(), dstType, asmSrc1, asmSrc2, asmDst);
            }
            auto isSigned = Assembly::isAsmDouble(*srcType)
                                ? false
                                : Types::isSigned(*tackyType(binaryInst->getSrc1()));
            auto condCode = convertCondCode(binaryInst->getOp(), isSigned);

            return {
                std::make_shared<Assembly::Cmp>(srcType, asmSrc1, asmSrc2),
                std::make_shared<Assembly::Mov>(dstType, zero(), asmDst),
                std::make_shared<Assembly::SetCC>(condCode, asmDst),
            };
        }

        // For Division/Modulo
        case TACKY::BinaryOp::Divide:
        case TACKY::BinaryOp::Remainder:
        {
            if (!Assembly::isAsmDouble(*srcType))
            {
                Assembly::RegName resultRegName =
                    binaryInst->getOp() == TACKY::BinaryOp::Divide
                        ? Assembly::RegName::AX
                        : Assembly::RegName::DX;

                if (Types::isSigned(*tackyType(binaryInst->getSrc1())))
                {
                    return {
                        std::make_shared<Assembly::Mov>(srcType, asmSrc1, std::make_shared<Assembly::Reg>(Assembly::RegName::AX)),
                        std::make_shared<Assembly::Cdq>(srcType),
                        std::make_shared<Assembly::Idiv>(srcType, asmSrc2),
                        std::make_shared<Assembly::Mov>(srcType, std::make_shared<Assembly::Reg>(resultRegName), asmDst),
                    };
                }
                else
                {
                    return {
                        std::make_shared<Assembly::Mov>(srcType, asmSrc1, std::make_shared<Assembly::Reg>(Assembly::RegName::AX)),
                        std::make_shared<Assembly::Mov>(srcType, zero(), std::make_shared<Assembly::Reg>(Assembly::RegName::DX)),
                        std::make_shared<Assembly::Div>(srcType, asmSrc2),
                        std::make_shared<Assembly::Mov>(srcType, std::make_shared<Assembly::Reg>(resultRegName), asmDst),
                    };
                }
            }

            auto asmOp = convertBinop(binaryInst->getOp());

            return {
                std::make_shared<Assembly::Mov>(srcType, asmSrc1, asmDst),
                std::make_shared<Assembly::Binary>(asmOp, srcType, asmSrc2, asmDst),
            };
        }

        // For Bit Shift instructions, source 2 can only be either in CX register or an Imm
        case TACKY::BinaryOp::BitShiftLeft:
        case TACKY::BinaryOp::BitShiftRight:
        {
            auto isSigned = Types::isSigned(*tackyType(binaryInst->getSrc1()));
            auto asmOp = convertShiftOp(binaryInst->getOp(), isSigned);
            auto asmType = getAsmType(binaryInst->getSrc1());

            if (asmSrc2->getType() == Assembly::NodeType::Imm)
            {
                return {
                    std::make_shared<Assembly::Mov>(asmType, asmSrc1, asmDst),
                    std::make_shared<Assembly::Binary>(asmOp, asmType, asmSrc2, asmDst),
                };
            }
            else
            {
                // NOTE: only lower byte of CX is used.
                auto RegCX = std::make_shared<Assembly::Reg>(Assembly::RegName::CX);

                return {
                    std::make_shared<Assembly::Mov>(asmType, asmSrc1, asmDst),
                    std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Byte()), asmSrc2, RegCX),
                    std::make_shared<Assembly::Binary>(asmOp, asmType, RegCX, asmDst),
                };
            }
        }

        // Addition/Subtraction/Multiplication
        default:
        {
            auto asmOp = convertBinop(binaryInst->getOp());

            return {
                std::make_shared<Assembly::Mov>(srcType, asmSrc1, asmDst),
                std::make_shared<Assembly::Binary>(asmOp, srcType, asmSrc2, asmDst),
            };
        }
        }
    }
    case TACKY::NodeType::Load:
    {
        auto load = std::dynamic_pointer_cast<TACKY::Load>(inst);

        if (Types::isScalar(*tackyType(load->getDst())))
        {
            auto asmSrcPtr = convertVal(load->getSrcPtr());
            auto asmDst = convertVal(load->getDst());
            auto asmType = getAsmType(load->getDst());

            return {
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), asmSrcPtr, std::make_shared<Assembly::Reg>(Assembly::RegName::R9)),
                std::make_shared<Assembly::Mov>(asmType, std::make_shared<Assembly::Memory>(std::make_shared<Assembly::Reg>(Assembly::RegName::R9), 0), asmDst),
            };
        }
        else
        {
            auto asmSrcPtr = convertVal(load->getSrcPtr());
            auto asmDst = convertVal(load->getDst());
            auto byteCount = Types::getSize(*tackyType(load->getDst()), _typeTable);

            auto insts = std::vector<std::shared_ptr<Assembly::Instruction>>();
            insts.push_back(std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), asmSrcPtr, std::make_shared<Assembly::Reg>(Assembly::RegName::R9)));
            auto copyInsts = copyBytes(std::make_shared<Assembly::Memory>(std::make_shared<Assembly::Reg>(Assembly::RegName::R9), 0), asmDst, byteCount);
            insts.insert(insts.end(), copyInsts.begin(), copyInsts.end());
            return insts;
        }
    }
    case TACKY::NodeType::Store:
    {
        auto store = std::dynamic_pointer_cast<TACKY::Store>(inst);
        if (Types::isScalar(*tackyType(store->getSrc())))
        {
            auto asmSrc = convertVal(store->getSrc());
            auto asmType = getAsmType(store->getSrc());
            auto asmDstPtr = convertVal(store->getDstPtr());

            return {
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), asmDstPtr, std::make_shared<Assembly::Reg>(Assembly::RegName::R9)),
                std::make_shared<Assembly::Mov>(asmType, asmSrc, std::make_shared<Assembly::Memory>(std::make_shared<Assembly::Reg>(Assembly::RegName::R9), 0)),
            };
        }
        else
        {
            auto asmSrc = convertVal(store->getSrc());
            auto asmDstPtr = convertVal(store->getDstPtr());
            auto byteCount = Types::getSize(*tackyType(store->getSrc()), _typeTable);
            auto insts = std::vector<std::shared_ptr<Assembly::Instruction>>();
            insts.push_back(std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), asmDstPtr, std::make_shared<Assembly::Reg>(Assembly::RegName::R9)));
            auto copyInsts = copyBytes(asmSrc, std::make_shared<Assembly::Memory>(std::make_shared<Assembly::Reg>(Assembly::RegName::R9), 0), byteCount);
            insts.insert(insts.end(), copyInsts.begin(), copyInsts.end());
            return insts;
        }
    }
    case TACKY::NodeType::GetAddress:
    {
        auto getAddress = std::dynamic_pointer_cast<TACKY::GetAddress>(inst);
        auto asmSrc = convertVal(getAddress->getSrc());
        auto asmDst = convertVal(getAddress->getDst());

        return {
            std::make_shared<Assembly::Lea>(asmSrc, asmDst),
        };
    }
    case TACKY::NodeType::Jump:
    {
        return {
            std::make_shared<Assembly::Jmp>(std::dynamic_pointer_cast<TACKY::Jump>(inst)->getTarget()),
        };
    }
    case TACKY::NodeType::JumpIfZero:
    {
        auto jumpIfZeroInst = std::dynamic_pointer_cast<TACKY::JumpIfZero>(inst);

        auto asmType = getAsmType(jumpIfZeroInst->getCond());
        auto asmCond = convertVal(jumpIfZeroInst->getCond());

        if (Assembly::isAsmDouble(*asmType))
        {
            auto compareToZero = std::vector<std::shared_ptr<Assembly::Instruction>>{
                std::make_shared<Assembly::Binary>(
                    Assembly::BinaryOp::Xor,
                    std::make_shared<Assembly::AsmType>(Assembly::Double()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0)),
                std::make_shared<Assembly::Cmp>(asmType, asmCond, std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0)),
            };

            auto lbl = UniqueIds::makeLabel("nan.jmp.end");
            auto conditionalJmp = std::vector<std::shared_ptr<Assembly::Instruction>>{
                // Comparison to NaN sets ZF and PF flags;
                // to treat NaN as nonzero, skip over je instruction if PF flag is set
                std::make_shared<Assembly::JmpCC>(Assembly::CondCode::P, lbl),
                std::make_shared<Assembly::JmpCC>(Assembly::CondCode::E, jumpIfZeroInst->getTarget()),
                std::make_shared<Assembly::Label>(lbl),
            };

            compareToZero.insert(compareToZero.end(), conditionalJmp.begin(), conditionalJmp.end());
            return compareToZero;
        }

        return {
            std::make_shared<Assembly::Cmp>(asmType, std::make_shared<Assembly::Imm>(0), asmCond),
            std::make_shared<Assembly::JmpCC>(Assembly::CondCode::E, jumpIfZeroInst->getTarget()),
        };
    }
    case TACKY::NodeType::JumpIfNotZero:
    {
        auto jumpIfNotZeroInst = std::dynamic_pointer_cast<TACKY::JumpIfNotZero>(inst);

        auto asmType = getAsmType(jumpIfNotZeroInst->getCond());
        auto asmCond = convertVal(jumpIfNotZeroInst->getCond());

        if (Assembly::isAsmDouble(*asmType))
        {
            return {
                std::make_shared<Assembly::Binary>(
                    Assembly::BinaryOp::Xor,
                    std::make_shared<Assembly::AsmType>(Assembly::Double()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0)),
                std::make_shared<Assembly::Cmp>(asmType, asmCond, std::make_shared<Assembly::Reg>(Assembly::RegName::XMM0)),
                std::make_shared<Assembly::JmpCC>(Assembly::CondCode::NE, jumpIfNotZeroInst->getTarget()),

                // Also jumpt to target on Nan, which is nonzero
                std::make_shared<Assembly::JmpCC>(Assembly::CondCode::P, jumpIfNotZeroInst->getTarget()),
            };
        }

        return {
            std::make_shared<Assembly::Cmp>(asmType, std::make_shared<Assembly::Imm>(0), asmCond),
            std::make_shared<Assembly::JmpCC>(Assembly::CondCode::NE, jumpIfNotZeroInst->getTarget()),
        };
    }
    case TACKY::NodeType::Label:
    {
        return {
            std::make_shared<Assembly::Label>(std::dynamic_pointer_cast<TACKY::Label>(inst)->getName()),
        };
    }
    case TACKY::NodeType::FunCall:
    {
        return convertFunCall(std::dynamic_pointer_cast<TACKY::FunCall>(inst));
    }
    case TACKY::NodeType::SignExtend:
    {
        auto signExtend = std::dynamic_pointer_cast<TACKY::SignExtend>(inst);

        auto asmSrc = convertVal(signExtend->getSrc());
        auto asmDst = convertVal(signExtend->getDst());

        return {
            std::make_shared<Assembly::Movsx>(getAsmType(signExtend->getSrc()), getAsmType(signExtend->getDst()), asmSrc, asmDst),
        };
    }
    case TACKY::NodeType::Truncate:
    {
        auto truncate = std::dynamic_pointer_cast<TACKY::Truncate>(inst);

        auto asmSrc = convertVal(truncate->getSrc());
        auto asmDst = convertVal(truncate->getDst());

        return {
            std::make_shared<Assembly::Mov>(getAsmType(truncate->getDst()), asmSrc, asmDst),
        };
    }
    case TACKY::NodeType::ZeroExtend:
    {
        auto zeroExt = std::dynamic_pointer_cast<TACKY::ZeroExtend>(inst);

        auto asmSrc = convertVal(zeroExt->getSrc());
        auto asmDst = convertVal(zeroExt->getDst());

        return {
            std::make_shared<Assembly::MovZeroExtend>(getAsmType(zeroExt->getSrc()), getAsmType(zeroExt->getDst()), asmSrc, asmDst),
        };
    }
    case TACKY::NodeType::IntToDouble:
    {
        auto int2Dbl = std::dynamic_pointer_cast<TACKY::IntToDouble>(inst);

        auto asmSrc = convertVal(int2Dbl->getSrc());
        auto asmDst = convertVal(int2Dbl->getDst());
        auto t = getAsmType(int2Dbl->getSrc());

        if (Assembly::isAsmByte(*t))
        {
            auto byteType = std::make_shared<Assembly::AsmType>(Assembly::Byte());
            auto longwordType = std::make_shared<Assembly::AsmType>(Assembly::Longword());
            auto r9 = std::make_shared<Assembly::Reg>(Assembly::RegName::R9);

            return {
                std::make_shared<Assembly::Movsx>(byteType, longwordType, asmSrc, r9),
                std::make_shared<Assembly::Cvtsi2sd>(longwordType, r9, asmDst),
            };
        }

        return {
            std::make_shared<Assembly::Cvtsi2sd>(t, asmSrc, asmDst),
        };
    }
    case TACKY::NodeType::DoubleToInt:
    {
        auto cvt = std::dynamic_pointer_cast<TACKY::DoubleToInt>(inst);

        auto asmSrc = convertVal(cvt->getSrc());
        auto asmDst = convertVal(cvt->getDst());
        auto t = getAsmType(cvt->getDst());

        if (Assembly::isAsmByte(*t))
        {
            auto byteType = std::make_shared<Assembly::AsmType>(Assembly::Byte());
            auto longwordType = std::make_shared<Assembly::AsmType>(Assembly::Longword());
            auto r9 = std::make_shared<Assembly::Reg>(Assembly::RegName::R9);

            return {
                std::make_shared<Assembly::Cvttsd2si>(longwordType, asmSrc, r9),
                std::make_shared<Assembly::Mov>(byteType, r9, asmDst),
            };
        }

        return {
            std::make_shared<Assembly::Cvttsd2si>(t, asmSrc, asmDst),
        };
    }
    case TACKY::NodeType::UIntToDouble:
    {
        auto cvt = std::dynamic_pointer_cast<TACKY::UIntToDouble>(inst);

        auto asmSrc = convertVal(cvt->getSrc());
        auto asmDst = convertVal(cvt->getDst());

        if (Types::isUCharType(*tackyType(cvt->getSrc())))
        {
            auto byteType = std::make_shared<Assembly::AsmType>(Assembly::Byte());
            auto longwordType = std::make_shared<Assembly::AsmType>(Assembly::Longword());
            auto r9 = std::make_shared<Assembly::Reg>(Assembly::RegName::R9);

            return {
                std::make_shared<Assembly::MovZeroExtend>(byteType, longwordType, asmSrc, r9),
                std::make_shared<Assembly::Cvtsi2sd>(longwordType, r9, asmDst),
            };
        }
        else if (Types::isUIntType(*tackyType(cvt->getSrc())))
        {
            auto longwordType = std::make_shared<Assembly::AsmType>(Assembly::Longword());
            auto quadwordType = std::make_shared<Assembly::AsmType>(Assembly::Quadword());
            auto r9 = std::make_shared<Assembly::Reg>(Assembly::RegName::R9);

            return {
                std::make_shared<Assembly::MovZeroExtend>(longwordType, quadwordType, asmSrc, r9),
                std::make_shared<Assembly::Cvtsi2sd>(quadwordType, r9, asmDst),
            };
        }
        else
        {
            auto outOfBound = UniqueIds::makeLabel("ulong2dbl.oob");
            auto endLbl = UniqueIds::makeLabel("ulong2dbl.end");
            auto r1 = std::make_shared<Assembly::Reg>(Assembly::RegName::R8);
            auto r2 = std::make_shared<Assembly::Reg>(Assembly::RegName::R9);

            return {
                std::make_shared<Assembly::Cmp>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), zero(), asmSrc),
                std::make_shared<Assembly::JmpCC>(Assembly::CondCode::L, outOfBound),
                std::make_shared<Assembly::Cvtsi2sd>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), asmSrc, asmDst),
                std::make_shared<Assembly::Jmp>(endLbl),
                std::make_shared<Assembly::Label>(outOfBound),
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), asmSrc, r1),
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), r1, r2),
                std::make_shared<Assembly::Unary>(Assembly::UnaryOp::ShrOneOp, std::make_shared<Assembly::AsmType>(Assembly::Quadword()), r2),
                std::make_shared<Assembly::Binary>(Assembly::BinaryOp::And, std::make_shared<Assembly::AsmType>(Assembly::Quadword()), std::make_shared<Assembly::Imm>(1), r1),
                std::make_shared<Assembly::Binary>(Assembly::BinaryOp::Or, std::make_shared<Assembly::AsmType>(Assembly::Quadword()), r1, r2),
                std::make_shared<Assembly::Cvtsi2sd>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), r2, asmDst),
                std::make_shared<Assembly::Binary>(Assembly::BinaryOp::Add, std::make_shared<Assembly::AsmType>(Assembly::Double()), asmDst, asmDst),
                std::make_shared<Assembly::Label>(endLbl),
            };
        }
    }
    case TACKY::NodeType::DoubleToUInt:
    {
        auto cvt = std::dynamic_pointer_cast<TACKY::DoubleToUInt>(inst);

        auto asmSrc = convertVal(cvt->getSrc());
        auto asmDst = convertVal(cvt->getDst());

        if (Types::isUCharType(*tackyType(cvt->getDst())))
        {
            auto longwordType = std::make_shared<Assembly::AsmType>(Assembly::Longword());
            auto byteType = std::make_shared<Assembly::AsmType>(Assembly::Byte());
            auto r9 = std::make_shared<Assembly::Reg>(Assembly::RegName::R9);

            return {
                std::make_shared<Assembly::Cvttsd2si>(longwordType, asmSrc, r9),
                std::make_shared<Assembly::Mov>(byteType, r9, asmDst),
            };
        }
        else if (Types::isUIntType(*tackyType(cvt->getDst())))
        {
            auto quadwordType = std::make_shared<Assembly::AsmType>(Assembly::Quadword());
            auto longwordType = std::make_shared<Assembly::AsmType>(Assembly::Longword());
            auto r9 = std::make_shared<Assembly::Reg>(Assembly::RegName::R9);

            return {
                std::make_shared<Assembly::Cvttsd2si>(quadwordType, asmSrc, r9),
                std::make_shared<Assembly::Mov>(longwordType, r9, asmDst),
            };
        }
        else
        {
            auto outOfBound = UniqueIds::makeLabel("dbl2ulong.oob");
            auto endLbl = UniqueIds::makeLabel("dbl2ulong.end");
            auto upperBound = addConstant(9223372036854775808.0, 8);
            auto upperBoundAsInt = std::make_shared<Assembly::Imm>(std::numeric_limits<int64_t>::min());
            auto r = std::make_shared<Assembly::Reg>(Assembly::RegName::R9);
            auto x = std::make_shared<Assembly::Reg>(Assembly::RegName::XMM7);

            return {
                std::make_shared<Assembly::Cmp>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), std::make_shared<Assembly::Data>(upperBound, 0), asmSrc),
                std::make_shared<Assembly::JmpCC>(Assembly::CondCode::AE, outOfBound),
                std::make_shared<Assembly::Cvttsd2si>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), asmSrc, asmDst),
                std::make_shared<Assembly::Jmp>(endLbl),
                std::make_shared<Assembly::Label>(outOfBound),
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Double()), asmSrc, x),
                std::make_shared<Assembly::Binary>(Assembly::BinaryOp::Sub, std::make_shared<Assembly::AsmType>(Assembly::Double()), std::make_shared<Assembly::Data>(upperBound, 0), x),
                std::make_shared<Assembly::Cvttsd2si>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), x, asmDst),
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Quadword()), upperBoundAsInt, r),
                std::make_shared<Assembly::Binary>(Assembly::BinaryOp::Add, std::make_shared<Assembly::AsmType>(Assembly::Quadword()), r, asmDst),
                std::make_shared<Assembly::Label>(endLbl),
            };
        }
    }
    case TACKY::NodeType::CopyToOffset:
    {
        auto copyToOffset = std::dynamic_pointer_cast<TACKY::CopyToOffset>(inst);

        if (Types::isScalar(*tackyType(copyToOffset->getSrc())))
        {
            return {
                std::make_shared<Assembly::Mov>(
                    getAsmType(copyToOffset->getSrc()),
                    convertVal(copyToOffset->getSrc()),
                    std::make_shared<Assembly::PseudoMem>(copyToOffset->getDst(), copyToOffset->getOffset())),
            };
        }
        else
        {
            auto asmSrc = convertVal(copyToOffset->getSrc());
            auto asmDst = std::make_shared<Assembly::PseudoMem>(copyToOffset->getDst(), copyToOffset->getOffset());
            auto byteCount = Types::getSize(*tackyType(copyToOffset->getSrc()), _typeTable);

            return copyBytes(asmSrc, asmDst, byteCount);
        }
    }
    case TACKY::NodeType::CopyFromOffset:
    {
        auto copyFromOffset = std::dynamic_pointer_cast<TACKY::CopyFromOffset>(inst);

        if (Types::isScalar(*tackyType(copyFromOffset->getDst())))
        {
            return {
                std::make_shared<Assembly::Mov>(
                    getAsmType(copyFromOffset->getDst()),
                    std::make_shared<Assembly::PseudoMem>(copyFromOffset->getSrc(), copyFromOffset->getOffset()),
                    convertVal(copyFromOffset->getDst())),
            };
        }
        else
        {
            auto asmSrc = std::make_shared<Assembly::PseudoMem>(copyFromOffset->getSrc(), copyFromOffset->getOffset());
            auto asmDst = convertVal(copyFromOffset->getDst());
            auto byteCount = Types::getSize(*tackyType(copyFromOffset->getDst()), _typeTable);

            return copyBytes(asmSrc, asmDst, byteCount);
        }
    }
    case TACKY::NodeType::AddPtr:
    {
        auto addPtr = std::dynamic_pointer_cast<TACKY::AddPtr>(inst);

        if (auto c = [addPtr, this]() -> std::optional<int64_t>
            {
                auto cnst = std::dynamic_pointer_cast<TACKY::Constant>(addPtr->getIndex());
                if (cnst && Constants::isConstLong(*cnst->getConst()))
                {
                    return Constants::getConstLong(*cnst->getConst())->val;
                }

                return std::nullopt;
            }())
        {
            // note that typechecker converts index to long
            // QUESTION: what's the largest offset we should support?
            auto i = c;
            return {
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                    convertVal(addPtr->getPtr()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R9)),
                std::make_shared<Assembly::Lea>(
                    std::make_shared<Assembly::Memory>(
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R9),
                        i.value() * addPtr->getScale()),
                    convertVal(addPtr->getDst())),
            };
        }
        else
        {
            if (addPtr->getScale() == 1 || addPtr->getScale() == 2 || addPtr->getScale() == 4 || addPtr->getScale() == 8)
            {
                return {
                    std::make_shared<Assembly::Mov>(
                        std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                        convertVal(addPtr->getPtr()),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R8)),
                    std::make_shared<Assembly::Mov>(
                        std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                        convertVal(addPtr->getIndex()),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R9)),
                    std::make_shared<Assembly::Lea>(
                        std::make_shared<Assembly::Indexed>(
                            std::make_shared<Assembly::Reg>(Assembly::RegName::R8),
                            std::make_shared<Assembly::Reg>(Assembly::RegName::R9),
                            addPtr->getScale()),
                        convertVal(addPtr->getDst())),
                };
            }
            else
            {
                return {
                    std::make_shared<Assembly::Mov>(
                        std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                        convertVal(addPtr->getPtr()),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R8)),
                    std::make_shared<Assembly::Mov>(
                        std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                        convertVal(addPtr->getIndex()),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R9)),
                    std::make_shared<Assembly::Binary>(
                        Assembly::BinaryOp::Mult,
                        std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                        std::make_shared<Assembly::Imm>(addPtr->getScale()),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R9)),
                    std::make_shared<Assembly::Lea>(
                        std::make_shared<Assembly::Indexed>(
                            std::make_shared<Assembly::Reg>(Assembly::RegName::R8),
                            std::make_shared<Assembly::Reg>(Assembly::RegName::R9),
                            1),
                        convertVal(addPtr->getDst())),
                };
            }
        }
    }
    default:
        throw std::runtime_error("Internal Error: Invalid TACKY instruction");
    }
}

std::shared_ptr<Assembly::TopLevel>
CodeGen::convertTopLevel(const std::shared_ptr<TACKY::TopLevel> &topLevel)
{
    if (auto fn = std::dynamic_pointer_cast<TACKY::Function>(topLevel))
    {
        std::vector<std::shared_ptr<Assembly::Instruction>> insts{};
        std::vector<std::shared_ptr<TACKY::Val>> paramsAsTacky;
        bool returnOnStack = returnsOnStack(fn->getName());
        const auto &params = fn->getParams();
        paramsAsTacky.reserve(params.size());
        std::transform(params.begin(), params.end(), std::back_inserter(paramsAsTacky),
                       [](const std::string &name)
                       {
                           return std::make_shared<TACKY::Var>(name);
                       });

        auto paramInsts = passParams(paramsAsTacky, returnOnStack);
        insts.insert(insts.end(), paramInsts.begin(), paramInsts.end());

        for (auto &inst : fn->getInstructions())
        {
            auto asmInsts = convertInstruction(inst);
            insts.insert(insts.end(), asmInsts.begin(), asmInsts.end());
        }

        return std::make_shared<Assembly::Function>(fn->getName(), fn->isGlobal(), insts);
    }
    else if (auto staticVar = std::dynamic_pointer_cast<TACKY::StaticVariable>(topLevel))
    {
        return std::make_shared<Assembly::StaticVariable>(
            staticVar->getName(),
            staticVar->isGlobal(),
            getVarAlignment(staticVar->getDataType()),
            staticVar->getInits());
    }
    else if (auto staticConst = std::dynamic_pointer_cast<TACKY::StaticConstant>(topLevel))
    {
        return std::make_shared<Assembly::StaticConstant>(
            staticConst->getName(),
            getVarAlignment(staticConst->getDataType()),
            *staticConst->getInit());
    }
    else
    {
        throw std::runtime_error("Internal Error: Invalid TACKY top level");
    }
}

std::shared_ptr<Assembly::StaticConstant>
CodeGen::convertConstant(double key, const std::pair<std::string, size_t> &constant)
{
    _asmSymbolTable.addConstant(constant.first, std::make_shared<Assembly::AsmType>(Assembly::Double()));
    return std::make_shared<Assembly::StaticConstant>(
        constant.first,
        constant.second,
        Initializers::DoubleInit(key));
}

void CodeGen::convertSymbol(const std::string &name, const Symbols::Symbol &symbol)
{
    if (auto funAttr = Symbols::getFunAttr(symbol.attrs))
    {
        auto fnType = Types::getFunType(symbol.type);
        // If this function has incomplete return type (implying we don't define or call it in this translation unit)
        // use a dummy value for fun_returns_on_stack
        auto funReturnsOnStack = Types::isComplete(*fnType->retType, _typeTable) || Types::isVoidType(*fnType->retType)
                                     ? returnsOnStack(name)
                                     : false;
        _asmSymbolTable.addFun(name, funAttr->defined, funReturnsOnStack);
    }
    else if (auto constAttr = Symbols::getConstAttr(symbol.attrs))
    {
        _asmSymbolTable.addConstant(name, convertType(symbol.type));
    }
    else if (auto staticAttr = Symbols::getStaticAttr(symbol.attrs))
    {
        if (Types::isComplete(symbol.type, _typeTable))
        {
            // use dummy type for static variables of incomplete type:
            _asmSymbolTable.addVar(name, std::make_shared<Assembly::AsmType>(Assembly::Byte()), true);
        }
        else
        {
            _asmSymbolTable.addVar(name, convertVarType(symbol.type), true);
        }
    }
    else
    {
        _asmSymbolTable.addVar(name, convertVarType(symbol.type), false);
    }
}

std::shared_ptr<Assembly::Program>
CodeGen::gen(std::shared_ptr<TACKY::Program> prog)
{
    // Clear the hashtable (necessary if we're compiling multiple sources)
    _constants.clear();
    std::vector<std::shared_ptr<Assembly::TopLevel>> convertedTl{};

    for (const auto &tl : prog->getTopLevels())
    {
        auto asmTl = convertTopLevel(tl);
        convertedTl.push_back(asmTl);
    }

    std::vector<std::shared_ptr<Assembly::StaticConstant>> convertedConstants{};

    for (const auto &[key, constant] : _constants)
    {
        convertedConstants.push_back(convertConstant(key, constant));
    }

    for (const auto &[name, symbol] : _symbolTable.getAllSymbols())
    {
        convertSymbol(name, symbol);
    }

    convertedTl.insert(convertedTl.begin(), convertedConstants.begin(), convertedConstants.end());
    return std::make_shared<Assembly::Program>(convertedTl);
}
