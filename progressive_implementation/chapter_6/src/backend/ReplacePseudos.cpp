#include <utility>
#include <map>
#include <string>
#include <memory>

#include "Assembly.h"
#include "ReplacePseudos.h"

ReplacementState ReplacePseudos::createInitState()
{
    return {
        currOffset : 0,
        offsetMap : {},
    };
}

ReplaceOperandPair ReplacePseudos::replaceOperand(const std::shared_ptr<Assembly::Operand> &operand, ReplacementState &state)
{
    switch (operand->getType())
    {
    case Assembly::NodeType::Pseudo:
    {
        auto pseudo = std::dynamic_pointer_cast<Assembly::Pseudo>(operand);

        auto it = state.offsetMap.find(pseudo->getName());
        if (it == state.offsetMap.end())
        {
            int newOffset = state.currOffset - 4;
            state.offsetMap[pseudo->getName()] = newOffset;

            ReplacementState newState = {
                currOffset : newOffset,
                offsetMap : state.offsetMap,
            };

            return {
                std::make_shared<Assembly::Stack>(newOffset),
                newState,
            };
        }
        else
        {
            return {
                std::make_shared<Assembly::Stack>(it->second),
                state};
        }
    }
    default:
        return {operand, state};
    }
}

ReplaceInstPair ReplacePseudos::replacePseudosInInstruction(const std::shared_ptr<Assembly::Instruction> &inst, ReplacementState &state)
{
    switch (inst->getType())
    {
    case Assembly::NodeType::Mov:
    {
        auto mov = std::dynamic_pointer_cast<Assembly::Mov>(inst);

        auto [newSrc, state1] = replaceOperand(mov->getSrc(), state);
        auto [newDst, state2] = replaceOperand(mov->getDst(), state1);

        auto newMov = std::make_shared<Assembly::Mov>(newSrc, newDst);

        return {
            newMov,
            state2,
        };
    }
    case Assembly::NodeType::Unary:
    {
        auto unary = std::dynamic_pointer_cast<Assembly::Unary>(inst);

        auto [newDst, state1] = replaceOperand(unary->getOperand(), state);

        auto newUnary = std::make_shared<Assembly::Unary>(unary->getOp(), newDst);

        return {
            newUnary,
            state1,
        };
    }
    case Assembly::NodeType::Binary:
    {
        auto binary = std::dynamic_pointer_cast<Assembly::Binary>(inst);

        auto [newSrc, state1] = replaceOperand(binary->getSrc(), state);
        auto [newDst, state2] = replaceOperand(binary->getDst(), state1);

        auto newBinary = std::make_shared<Assembly::Binary>(binary->getOp(), newSrc, newDst);

        return {
            newBinary,
            state2,
        };
    }
    case Assembly::NodeType::Cmp:
    {
        auto cmp = std::dynamic_pointer_cast<Assembly::Cmp>(inst);

        auto [newSrc, state1] = replaceOperand(cmp->getSrc(), state);
        auto [newDst, state2] = replaceOperand(cmp->getDst(), state1);

        auto newCmp = std::make_shared<Assembly::Cmp>(newSrc, newDst);

        return {
            newCmp,
            state2,
        };
    }
    case Assembly::NodeType::Idiv:
    {
        auto idiv = std::dynamic_pointer_cast<Assembly::Idiv>(inst);

        auto [newOperand, state1] = replaceOperand(idiv->getOperand(), state);

        auto newIdiv = std::make_shared<Assembly::Idiv>(newOperand);

        return {
            newIdiv,
            state1,
        };
    }
    case Assembly::NodeType::SetCC:
    {
        auto setCC = std::dynamic_pointer_cast<Assembly::SetCC>(inst);

        auto [newOperand, state1] = replaceOperand(setCC->getOperand(), state);

        auto newSetCC = std::make_shared<Assembly::SetCC>(setCC->getCondCode(), newOperand);

        return {
            newSetCC,
            state1,
        };
    }
    case Assembly::NodeType::Ret:
    case Assembly::NodeType::Cdq:
    case Assembly::NodeType::Label:
    case Assembly::NodeType::JmpCC:
    case Assembly::NodeType::Jmp:
    {
        return {
            inst,
            state,
        };
    }
    default:
        throw std::runtime_error("Invalid instruction to replace operand!");
    }
}

ReplaceFunctionPair ReplacePseudos::replacePseudosInFunction(const std::shared_ptr<Assembly::Function> &func)
{
    auto currState{createInitState()};
    std::vector<std::shared_ptr<Assembly::Instruction>> fixedInstructions{};

    for (auto &inst : func->getInstructions())
    {
        auto [newInst, newState] = replacePseudosInInstruction(inst, currState);
        currState = newState;
        fixedInstructions.push_back(newInst);
    }

    return {
        std::make_shared<Assembly::Function>(func->getName(), fixedInstructions),
        currState.currOffset,
    };
}

ReplaceProgramPair ReplacePseudos::replacePseudos(const std::shared_ptr<Assembly::Program> &prog)
{
    auto [fixedFun, lastStackSlot] = replacePseudosInFunction(prog->getFunction());

    return {
        std::make_shared<Assembly::Program>(fixedFun),
        lastStackSlot,
    };
}
