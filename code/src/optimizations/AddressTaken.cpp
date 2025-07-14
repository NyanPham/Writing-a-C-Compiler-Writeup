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