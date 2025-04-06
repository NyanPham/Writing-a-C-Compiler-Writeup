#include <string>
#include <memory>
#include <vector>

#include "Assembly.h"
#include "InstructionFixup.h"
#include "Rounding.h"

std::vector<std::shared_ptr<Assembly::Instruction>>
InstructionFixup::fixupInstruction(const std::shared_ptr<Assembly::Instruction> &inst)
{
    switch (inst->getType())
    {
    case Assembly::NodeType::Mov:
    {
        /* Mov can't move a value from one memory address to another */
        auto mov = std::dynamic_pointer_cast<Assembly::Mov>(inst);

        if (
            mov->getSrc()->getType() == Assembly::NodeType::Stack &&
            mov->getDst()->getType() == Assembly::NodeType::Stack)
        {
            auto RegR10{std::make_shared<Assembly::Reg>(Assembly::RegName::R10)};

            return {
                std::make_shared<Assembly::Mov>(mov->getSrc(), RegR10),
                std::make_shared<Assembly::Mov>(RegR10, mov->getDst()),
            };
        }
        else
        {
            return {
                inst,
            };
        }
    }
    case Assembly::NodeType::Idiv:
    {
        /* Idiv cannot operate on constant */
        auto idiv = std::dynamic_pointer_cast<Assembly::Idiv>(inst);

        if (idiv->getOperand()->getType() == Assembly::NodeType::Imm)
        {
            return {
                std::make_shared<Assembly::Mov>(
                    std::dynamic_pointer_cast<Assembly::Imm>(idiv->getOperand()),
                    std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
                std::make_shared<Assembly::Idiv>(std::make_shared<Assembly::Reg>(Assembly::RegName::R10)),
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
            /* Add/Sub can't use memory addresses for both operands */
            if (binary->getSrc()->getType() == Assembly::NodeType::Stack &&
                binary->getDst()->getType() == Assembly::NodeType::Stack)
            {
                auto RegR10{std::make_shared<Assembly::Reg>(Assembly::RegName::R10)};

                return {
                    std::make_shared<Assembly::Mov>(binary->getSrc(), RegR10),
                    std::make_shared<Assembly::Binary>(binary->getOp(), RegR10, binary->getDst()),
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
            /* Mult can't have destination as a memory address */
            if (binary->getDst()->getType() == Assembly::NodeType::Stack)
            {
                auto RegR11{std::make_shared<Assembly::Reg>(Assembly::RegName::R11)};

                return {
                    std::make_shared<Assembly::Mov>(binary->getDst(), RegR11),
                    std::make_shared<Assembly::Binary>(Assembly::BinaryOp::Mult, binary->getSrc(), RegR11),
                    std::make_shared<Assembly::Mov>(RegR11, binary->getDst()),
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
                binary,
            };
        }
        }
    }
    case Assembly::NodeType::Cmp:
    {
        auto cmp = std::dynamic_pointer_cast<Assembly::Cmp>(inst);

        if (cmp->getSrc()->getType() == Assembly::NodeType::Stack && cmp->getDst()->getType() == Assembly::NodeType::Stack)
        {
            // Both operands of cmp can't be in memory
            auto r10Reg{std::make_shared<Assembly::Reg>(Assembly::RegName::R10)};

            return {
                std::make_shared<Assembly::Mov>(cmp->getSrc(), r10Reg),
                std::make_shared<Assembly::Cmp>(r10Reg, cmp->getDst()),
            };
        }
        else if (cmp->getDst()->getType() == Assembly::NodeType::Imm)
        {
            // Destination of cmp cannot be an immediate
            auto r11Reg{std::make_shared<Assembly::Reg>(Assembly::RegName::R11)};
            auto immVal = std::dynamic_pointer_cast<Assembly::Imm>(cmp->getDst())->getValue();

            return {
                std::make_shared<Assembly::Mov>(std::make_shared<Assembly::Imm>(immVal), r11Reg),
                std::make_shared<Assembly::Cmp>(cmp->getSrc(), r11Reg)};
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
            inst};
    }
    }
}

std::shared_ptr<Assembly::Function>
InstructionFixup::fixupFunction(const std::shared_ptr<Assembly::Function> &fun)
{
    auto stackBytes = -_symbolTable.get(fun->getName()).stackFrameSize;

    std::vector<std::shared_ptr<Assembly::Instruction>> fixedInstructions{
        std::make_shared<Assembly::AllocateStack>(Rounding::roundAwayFromZero(16, stackBytes)),
    };

    for (auto &inst : fun->getInstructions())
    {
        auto innerFixedInsts = fixupInstruction(inst);
        fixedInstructions.insert(fixedInstructions.end(), innerFixedInsts.begin(), innerFixedInsts.end());
    }

    return std::make_shared<Assembly::Function>(fun->getName(), fixedInstructions);
}

std::shared_ptr<Assembly::Program>
InstructionFixup::fixupProgram(const std::shared_ptr<Assembly::Program> &prog)
{
    std::vector<std::shared_ptr<Assembly::Function>> fixedFns{};

    for (auto &fn : prog->getFunctions())
    {
        fixedFns.push_back(fixupFunction(fn));
    }

    return std::make_shared<Assembly::Program>(fixedFns);
}
