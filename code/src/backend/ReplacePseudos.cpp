#include <utility>
#include <map>
#include <string>
#include <memory>

#include "Assembly.h"
#include "ReplacePseudos.h"
#include "../Rounding.h"

ReplacementState
ReplacePseudos::createInitState(int startingOffset)
{
    return {
        currOffset : startingOffset,
        offsetMap : {},
    };
}

std::pair<int, ReplacementState>
ReplacePseudos::calculateOffset(const std::string &name, ReplacementState &state)
{
    auto size = _asmSymbolTable.getSize(name);
    auto alignment = _asmSymbolTable.getAlignment(name);
    int newOffset = Rounding::roundAwayFromZero(alignment, state.currOffset - size);

    ReplacementState newState = {
        currOffset : newOffset,
        offsetMap : state.offsetMap,
    };

    newState.offsetMap[name] = newOffset;

    return {
        newOffset,
        newState,
    };
}

ReplaceOperandPair
ReplacePseudos::replaceOperand(const std::shared_ptr<Assembly::Operand> &operand, ReplacementState &state)
{
    if (auto pseudo = std::dynamic_pointer_cast<Assembly::Pseudo>(operand))
    {
        if (_asmSymbolTable.isStatic(pseudo->getName()))
        {
            return {
                std::make_shared<Assembly::Data>(pseudo->getName(), 0),
                state,
            };
        }
        else
        {
            auto it = state.offsetMap.find(pseudo->getName());
            if (it != state.offsetMap.end())
            {
                return {
                    std::make_shared<Assembly::Memory>(std::make_shared<Assembly::Reg>(Assembly::RegName::BP), it->second),
                    state,
                };
            }
            else
            {
                auto [newOffset, newState] = calculateOffset(pseudo->getName(), state);

                return {
                    std::make_shared<Assembly::Memory>(std::make_shared<Assembly::Reg>(Assembly::RegName::BP), newOffset),
                    newState,
                };
            }
        }
    }
    else if (auto pseudoMem = std::dynamic_pointer_cast<Assembly::PseudoMem>(operand))
    {
        if (_asmSymbolTable.isStatic(pseudoMem->getBase()))
        {
            return {
                std::make_shared<Assembly::Data>(pseudoMem->getBase(), pseudoMem->getOffset()),
                state,
            };
        }
        else
        {
            auto it = state.offsetMap.find(pseudoMem->getBase());
            if (it != state.offsetMap.end())
            {
                // We've already assigned this operand a stack slot
                return {
                    std::make_shared<Assembly::Memory>(std::make_shared<Assembly::Reg>(Assembly::RegName::BP), it->second + pseudoMem->getOffset()),
                    state,
                };
            }
            else
            {
                // assign operand name a stack slot, and add its offset to the offset w/tin operand.name to get new operand
                auto [newOffset, newState] = calculateOffset(pseudoMem->getBase(), state);
                return {
                    std::make_shared<Assembly::Memory>(std::make_shared<Assembly::Reg>(Assembly::RegName::BP), newOffset + pseudoMem->getOffset()),
                    newState,
                };
            }
        }
    }
    else
    {
        return {operand, state};
    }
}

ReplaceInstPair
ReplacePseudos::replacePseudosInInstruction(const std::shared_ptr<Assembly::Instruction> &inst, ReplacementState &state)
{
    switch (inst->getType())
    {
    case Assembly::NodeType::Mov:
    {
        auto mov = std::dynamic_pointer_cast<Assembly::Mov>(inst);

        auto [newSrc, state1] = replaceOperand(mov->getSrc(), state);
        auto [newDst, state2] = replaceOperand(mov->getDst(), state1);

        auto newMov = std::make_shared<Assembly::Mov>(mov->getAsmType(), newSrc, newDst);

        return {
            newMov,
            state2,
        };
    }
    case Assembly::NodeType::Movsx:
    {
        auto movsx = std::dynamic_pointer_cast<Assembly::Movsx>(inst);

        auto [newSrc, state1] = replaceOperand(movsx->getSrc(), state);
        auto [newDst, state2] = replaceOperand(movsx->getDst(), state1);

        auto newMovsx = std::make_shared<Assembly::Movsx>(movsx->getSrcType(), movsx->getDstType(), newSrc, newDst);

        return {
            newMovsx,
            state2,
        };
    }
    case Assembly::NodeType::MovZeroExtend:
    {
        auto movzx = std::dynamic_pointer_cast<Assembly::MovZeroExtend>(inst);

        auto [newSrc, state1] = replaceOperand(movzx->getSrc(), state);
        auto [newDst, state2] = replaceOperand(movzx->getDst(), state1);

        auto newMovzx = std::make_shared<Assembly::MovZeroExtend>(movzx->getSrcType(), movzx->getDstType(), newSrc, newDst);

        return {
            newMovzx,
            state2,
        };
    }
    case Assembly::NodeType::Lea:
    {
        auto lea = std::dynamic_pointer_cast<Assembly::Lea>(inst);

        auto [newSrc, state1] = replaceOperand(lea->getSrc(), state);
        auto [newDst, state2] = replaceOperand(lea->getDst(), state1);

        auto newLea = std::make_shared<Assembly::Lea>(newSrc, newDst);

        return {
            newLea,
            state2,
        };
    }
    case Assembly::NodeType::Unary:
    {
        auto unary = std::dynamic_pointer_cast<Assembly::Unary>(inst);

        auto [newDst, state1] = replaceOperand(unary->getOperand(), state);

        auto newUnary = std::make_shared<Assembly::Unary>(unary->getOp(), unary->getAsmType(), newDst);

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

        auto newBinary = std::make_shared<Assembly::Binary>(binary->getOp(), binary->getAsmType(), newSrc, newDst);

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

        auto newCmp = std::make_shared<Assembly::Cmp>(cmp->getAsmType(), newSrc, newDst);

        return {
            newCmp,
            state2,
        };
    }
    case Assembly::NodeType::Idiv:
    {
        auto idiv = std::dynamic_pointer_cast<Assembly::Idiv>(inst);

        auto [newOperand, state1] = replaceOperand(idiv->getOperand(), state);

        auto newIdiv = std::make_shared<Assembly::Idiv>(idiv->getAsmType(), newOperand);

        return {
            newIdiv,
            state1,
        };
    }
    case Assembly::NodeType::Div:
    {
        auto div = std::dynamic_pointer_cast<Assembly::Div>(inst);

        auto [newOperand, state1] = replaceOperand(div->getOperand(), state);

        auto newDiv = std::make_shared<Assembly::Div>(div->getAsmType(), newOperand);

        return {
            newDiv,
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
    case Assembly::NodeType::Push:
    {
        auto push = std::dynamic_pointer_cast<Assembly::Push>(inst);

        auto [newOperand, state1] = replaceOperand(push->getOperand(), state);
        auto newPush = std::make_shared<Assembly::Push>(newOperand);

        return {
            newPush,
            state1,
        };
    }
    case Assembly::NodeType::Cvttsd2si:
    {
        auto cvt = std::dynamic_pointer_cast<Assembly::Cvttsd2si>(inst);

        auto [newSrc, state1] = replaceOperand(cvt->getSrc(), state);
        auto [newDst, state2] = replaceOperand(cvt->getDst(), state1);
        auto newCvt = std::make_shared<Assembly::Cvttsd2si>(cvt->getAsmType(), newSrc, newDst);

        return {
            newCvt,
            state2,
        };
    }
    case Assembly::NodeType::Cvtsi2sd:
    {
        auto cvt = std::dynamic_pointer_cast<Assembly::Cvtsi2sd>(inst);

        auto [newSrc, state1] = replaceOperand(cvt->getSrc(), state);
        auto [newDst, state2] = replaceOperand(cvt->getDst(), state1);
        auto newCvt = std::make_shared<Assembly::Cvtsi2sd>(cvt->getAsmType(), newSrc, newDst);

        return {
            newCvt,
            state2,
        };
    }

    case Assembly::NodeType::Ret:
    case Assembly::NodeType::Cdq:
    case Assembly::NodeType::Label:
    case Assembly::NodeType::JmpCC:
    case Assembly::NodeType::Jmp:
    case Assembly::NodeType::Call:
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

std::shared_ptr<Assembly::TopLevel>
ReplacePseudos::replacePseudosInTopLevel(const std::shared_ptr<Assembly::TopLevel> &topLevel)
{
    if (auto func = std::dynamic_pointer_cast<Assembly::Function>(topLevel))
    {
        // should we stick returns_on_stack in the AST or symbol table?
        int startingOffset = 0;
        if (_asmSymbolTable.returnsOnStack(func->getName()))
        {
            startingOffset = -8;
        }

        auto currState{createInitState(startingOffset)};
        std::vector<std::shared_ptr<Assembly::Instruction>> fixedInstructions{};

        for (auto &inst : func->getInstructions())
        {
            auto [newInst, newState] = replacePseudosInInstruction(inst, currState);
            currState = newState;
            fixedInstructions.push_back(newInst);
        }
        _asmSymbolTable.setBytesRequired(func->getName(), currState.currOffset);

        return std::make_shared<Assembly::Function>(func->getName(), func->isGlobal(), fixedInstructions);
    }
    else
        return topLevel;
}

std::shared_ptr<Assembly::Program>
ReplacePseudos::replacePseudos(const std::shared_ptr<Assembly::Program> &prog)
{
    std::vector<std::shared_ptr<Assembly::TopLevel>> fixedTls{};

    for (auto &tl : prog->getTopLevels())
    {
        auto fixedTl = replacePseudosInTopLevel(tl);
        fixedTls.push_back(fixedTl);
    }

    return std::make_shared<Assembly::Program>(fixedTls);
}
