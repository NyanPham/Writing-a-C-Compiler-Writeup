#include <string>
#include <memory>
#include <vector>
#include <limits>

#include "Assembly.h"
#include "InstructionFixup.h"
#include "../Rounding.h"

inline int64_t convertTo64Bit(int32_t value)
{
    return static_cast<int64_t>(value);
}

inline bool isLarge(const int64_t imm)
{
    int64_t int32Max = convertTo64Bit(std::numeric_limits<int32_t>::max());
    int64_t int32Min = convertTo64Bit(std::numeric_limits<int32_t>::min());

    return imm > int32Max || imm < int32Min;
}

bool isLargerThanUint(const int64_t imm)
{
    int64_t maxUnsigned32 = convertTo64Bit(static_cast<int64_t>(std::numeric_limits<uint32_t>::max()));
    int64_t int32Min = convertTo64Bit(std::numeric_limits<int32_t>::min());
    return imm > maxUnsigned32 || imm < int32Min;
}

bool isMemoryOperand(const std::shared_ptr<Assembly::Operand> &operand)
{
    switch (operand->getType())
    {
    case Assembly::NodeType::Stack:
    case Assembly::NodeType::Data:
        return true;
    default:
        return false;
    }
}

bool isImmOperand(const std::shared_ptr<Assembly::Operand> &operand)
{
    return operand->getType() == Assembly::NodeType::Imm;
}

std::vector<std::shared_ptr<Assembly::Instruction>>
InstructionFixup::fixupInstruction(const std::shared_ptr<Assembly::Instruction> &inst)
{
    switch (inst->getType())
    {
    case Assembly::NodeType::Mov:
    {
        /* Mov can't move a value from one memory address to another */
        auto mov = std::dynamic_pointer_cast<Assembly::Mov>(inst);

        if (isMemoryOperand(mov->getSrc()) && isMemoryOperand(mov->getDst()))
        {
            auto scratchReg{
                std::make_shared<Assembly::Reg>(Assembly::isAsmDouble(*mov->getAsmType())
                                                    ? Assembly::RegName::XMM14
                                                    : Assembly::RegName::R10)};

            return {
                std::make_shared<Assembly::Mov>(mov->getAsmType(), mov->getSrc(), scratchReg),
                std::make_shared<Assembly::Mov>(mov->getAsmType(), scratchReg, mov->getDst()),
            };
        }
        // Mov can't move a large constant to a memory address
        else if (
            Assembly::isAsmQuadword(*mov->getAsmType()) &&
            isImmOperand(mov->getSrc()) &&
            isMemoryOperand(mov->getDst()) &&
            isLarge(std::dynamic_pointer_cast<Assembly::Imm>(mov->getSrc())->getValue()))
        {
            return {
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                    mov->getSrc(),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10),
                    mov->getDst()),
            };
        }
        // Moving a quadword-size constant with a longword operand size produces assembler warning
        else if (
            Assembly::isAsmLongword(*mov->getAsmType()) &&
            isImmOperand(mov->getSrc()) &&
            isLargerThanUint(std::dynamic_pointer_cast<Assembly::Imm>(mov->getSrc())->getValue()))
        {
            // reduce modulo 2^32 by zeroing out upper 32 bit
            int64_t bitmask = convertTo64Bit(0xffffffff);
            int64_t reduced = std::dynamic_pointer_cast<Assembly::Imm>(mov->getSrc())->getValue() & bitmask;

            return {
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Longword()),
                    std::make_shared<Assembly::Imm>(reduced),
                    mov->getDst()),
            };
        }
        else
        {
            return {
                inst,
            };
        }
    }
    case Assembly::NodeType::Movsx:
    {
        auto movsx = std::dynamic_pointer_cast<Assembly::Movsx>(inst);
        // Movsx cannot handle immediate src or memory dst

        if (
            isImmOperand(movsx->getSrc()) &&
            isMemoryOperand(movsx->getDst()))
        {
            return {
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Longword()),
                    std::make_shared<Assembly::Imm>(std::dynamic_pointer_cast<Assembly::Imm>(movsx->getSrc())->getValue()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                std::make_shared<Assembly::Movsx>(
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R11)),
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R11),
                    movsx->getDst()),
            };
        }
        else if (movsx->getSrc()->getType() == Assembly::NodeType::Imm)
        {
            return {
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Longword()),
                    std::make_shared<Assembly::Imm>(std::dynamic_pointer_cast<Assembly::Imm>(movsx->getSrc())->getValue()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                std::make_shared<Assembly::Movsx>(
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10),
                    movsx->getDst()),
            };
        }
        else if (isMemoryOperand(movsx->getDst()))
        {
            return {
                std::make_shared<Assembly::Movsx>(
                    movsx->getSrc(),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R11)),
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R11),
                    movsx->getDst()),
            };
        }
        else
        {
            return {
                inst,
            };
        }
    }
    case Assembly::NodeType::MovZeroExtend:
    {
        /* Rewrite MovZeroExtend as one or two instructions */
        auto movzx = std::dynamic_pointer_cast<Assembly::MovZeroExtend>(inst);

        if (movzx->getSrc()->getType() == Assembly::NodeType::Reg)
        {
            return {
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Longword()),
                    movzx->getSrc(),
                    movzx->getDst()),
            };
        }
        else
        {
            return {
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Longword()),
                    movzx->getSrc(),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R11)),
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R11),
                    movzx->getDst()),
            };
        }
    }
    case Assembly::NodeType::Idiv:
    {
        /* Idiv cannot operate on constant */
        auto idiv = std::dynamic_pointer_cast<Assembly::Idiv>(inst);

        if (isImmOperand(idiv->getOperand()))
        {
            return {
                std::make_shared<Assembly::Mov>(
                    idiv->getAsmType(),
                    std::dynamic_pointer_cast<Assembly::Imm>(idiv->getOperand()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                std::make_shared<Assembly::Idiv>(
                    idiv->getAsmType(),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
            };
        }
        else
        {
            return {
                inst,
            };
        }
    }
    case Assembly::NodeType::Div:
    {
        /* Div cannot operate on constant */
        auto div = std::dynamic_pointer_cast<Assembly::Div>(inst);

        if (isImmOperand(div->getOperand()))
        {
            return {
                std::make_shared<Assembly::Mov>(
                    div->getAsmType(),
                    std::dynamic_pointer_cast<Assembly::Imm>(div->getOperand()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                std::make_shared<Assembly::Div>(
                    div->getAsmType(),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
            };
        }
        else
        {
            return {
                inst,
            };
        }
    }
    case Assembly::NodeType::Binary:
    {
        auto binary = std::dynamic_pointer_cast<Assembly::Binary>(inst);

        switch (binary->getOp())
        {
        case Assembly::BinaryOp::Add:
        case Assembly::BinaryOp::Sub:
        case Assembly::BinaryOp::And:
        case Assembly::BinaryOp::Or:
        case Assembly::BinaryOp::Xor:
        {
            // Binary operations on double require register as destination
            if (
                Assembly::isAsmDouble(*binary->getAsmType()))
            {
                if (binary->getDst()->getType() == Assembly::NodeType::Reg)
                {
                    return {
                        binary,
                    };
                }

                return {
                    std::make_shared<Assembly::Mov>(
                        std::make_shared<Assembly::AsmType>(Assembly::Double()),
                        binary->getDst(),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::XMM15)),
                    std::make_shared<Assembly::Binary>(
                        binary->getOp(),
                        std::make_shared<Assembly::AsmType>(Assembly::Double()),
                        binary->getSrc(),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::XMM15)),
                    std::make_shared<Assembly::Mov>(
                        std::make_shared<Assembly::AsmType>(Assembly::Double()),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::XMM15),
                        binary->getDst()),
                };
            }

            // Add/Sub/And/Or/Xor can't take large immediates as source operands
            if (
                Assembly::isAsmQuadword(*binary->getAsmType()) &&
                isImmOperand(binary->getSrc()) &&
                isLarge(std::dynamic_pointer_cast<Assembly::Imm>(binary->getSrc())->getValue()))
            {
                return {
                    std::make_shared<Assembly::Mov>(
                        std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                        binary->getSrc(),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                    std::make_shared<Assembly::Binary>(
                        binary->getOp(),
                        std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R10),
                        binary->getDst()),
                };
            }

            // Add/Sub/And/Or/Xor can't use memory addresses for both operands */
            if (isMemoryOperand(binary->getSrc()) && isMemoryOperand(binary->getDst()))
            {
                auto RegR10{std::make_shared<Assembly::Reg>(Assembly::RegName::R10)};

                return {
                    std::make_shared<Assembly::Mov>(binary->getAsmType(), binary->getSrc(), RegR10),
                    std::make_shared<Assembly::Binary>(binary->getOp(), binary->getAsmType(), RegR10, binary->getDst()),
                };
            }
            else
            {
                return {
                    binary,
                };
            }
        }
        case Assembly::BinaryOp::Mult:
        {
            /*
                Mult can't have destination as a memory address;
                And its source cannot be a large operand.
            */
            if (
                isMemoryOperand(binary->getDst()) &&
                Assembly::isAsmQuadword(*binary->getAsmType()) &&
                isImmOperand(binary->getSrc()) &&
                isLarge(std::dynamic_pointer_cast<Assembly::Imm>(binary->getSrc())->getValue()))
            {
                // rewrite both operands
                return {
                    std::make_shared<Assembly::Mov>(
                        std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                        binary->getSrc(),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                    std::make_shared<Assembly::Mov>(
                        std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                        binary->getDst(),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R11)),
                    std::make_shared<Assembly::Binary>(
                        Assembly::BinaryOp::Mult,
                        std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R10),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R11)),
                    std::make_shared<Assembly::Mov>(
                        std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R11),
                        binary->getDst()),
                };
            }

            if (
                Assembly::isAsmQuadword(*binary->getAsmType()) &&
                isImmOperand(binary->getSrc()) &&
                isLarge(std::dynamic_pointer_cast<Assembly::Imm>(binary->getSrc())->getValue()))
            {
                // just rewrite src
                return {
                    std::make_shared<Assembly::Mov>(
                        std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                        binary->getSrc(),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                    std::make_shared<Assembly::Binary>(
                        Assembly::BinaryOp::Mult,
                        std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R10),
                        binary->getDst()),
                };
            }

            if (isMemoryOperand(binary->getDst()))
            {
                return {
                    std::make_shared<Assembly::Mov>(
                        binary->getAsmType(),
                        binary->getDst(),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R11)),
                    std::make_shared<Assembly::Binary>(
                        Assembly::BinaryOp::Mult,
                        binary->getAsmType(),
                        binary->getSrc(),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R11)),
                    std::make_shared<Assembly::Mov>(
                        binary->getAsmType(),
                        std::make_shared<Assembly::Reg>(Assembly::RegName::R11),
                        binary->getDst()),
                };
            }
            else
            {
                return {
                    binary};
            }
        }

        default:
        {
            return {
                inst,
            };
        }
        }
    }
    case Assembly::NodeType::Cmp:
    {
        auto cmp = std::dynamic_pointer_cast<Assembly::Cmp>(inst);
        // Destination of comisd must be a register
        if (Assembly::isAsmDouble(*cmp->getAsmType()))
        {
            if (cmp->getDst()->getType() == Assembly::NodeType::Reg)
            {
                return {
                    cmp,
                };
            }

            return {
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Double()),
                    cmp->getDst(),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::XMM15)),
                std::make_shared<Assembly::Cmp>(
                    std::make_shared<Assembly::AsmType>(Assembly::Double()),
                    cmp->getSrc(),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::XMM15)),
            };
        }

        // Both operands of cmp can't be in memory
        if (isMemoryOperand(cmp->getSrc()) && isMemoryOperand(cmp->getDst()))
        {
            return {
                std::make_shared<Assembly::Mov>(
                    cmp->getAsmType(),
                    cmp->getSrc(),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                std::make_shared<Assembly::Cmp>(
                    cmp->getAsmType(),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10),
                    cmp->getDst()),
            };
        }
        // First operand of Cmp can't be a large constant, second can't be a constant at all.
        else if (
            Assembly::isAsmQuadword(*cmp->getAsmType()) &&
            isImmOperand(cmp->getSrc()) &&
            isImmOperand(cmp->getDst()) &&
            isLarge(std::dynamic_pointer_cast<Assembly::Imm>(cmp->getSrc())->getValue()))
        {
            return {
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                    cmp->getSrc(),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                    cmp->getDst(),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R11)),
                std::make_shared<Assembly::Cmp>(
                    std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R11)),
            };
        }
        else if (
            Assembly::isAsmQuadword(*cmp->getAsmType()) &&
            isImmOperand(cmp->getSrc()) &&
            isLarge(std::dynamic_pointer_cast<Assembly::Imm>(cmp->getSrc())->getValue()))
        {
            return {
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                    cmp->getSrc(),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                std::make_shared<Assembly::Cmp>(
                    std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10),
                    cmp->getDst()),
            };
        }
        else if (isImmOperand(cmp->getDst()))
        {
            // Destination of cmp cannot be an immediate
            auto immVal = std::dynamic_pointer_cast<Assembly::Imm>(cmp->getDst())->getValue();

            return {
                std::make_shared<Assembly::Mov>(
                    cmp->getAsmType(),
                    std::make_shared<Assembly::Imm>(immVal),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R11)),
                std::make_shared<Assembly::Cmp>(
                    cmp->getAsmType(),
                    cmp->getSrc(),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R11))};
        }
        else
        {
            return {
                inst,
            };
        }
    }
    case Assembly::NodeType::Push:
    {
        auto push = std::dynamic_pointer_cast<Assembly::Push>(inst);

        if (isImmOperand(push->getOperand()) && isLarge(std::dynamic_pointer_cast<Assembly::Imm>(push->getOperand())->getValue()))
        {
            return {
                std::make_shared<Assembly::Mov>(
                    std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                    push->getOperand(),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                std::make_shared<Assembly::Push>(
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
            };
        }
        else
        {
            return {
                inst,
            };
        }
    }
    case Assembly::NodeType::Cvttsd2si:
    {
        auto cvt = std::dynamic_pointer_cast<Assembly::Cvttsd2si>(inst);
        if (cvt->getDst()->getType() != Assembly::NodeType::Reg)
        {
            return {
                std::make_shared<Assembly::Cvttsd2si>(cvt->getAsmType(), cvt->getSrc(), std::make_shared<Assembly::Reg>(Assembly::RegName::R11)),
                std::make_shared<Assembly::Mov>(cvt->getAsmType(), std::make_shared<Assembly::Reg>(Assembly::RegName::R11), cvt->getDst()),
            };
        }
        else
        {
            return {
                inst,
            };
        }
    }
    case Assembly::NodeType::Cvtsi2sd:
    {
        auto cvt = std::dynamic_pointer_cast<Assembly::Cvtsi2sd>(inst);
        if (isImmOperand(cvt->getSrc()) && isMemoryOperand(cvt->getDst()))
        {
            return {
                std::make_shared<Assembly::Mov>(cvt->getAsmType(), cvt->getSrc(), std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                std::make_shared<Assembly::Cvtsi2sd>(cvt->getAsmType(), std::make_shared<Assembly::Reg>(Assembly::RegName::R10), std::make_shared<Assembly::Reg>(Assembly::RegName::XMM15)),
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Double()), std::make_shared<Assembly::Reg>(Assembly::RegName::XMM15), cvt->getDst()),
            };
        }
        else if (isImmOperand(cvt->getSrc()))
        {
            return {
                std::make_shared<Assembly::Mov>(cvt->getAsmType(), cvt->getSrc(), std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                std::make_shared<Assembly::Cvtsi2sd>(cvt->getAsmType(), std::make_shared<Assembly::Reg>(Assembly::RegName::R10), cvt->getDst()),
            };
        }
        else if (isMemoryOperand(cvt->getDst()))
        {
            return {
                std::make_shared<Assembly::Cvtsi2sd>(cvt->getAsmType(), cvt->getSrc(), std::make_shared<Assembly::Reg>(Assembly::RegName::XMM15)),
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::AsmType>(Assembly::Double()), std::make_shared<Assembly::Reg>(Assembly::RegName::XMM15), cvt->getDst()),
            };
        }
        else
        {
            return {
                inst,
            };
        }
    }
    default:
    {
        return {
            inst,
        };
    }
    }
}

std::shared_ptr<Assembly::TopLevel>
InstructionFixup::fixupTopLevel(const std::shared_ptr<Assembly::TopLevel> &topLevel)
{
    if (auto fun = std::dynamic_pointer_cast<Assembly::Function>(topLevel))
    {
        auto stackBytes = Rounding::roundAwayFromZero(16, -_asmSymbolTable.getBytesRequired(fun->getName()));
        auto stackByteOperand = std::make_shared<Assembly::Imm>(static_cast<int64_t>(stackBytes));

        std::vector<std::shared_ptr<Assembly::Instruction>> fixedInstructions{
            std::make_shared<Assembly::Binary>(
                Assembly::BinaryOp::Sub,
                std::make_shared<Assembly::AsmType>(Assembly::Quadword()),
                stackByteOperand,
                std::make_shared<Assembly::Reg>(Assembly::RegName::SP)),
        };

        for (auto &inst : fun->getInstructions())
        {
            auto innerFixedInsts = fixupInstruction(inst);
            fixedInstructions.insert(fixedInstructions.end(), innerFixedInsts.begin(), innerFixedInsts.end());
        }

        return std::make_shared<Assembly::Function>(fun->getName(), fun->isGlobal(), fixedInstructions);
    }
    else
        return topLevel;
}

std::shared_ptr<Assembly::Program>
InstructionFixup::fixupProgram(const std::shared_ptr<Assembly::Program> &prog)
{
    std::vector<std::shared_ptr<Assembly::TopLevel>> fixedTls{};

    for (auto &tl : prog->getTopLevels())
    {
        fixedTls.push_back(fixupTopLevel(tl));
    }

    return std::make_shared<Assembly::Program>(fixedTls);
}
