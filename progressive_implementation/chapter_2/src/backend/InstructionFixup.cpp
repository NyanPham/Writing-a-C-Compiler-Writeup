#include <string>
#include <memory>
#include <vector>

#include "Assembly.h"
#include "InstructionFixup.h"

std::vector<std::shared_ptr<Assembly::Instruction>> InstructionFixup::fixupInstruction(const std::shared_ptr<Assembly::Instruction> &inst)
{
    switch (inst->getType())
    {
    case Assembly::NodeType::Mov:
    {
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
                inst};
        }
    };
    default:
        return {
            inst};
    }
}

std::shared_ptr<Assembly::Function> InstructionFixup::fixupFunction(const std::shared_ptr<Assembly::Function> &fun, int lastStackSlot)
{
    std::vector<std::shared_ptr<Assembly::Instruction>> fixedInstructions{
        std::make_shared<Assembly::AllocateStack>(-lastStackSlot)};

    for (auto &inst : fun->getInstructions())
    {
        auto innerFixedInsts = fixupInstruction(inst);
        fixedInstructions.insert(fixedInstructions.end(), innerFixedInsts.begin(), innerFixedInsts.end());
    }

    return std::make_shared<Assembly::Function>(fun->getName(), fixedInstructions);
}

std::shared_ptr<Assembly::Program> InstructionFixup::fixupProgram(const std::shared_ptr<Assembly::Program> &prog, int lastStackSlot)
{
    return std::make_shared<Assembly::Program>(fixupFunction(prog->getFunction(), lastStackSlot));
}
