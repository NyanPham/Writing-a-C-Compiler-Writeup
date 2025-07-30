#include "AddressTaken.h"

std::set<std::string> analyzeAddressTaken(const std::vector<std::shared_ptr<TACKY::Instruction>> &instructions)
{
    std::set<std::string> result;
    for (const auto &instr : instructions)
    {
        if (instr->getType() == TACKY::NodeType::GetAddress)
        {
            auto getAddr = std::static_pointer_cast<TACKY::GetAddress>(instr);
            auto src = getAddr->getSrc();
            if (src->getType() == TACKY::NodeType::Var)
            {
                auto var = std::static_pointer_cast<TACKY::Var>(src);
                result.insert(var->getName());
            }
        }
    }
    return result;
}

std::set<std::string> analyzeProgram(const std::vector<std::shared_ptr<TACKY::TopLevel>> &topLevels)
{
    std::set<std::string> result;
    for (const auto &tl : topLevels)
    {
        if (auto fn = std::dynamic_pointer_cast<TACKY::Function>(tl))
        {
            auto vars = analyzeAddressTaken(fn->getInstructions());
            result.insert(vars.begin(), vars.end());
        }
    }
    return result;
}