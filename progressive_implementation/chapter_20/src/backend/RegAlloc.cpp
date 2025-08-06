#include "RegAlloc.h"
#include "Assembly.h"
#include <vector>
#include <set>
#include <map>
#include <string>
#include <memory>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <functional>
#include <limits>
#include <sstream>

#include "../utils/CodeGenPrettyPrint.h"
#include "../Settings.h"
#include "../CFG.h"
#include "../BackwardDataflow.h"
#include "../utils/DisjointSet.h"

auto asmPrettyPrint = CodeGenPrettyPrint();

class RegAllocImpl
{

public:
    struct RegType
    {
        std::string suffix;
        std::vector<Assembly::RegName> allHardregs;
        std::vector<Assembly::RegName> callerSavedRegs;
        std::function<bool(const std::string &)> pseudoIsCurrentType;
        std::function<Settings::RegAllocDebugOptions(Settings &)> debugSettings;
    };

    struct Node
    {
        std::shared_ptr<Assembly::Operand> id;
        std::set<std::shared_ptr<Assembly::Operand>> neighbors;
        double spillCost;
        int color; // -1 means no color assigned
        bool pruned;

        // Default constructor (needed for std::map::operator[])
        Node()
            : id(nullptr), neighbors(), spillCost(0.0), color(-1), pruned(false) {}

        Node(const std::shared_ptr<Assembly::Operand> &id_,
             const std::set<std::shared_ptr<Assembly::Operand>> &neighbors_ = {},
             double spillCost_ = 0.0,
             int color_ = -1,
             bool pruned_ = false)
            : id(id_), neighbors(neighbors_), spillCost(spillCost_), color(color_), pruned(pruned_) {}

        std::string toString() const
        {
            std::ostringstream oss;
            oss << "Node: ";
            try
            {
                oss << RegAllocImpl::showNodeId(id);
            }
            catch (...)
            {
                oss << "[showNodeId ERROR]";
            }
            int typeVal = id ? static_cast<int>(id->getType()) : -1;
            oss << " type=" << typeVal
                << " pruned=" << pruned
                << " color=" << color
                << " spillCost=" << spillCost
                << "\n  Neighbors: ";
            for (const auto &ngh : neighbors)
            {
                try
                {
                    oss << RegAllocImpl::showNodeId(ngh) << " ";
                }
                catch (...)
                {
                    oss << "[showNodeId ERROR] ";
                }
                oss << "(type=" << static_cast<int>(ngh->getType()) << ") ";
            }
            return oss.str();
        }
    };

    using Graph = std::map<std::shared_ptr<Assembly::Operand>, Node>;
    RegAllocImpl(const RegType &regType,
                 const std::map<std::string, std::shared_ptr<Assembly::Pseudo>> &pseudoMapIn,
                 const std::map<Assembly::RegName, std::shared_ptr<Assembly::Reg>> &hardregMapIn)
        : R(regType), pseudoMap(pseudoMapIn), hardRegMap(hardregMapIn) {}

    RegAllocImpl(const RegType &regType) : R(regType) {}

    // extract all operands from an instruction.
    // NOTE: don't need to include implicit operands (e.g. ax/dx for cdq)
    // because we only use this to find pseudos
    std::vector<std::shared_ptr<Assembly::Operand>> getOperands(const std::shared_ptr<Assembly::Instruction> &instr)
    {
        using namespace Assembly;
        switch (instr->getType())
        {
        case NodeType::Mov:
        {
            auto mov = std::dynamic_pointer_cast<Mov>(instr);
            return {mov->getSrc(), mov->getDst()};
        }
        case NodeType::Movsx:
        {
            auto movsx = std::dynamic_pointer_cast<Movsx>(instr);
            return {movsx->getSrc(), movsx->getDst()};
        }
        case NodeType::MovZeroExtend:
        {
            auto zx = std::dynamic_pointer_cast<MovZeroExtend>(instr);
            return {zx->getSrc(), zx->getDst()};
        }
        case NodeType::Lea:
        {
            auto lea = std::dynamic_pointer_cast<Lea>(instr);
            return {lea->getSrc(), lea->getDst()};
        }
        case NodeType::Cvttsd2si:
        {
            auto cvt = std::dynamic_pointer_cast<Cvttsd2si>(instr);
            return {cvt->getSrc(), cvt->getDst()};
        }
        case NodeType::Cvtsi2sd:
        {
            auto cvt = std::dynamic_pointer_cast<Cvtsi2sd>(instr);
            return {cvt->getSrc(), cvt->getDst()};
        }
        case NodeType::Unary:
        {
            auto unary = std::dynamic_pointer_cast<Unary>(instr);
            return {unary->getOperand()};
        }
        case NodeType::Binary:
        {
            auto bin = std::dynamic_pointer_cast<Binary>(instr);
            return {bin->getSrc(), bin->getDst()};
        }
        case NodeType::Cmp:
        {
            auto cmp = std::dynamic_pointer_cast<Cmp>(instr);
            return {cmp->getSrc(), cmp->getDst()};
        }
        case NodeType::Idiv:
        {
            auto idiv = std::dynamic_pointer_cast<Idiv>(instr);
            return {idiv->getOperand()};
        }
        case NodeType::Div:
        {
            auto div = std::dynamic_pointer_cast<Div>(instr);
            return {div->getOperand()};
        }
        case NodeType::SetCC:
        {
            auto setcc = std::dynamic_pointer_cast<SetCC>(instr);
            return {setcc->getOperand()};
        }
        case NodeType::Push:
        {
            auto push = std::dynamic_pointer_cast<Push>(instr);
            return {push->getOperand()};
        }
        case NodeType::Label:
        case NodeType::Call:
        case NodeType::Ret:
        case NodeType::Cdq:
        case NodeType::JmpCC:
        case NodeType::Jmp:
            return {};
        case NodeType::Pop:
            throw std::runtime_error("Internal error: Pop not supported");
        default:
            return {};
        }
    }

    static std::vector<std::shared_ptr<Assembly::Operand>> getOperandsStatic(const std::shared_ptr<Assembly::Instruction> &instr)
    {
        using namespace Assembly;
        switch (instr->getType())
        {
        case NodeType::Mov:
        {
            auto mov = std::dynamic_pointer_cast<Mov>(instr);
            return {mov->getSrc(), mov->getDst()};
        }
        case NodeType::Movsx:
        {
            auto movsx = std::dynamic_pointer_cast<Movsx>(instr);
            return {movsx->getSrc(), movsx->getDst()};
        }
        case NodeType::MovZeroExtend:
        {
            auto zx = std::dynamic_pointer_cast<MovZeroExtend>(instr);
            return {zx->getSrc(), zx->getDst()};
        }
        case NodeType::Lea:
        {
            auto lea = std::dynamic_pointer_cast<Lea>(instr);
            return {lea->getSrc(), lea->getDst()};
        }
        case NodeType::Cvttsd2si:
        {
            auto cvt = std::dynamic_pointer_cast<Cvttsd2si>(instr);
            return {cvt->getSrc(), cvt->getDst()};
        }
        case NodeType::Cvtsi2sd:
        {
            auto cvt = std::dynamic_pointer_cast<Cvtsi2sd>(instr);
            return {cvt->getSrc(), cvt->getDst()};
        }
        case NodeType::Unary:
        {
            auto unary = std::dynamic_pointer_cast<Unary>(instr);
            return {unary->getOperand()};
        }
        case NodeType::Binary:
        {
            auto bin = std::dynamic_pointer_cast<Binary>(instr);
            return {bin->getSrc(), bin->getDst()};
        }
        case NodeType::Cmp:
        {
            auto cmp = std::dynamic_pointer_cast<Cmp>(instr);
            return {cmp->getSrc(), cmp->getDst()};
        }
        case NodeType::Idiv:
        {
            auto idiv = std::dynamic_pointer_cast<Idiv>(instr);
            return {idiv->getOperand()};
        }
        case NodeType::Div:
        {
            auto div = std::dynamic_pointer_cast<Div>(instr);
            return {div->getOperand()};
        }
        case NodeType::SetCC:
        {
            auto setcc = std::dynamic_pointer_cast<SetCC>(instr);
            return {setcc->getOperand()};
        }
        case NodeType::Push:
        {
            auto push = std::dynamic_pointer_cast<Push>(instr);
            return {push->getOperand()};
        }
        case NodeType::Label:
        case NodeType::Call:
        case NodeType::Ret:
        case NodeType::Cdq:
        case NodeType::JmpCC:
        case NodeType::Jmp:
            return {};
        case NodeType::Pop:
            throw std::runtime_error("Internal error: Pop not supported");
        default:
            return {};
        }
    }

    // map functon f over all the operands in an instruction
    std::shared_ptr<Assembly::Instruction> replaceOps(
        const std::function<std::shared_ptr<Assembly::Operand>(const std::shared_ptr<Assembly::Operand> &)> &f,
        const std::shared_ptr<Assembly::Instruction> &instr)
    {
        using namespace Assembly;
        switch (instr->getType())
        {
        case NodeType::Mov:
        {
            auto mov = std::dynamic_pointer_cast<Mov>(instr);
            return std::make_shared<Mov>(mov->getAsmType(), f(mov->getSrc()), f(mov->getDst()));
        }
        case NodeType::Movsx:
        {
            auto movsx = std::dynamic_pointer_cast<Movsx>(instr);
            return std::make_shared<Movsx>(movsx->getSrcType(), movsx->getDstType(), f(movsx->getSrc()), f(movsx->getDst()));
        }
        case NodeType::MovZeroExtend:
        {
            auto zx = std::dynamic_pointer_cast<MovZeroExtend>(instr);
            return std::make_shared<MovZeroExtend>(zx->getSrcType(), zx->getDstType(), f(zx->getSrc()), f(zx->getDst()));
        }
        case NodeType::Lea:
        {
            auto lea = std::dynamic_pointer_cast<Lea>(instr);
            return std::make_shared<Lea>(f(lea->getSrc()), f(lea->getDst()));
        }
        case NodeType::Cvttsd2si:
        {
            auto cvt = std::dynamic_pointer_cast<Cvttsd2si>(instr);
            return std::make_shared<Cvttsd2si>(cvt->getAsmType(), f(cvt->getSrc()), f(cvt->getDst()));
        }
        case NodeType::Cvtsi2sd:
        {
            auto cvt = std::dynamic_pointer_cast<Cvtsi2sd>(instr);
            return std::make_shared<Cvtsi2sd>(cvt->getAsmType(), f(cvt->getSrc()), f(cvt->getDst()));
        }
        case NodeType::Unary:
        {
            auto unary = std::dynamic_pointer_cast<Unary>(instr);
            return std::make_shared<Unary>(unary->getOp(), unary->getAsmType(), f(unary->getOperand()));
        }
        case NodeType::Binary:
        {
            auto bin = std::dynamic_pointer_cast<Binary>(instr);
            return std::make_shared<Binary>(bin->getOp(), bin->getAsmType(), f(bin->getSrc()), f(bin->getDst()));
        }
        case NodeType::Cmp:
        {
            auto cmp = std::dynamic_pointer_cast<Cmp>(instr);
            return std::make_shared<Cmp>(cmp->getAsmType(), f(cmp->getSrc()), f(cmp->getDst()));
        }
        case NodeType::Idiv:
        {
            auto idiv = std::dynamic_pointer_cast<Idiv>(instr);
            return std::make_shared<Idiv>(idiv->getAsmType(), f(idiv->getOperand()));
        }
        case NodeType::Div:
        {
            auto div = std::dynamic_pointer_cast<Div>(instr);
            return std::make_shared<Div>(div->getAsmType(), f(div->getOperand()));
        }
        case NodeType::SetCC:
        {
            auto setcc = std::dynamic_pointer_cast<SetCC>(instr);
            return std::make_shared<SetCC>(setcc->getCondCode(), f(setcc->getOperand()));
        }
        case NodeType::Push:
        {
            auto push = std::dynamic_pointer_cast<Push>(instr);
            return std::make_shared<Push>(f(push->getOperand()));
        }
        case NodeType::Label:
        case NodeType::Call:
        case NodeType::Ret:
        case NodeType::Cdq:
        case NodeType::Jmp:
        case NodeType::JmpCC:
            return instr;
        case NodeType::Pop:
            throw std::runtime_error("Shouldn't use this yet");
        default:
            return instr;
        }
    }

    std::vector<std::shared_ptr<Assembly::Instruction>> cleanupMovs(
        const std::vector<std::shared_ptr<Assembly::Instruction>> &instrs)
    {
        using namespace Assembly;
        std::vector<std::shared_ptr<Assembly::Instruction>> result;
        for (const auto &instr : instrs)
        {
            if (instr->getType() == NodeType::Mov)
            {
                auto mov = std::dynamic_pointer_cast<Assembly::Mov>(instr);
                if (*mov->getSrc() == *mov->getDst())
                    continue;
            }
            result.push_back(instr);
        }
        return result;
    }

    static std::string showNodeId(const std::shared_ptr<Assembly::Operand> op)
    {
        using namespace Assembly;
        std::string s;
        switch (op->getType())
        {
        case NodeType::Reg:
        {
            auto r = std::dynamic_pointer_cast<Assembly::Reg>(op);
            s = asmPrettyPrint.showRegName(r->getName());
            break;
        }
        case NodeType::Pseudo:
        {
            auto p = std::dynamic_pointer_cast<Assembly::Pseudo>(op);
            s = p->getName();
            break;
        }
        default:
            throw std::runtime_error("Internal error: malformed interference graph");
        }
        std::replace(s.begin(), s.end(), '.', '_');
        return s;
    }

    int getK()
    {
        return static_cast<int>(R.allHardregs.size());
    }

    Node &getNodeById(Graph &g, const std::shared_ptr<Assembly::Operand> &id)
    {
        auto it = g.find(id);
        if (it == g.end())
        {
            throw std::runtime_error("getNodeById: node not found in interference graph");
        }
        return it->second;
    }

    void addEdge(Graph &g, const std::shared_ptr<Assembly::Operand> &a, const std::shared_ptr<Assembly::Operand> &b)
    {
        auto iterA = g.find(a);
        auto iterB = g.find(b);
        if (iterA != g.end() && iterB != g.end())
        {
            iterA->second.neighbors.insert(b);
            iterB->second.neighbors.insert(a);
        }
    }

    // utility function

    void regsUsedAndWritten(
        const std::shared_ptr<Assembly::Instruction> &instr,
        std::set<std::shared_ptr<Assembly::Operand>> &used,
        std::set<std::shared_ptr<Assembly::Operand>> &written,
        AssemblySymbols::AsmSymbolTable &asmSymbol)
    {
        using namespace Assembly;
        used.clear();
        written.clear();

        // Helper lambdas for operand analysis
        auto regsUsedToRead = [](const std::shared_ptr<Operand> &opr) -> std::vector<std::shared_ptr<Operand>>
        {
            switch (opr->getType())
            {
            case NodeType::Pseudo:
            case NodeType::Reg:
                return {opr};
            case NodeType::Memory:
            {
                auto mem = std::dynamic_pointer_cast<Assembly::Memory>(opr);
                std::vector<std::shared_ptr<Operand>> v;
                v.push_back(mem->getReg());
                return v;
            }
            case NodeType::Indexed:
            {
                auto idx = std::dynamic_pointer_cast<Assembly::Indexed>(opr);
                std::vector<std::shared_ptr<Operand>> v;
                v.push_back(idx->getBase());
                v.push_back(idx->getIndex());
                return v;
            }
            case NodeType::Imm:
            case NodeType::Data:
            case NodeType::PseudoMem:
                return {};
            default:
                return {};
            }
        };

        auto regsUsedToUpdate = [](const std::shared_ptr<Operand> &opr) -> std::pair<std::vector<std::shared_ptr<Operand>>, std::vector<std::shared_ptr<Operand>>>
        {
            switch (opr->getType())
            {
            case NodeType::Pseudo:
            case NodeType::Reg:
                return {{}, {opr}};
            case NodeType::Memory:
            {
                auto mem = std::dynamic_pointer_cast<Assembly::Memory>(opr);
                std::vector<std::shared_ptr<Operand>> v1, v2;
                v1.push_back(mem->getReg());
                return {v1, v2};
            }
            case NodeType::Indexed:
            {
                auto idx = std::dynamic_pointer_cast<Assembly::Indexed>(opr);
                std::vector<std::shared_ptr<Operand>> v1, v2;
                v1.push_back(idx->getBase());
                v1.push_back(idx->getIndex());
                return {v1, v2};
            }
            case NodeType::Imm:
            case NodeType::Data:
            case NodeType::PseudoMem:
                return {{}, {}};
            default:
                return {{}, {}};
            }
        };

        // Determine opsUsed and opsWritten for the instruction
        std::vector<std::shared_ptr<Operand>> opsUsed, opsWritten;
        switch (instr->getType())
        {
        case NodeType::Mov:
        {
            auto mov = std::dynamic_pointer_cast<Mov>(instr);
            opsUsed = {mov->getSrc()};
            opsWritten = {mov->getDst()};
            break;
        }
        case NodeType::MovZeroExtend:
        {
            auto zx = std::dynamic_pointer_cast<MovZeroExtend>(instr);
            opsUsed = {zx->getSrc()};
            opsWritten = {zx->getDst()};
            break;
        }
        case NodeType::Movsx:
        {
            auto sx = std::dynamic_pointer_cast<Movsx>(instr);
            opsUsed = {sx->getSrc()};
            opsWritten = {sx->getDst()};
            break;
        }
        case NodeType::Cvtsi2sd:
        {
            auto cvt = std::dynamic_pointer_cast<Cvtsi2sd>(instr);
            opsUsed = {cvt->getSrc()};
            opsWritten = {cvt->getDst()};
            break;
        }
        case NodeType::Cvttsd2si:
        {
            auto cvt = std::dynamic_pointer_cast<Cvttsd2si>(instr);
            opsUsed = {cvt->getSrc()};
            opsWritten = {cvt->getDst()};
            break;
        }
        case NodeType::Binary:
        {
            auto bin = std::dynamic_pointer_cast<Binary>(instr);
            opsUsed = {bin->getSrc(), bin->getDst()};
            opsWritten = {bin->getDst()};
            break;
        }
        case NodeType::Unary:
        {
            auto unary = std::dynamic_pointer_cast<Unary>(instr);
            opsUsed = {unary->getOperand()};
            opsWritten = {unary->getOperand()};
            break;
        }
        case NodeType::Cmp:
        {
            auto cmp = std::dynamic_pointer_cast<Cmp>(instr);
            opsUsed = {cmp->getSrc(), cmp->getDst()};
            break;
        }
        case NodeType::SetCC:
        {
            auto setcc = std::dynamic_pointer_cast<SetCC>(instr);
            opsWritten = {setcc->getOperand()};
            break;
        }
        case NodeType::Push:
        {
            auto push = std::dynamic_pointer_cast<Push>(instr);
            opsUsed = {push->getOperand()};
            break;
        }
        case NodeType::Idiv:
        {
            auto idiv = std::dynamic_pointer_cast<Idiv>(instr);
            opsUsed = {idiv->getOperand(), std::make_shared<Reg>(RegName::AX), std::make_shared<Reg>(RegName::DX)};
            opsWritten = {std::make_shared<Reg>(RegName::AX), std::make_shared<Reg>(RegName::DX)};
            break;
        }
        case NodeType::Div:
        {
            auto div = std::dynamic_pointer_cast<Div>(instr);
            opsUsed = {div->getOperand(), std::make_shared<Reg>(RegName::AX), std::make_shared<Reg>(RegName::DX)};
            opsWritten = {std::make_shared<Reg>(RegName::AX), std::make_shared<Reg>(RegName::DX)};
            break;
        }
        case NodeType::Cdq:
        {
            opsUsed = {std::make_shared<Reg>(RegName::AX)};
            opsWritten = {std::make_shared<Reg>(RegName::DX)};
            break;
        }
        case NodeType::Call:
        {
            auto call = std::dynamic_pointer_cast<Call>(instr);
            std::vector<Assembly::RegName> paramRegs = asmSymbol.paramRegsUsed(call->getFnName());
            for (const auto &r : paramRegs)
            {
                if (std::find(R.allHardregs.begin(), R.allHardregs.end(), r) != R.allHardregs.end())
                {
                    opsUsed.push_back(std::make_shared<Reg>(r));
                }
            }
            for (const auto &reg : R.callerSavedRegs)
            {
                opsWritten.push_back(std::make_shared<Reg>(reg));
            }
            break;
        }
        case NodeType::Lea:
        {
            auto lea = std::dynamic_pointer_cast<Lea>(instr);
            opsUsed = {lea->getSrc()};
            opsWritten = {lea->getDst()};
            break;
        }
        case NodeType::Jmp:
        case NodeType::JmpCC:
        case NodeType::Label:
        case NodeType::Ret:
            break;
        case NodeType::Pop:
            throw std::runtime_error("Internal error: Pop not supported");
        default:
            break;
        }

        // regsRead1: flatten opsUsed via regsUsedToRead
        std::vector<std::shared_ptr<Operand>> regsRead1;
        for (const auto &op : opsUsed)
        {
            auto v = regsUsedToRead(op);
            regsRead1.insert(regsRead1.end(), v.begin(), v.end());
        }

        // regsRead2, regsWritten: split opsWritten via regsUsedToUpdate
        std::vector<std::shared_ptr<Operand>> regsRead2;
        std::vector<std::shared_ptr<Operand>> regsWrittenVec;
        for (const auto &op : opsWritten)
        {
            auto p = regsUsedToUpdate(op);
            regsRead2.insert(regsRead2.end(), p.first.begin(), p.first.end());
            regsWrittenVec.insert(regsWrittenVec.end(), p.second.begin(), p.second.end());
        }

        // Final sets
        used.insert(regsRead1.begin(), regsRead1.end());
        used.insert(regsRead2.begin(), regsRead2.end());
        written.insert(regsWrittenVec.begin(), regsWrittenVec.end());
    }

    void removeEdge(Graph &g, const std::shared_ptr<Assembly::Operand> &a, const std::shared_ptr<Assembly::Operand> &b)
    {
        Node &nd1 = getNodeById(g, a);
        Node &nd2 = getNodeById(g, b);
        nd1.neighbors.erase(b);
        nd2.neighbors.erase(a);
    }

    int degree(Graph &g, const std::shared_ptr<Assembly::Operand> &id)
    {
        Node &nd = getNodeById(g, id);
        return static_cast<int>(nd.neighbors.size());
    }

    bool areNeighbors(Graph &g, const std::shared_ptr<Assembly::Operand> &a, const std::shared_ptr<Assembly::Operand> &b)
    {
        Node &nd1 = getNodeById(g, a);
        return nd1.neighbors.find(b) != nd1.neighbors.end();
    }

    Graph mkBaseGraph()
    {
        Graph g{};
        // Add each hardreg as a node with canonical neighbors
        for (const auto &r : R.allHardregs)
        {
            auto reg = hardRegMap[r];
            std::set<std::shared_ptr<Assembly::Operand>> neighbors;
            for (const auto &r2 : R.allHardregs)
            {
                if (r2 != r)
                {
                    neighbors.insert(hardRegMap[r2]);
                }
            }
            Node node{
                reg,
                neighbors,
                std::numeric_limits<double>::infinity(),
                -1,
                false};
            g.insert_or_assign(reg, node);
        }
        return g;
    }

    // Convert list of operands to list of pseudoregisters - note that we don't need to include hardregs because they're already in the base graph
    // and some operands e.g. constants, pseudos with static storage duration, and regs of the wrong type) should be excluded
    std::vector<Node> getPseudoNodes(
        const std::set<std::string> &aliasedPseudos,
        const std::vector<std::shared_ptr<Assembly::Instruction>> &instrs,
        AssemblySymbols::AsmSymbolTable &asmSymbol)
    {
        using namespace Assembly;
        std::vector<Node> result;

        // Helper: check if operand is a pseudo we want
        auto isPseudo = [&](const std::shared_ptr<Operand> &op) -> bool
        {
            if (op->getType() == NodeType::Pseudo)
            {
                auto p = std::dynamic_pointer_cast<Assembly::Pseudo>(op);
                return (R.pseudoIsCurrentType(p->getName()) && !asmSymbol.isStatic(p->getName()) && aliasedPseudos.find(p->getName()) == aliasedPseudos.end());
            }
            return false;
        };

        for (const auto &instr : instrs)
        {
            auto ops = getOperands(instr);
            for (const auto &op : ops)
            {
                if (isPseudo(op))
                {
                    auto pseudo = std::dynamic_pointer_cast<Assembly::Pseudo>(op);
                    auto canonPseudo = canonicalize(pseudo);
                    result.push_back(Node{canonPseudo, {}, 0.0, -1, false});
                }
            }
        }
        return result;
    }

    void addPseudoNodes(
        Graph &g,
        const std::set<std::string> &aliasedPseudos,
        const std::vector<std::shared_ptr<Assembly::Instruction>> &instrs,
        AssemblySymbols::AsmSymbolTable &asmSymbol)
    {
        auto nds = getPseudoNodes(aliasedPseudos, instrs, asmSymbol);
        for (const auto &nd : nds)
        {
            g.insert_or_assign(nd.id, nd);
        }
    }

    void addEdges(
        cfg::Graph<std::set<std::shared_ptr<Assembly::Operand>>, Assembly::Instruction> &livenessCfg,
        Graph &g,
        AssemblySymbols::AsmSymbolTable &asmSymbol)
    {
        // Flatten all instrs in all basic blocks
        std::vector<std::pair<std::set<std::shared_ptr<Assembly::Operand>>, std::shared_ptr<Assembly::Instruction>>> allInsts;
        for (const auto &bbPair : livenessCfg.basicBlocks)
        {
            const auto &blk = bbPair.second;
            for (const auto &instPair : blk.instructions)
            {
                allInsts.push_back(instPair);
            }
        }

        for (const auto &instPair : allInsts)
        {
            const std::set<std::shared_ptr<Assembly::Operand>> &liveAfterInstr = instPair.first;
            const std::shared_ptr<Assembly::Instruction> &instr = instPair.second;

            // Get updatedRegs (regsWritten)
            std::set<std::shared_ptr<Assembly::Operand>> unused, updatedRegs;
            regsUsedAndWritten(instr, unused, updatedRegs, asmSymbol);

            // For each l in liveAfterInstr
            for (const auto &lRaw : liveAfterInstr)
            {
                auto l = canonicalize(lRaw);
                // If instr is Mov and src == l, skip
                bool skip = false;
                if (instr->getType() == Assembly::NodeType::Mov)
                {
                    auto mov = std::dynamic_pointer_cast<Assembly::Mov>(instr);
                    if (*mov->getSrc() == *l)
                    {
                        skip = true;
                    }
                }
                if (skip)
                    continue;

                // For each u in updatedRegs
                for (const auto &uRaw : updatedRegs)
                {
                    auto u = canonicalize(uRaw);
                    if (u != l && g.count(l) && g.count(u))
                    {
                        addEdge(g, l, u);
                    }
                }
            }
        }
    }

    Graph buildInterferenceGraph(
        const std::string &fnName,
        const std::set<std::string> &aliasedPseudos,
        const std::vector<std::shared_ptr<Assembly::Instruction>> &instrs,
        AssemblySymbols::AsmSymbolTable &asmSymbol,
        Settings &settings);

    void addSpillCosts(
        Graph &g,
        const std::vector<std::shared_ptr<Assembly::Instruction>> &instrs)
    {
        // given map from pseudo names to counts, incremene map entry for pseudo, or set to 1 if not already present
        // get list of all operands in the function, filter out all but pseudoregs
        // create map from pseudoregs to counts - note that this may include pseudos that aren't in the interference graph, we'll ignore them
        // set each node's spill cost to the count from countMap

        // Step 1: Count pseudo register usage in instrs
        std::map<std::string, int> pseudoCounts;
        for (const auto &instr : instrs)
        {
            auto ops = getOperands(instr);
            for (const auto &op : ops)
            {
                if (op->getType() == Assembly::NodeType::Pseudo)
                {
                    auto p = std::dynamic_pointer_cast<Assembly::Pseudo>(op);
                    pseudoCounts[p->getName()]++;
                }
            }
        }

        // Step 2: Set spillCost for each pseudo node in the graph
        for (auto &[id, node] : g)
        {
            if (id->getType() == Assembly::NodeType::Pseudo)
            {
                auto p = std::dynamic_pointer_cast<Assembly::Pseudo>(id);
                auto it = pseudoCounts.find(p->getName());
                node.spillCost = (it != pseudoCounts.end()) ? static_cast<double>(it->second) : 0.0;
            }
        }
    }

    // George's coalescing test: returns true if pseudo can be safely coalesced with hardreg
    bool georgeTest(Graph &g, const std::shared_ptr<Assembly::Operand> &hardreg, const std::shared_ptr<Assembly::Operand> &pseudo)
    {
        // Get neighbors of the pseudo register
        Node &pseudoNode = getNodeById(g, pseudo);
        int k = getK();
        for (const auto &neighborId : pseudoNode.neighbors)
        {
            // A neighbor is OK if it already interferes with hardreg, or has insignificant degree
            if (!(areNeighbors(g, neighborId, hardreg) || degree(g, neighborId) < k))
                return false;
        }
        return true;
    }

    // Briggs' coalescing test: returns true if x and y can be conservatively coalesced
    bool briggsTest(Graph &g, const std::shared_ptr<Assembly::Operand> &x, const std::shared_ptr<Assembly::Operand> &y)
    {
        Node &xNode = getNodeById(g, x);
        Node &yNode = getNodeById(g, y);
        // Union of neighbors
        std::set<std::shared_ptr<Assembly::Operand>> neighbors = xNode.neighbors;
        neighbors.insert(yNode.neighbors.begin(), yNode.neighbors.end());
        int k = getK();
        int significantNeighborCount = 0;
        for (const auto &neighborId : neighbors)
        {
            int deg = degree(g, neighborId);
            int adjustedDeg = deg;
            if (areNeighbors(g, x, neighborId) && areNeighbors(g, y, neighborId))
                adjustedDeg -= 1;
            if (adjustedDeg >= k)
                significantNeighborCount++;
        }
        return significantNeighborCount < k;
    }

    // Conservative coalescing test: returns true if src and dst can be safely coalesced
    bool conservativeCoalescable(Graph &g, const std::shared_ptr<Assembly::Operand> &src, const std::shared_ptr<Assembly::Operand> &dst)
    {
        if (briggsTest(g, src, dst))
            return true;
        // If either is a hardreg, use George's test
        if (src->getType() == Assembly::NodeType::Reg)
            return georgeTest(g, src, dst);
        if (dst->getType() == Assembly::NodeType::Reg)
            return georgeTest(g, dst, src);
        return false;
    }

    // Debug output for coalescing
    void printCoalesceMsg(Settings &settings, const std::shared_ptr<Assembly::Operand> &src, const std::shared_ptr<Assembly::Operand> &dst)
    {
        auto dbg = R.debugSettings(settings);
        // If debugMsg is enabled, print coalescing info
        if (dbg.debugMsg)
        {
            std::cout << "Coalescing " << showNodeId(src) << " into " << showNodeId(dst) << std::endl;
        }
    }

    // Merge neighbors of toMerge into toKeep, update edges, print debug, and remove toMerge from graph
    void updateGraph(
        Graph &g,
        Settings &settings,
        const std::shared_ptr<Assembly::Operand> &toMerge,
        const std::shared_ptr<Assembly::Operand> &toKeep)
    {
        // Print debug message
        printCoalesceMsg(settings, toMerge, toKeep);

        // Get neighbors of toMerge
        Node &mergeNode = getNodeById(g, toMerge);
        std::set<std::shared_ptr<Assembly::Operand>> neighbors = mergeNode.neighbors;

        // For each neighbor, add edge to toKeep and remove edge to toMerge
        for (const auto &nghborId : neighbors)
        {
            addEdge(g, nghborId, toKeep);
            removeEdge(g, nghborId, toMerge);
        }

        // Remove toMerge from the graph
        g.erase(toMerge);
    }

    // Coalesce pseudo registers in the interference graph based on move instructions
    // Returns the DisjointSet object for further queries
    DisjointSet<std::shared_ptr<Assembly::Operand>> coalesce(
        Graph &g,
        const std::vector<std::shared_ptr<Assembly::Instruction>> &instrs,
        Settings &settings)
    {
        using namespace Assembly;
        DisjointSet<std::shared_ptr<Assembly::Operand>> regSet;

        auto dbg = R.debugSettings(settings);
        if (dbg.debugMsg)
            std::cout << "Coalescing round\n=============" << std::endl;

        for (const auto &instr : instrs)
        {
            if (instr->getType() == NodeType::Mov)
            {
                auto mov = std::dynamic_pointer_cast<Mov>(instr);
                auto src = canonicalize(mov->getSrc());
                auto dst = canonicalize(mov->getDst());
                auto srcRep = canonicalize(regSet.find(src));
                auto dstRep = canonicalize(regSet.find(dst));
                if (g.count(srcRep) && g.count(dstRep) && srcRep != dstRep &&
                    !areNeighbors(g, srcRep, dstRep) && conservativeCoalescable(g, srcRep, dstRep))
                {
                    if (srcRep->getType() == NodeType::Reg)
                    {
                        updateGraph(g, settings, dstRep, srcRep);
                        regSet.unite(dstRep, srcRep);
                    }
                    else
                    {
                        updateGraph(g, settings, srcRep, dstRep);
                        regSet.unite(srcRep, dstRep);
                    }
                }
            }
        }

        return regSet;
    }

    // Rewrite instructions using the coalesced register mapping, removing redundant moves
    std::vector<std::shared_ptr<Assembly::Instruction>> rewriteCoalesced(
        const std::vector<std::shared_ptr<Assembly::Instruction>> &instrs,
        const DisjointSet<std::shared_ptr<Assembly::Operand>> &coalescedRegs)
    {
        using namespace Assembly;
        auto findRep = [&](const std::shared_ptr<Assembly::Operand> &op) -> std::shared_ptr<Assembly::Operand>
        {
            return coalescedRegs.find(op);
        };

        std::vector<std::shared_ptr<Assembly::Instruction>> result;
        for (const auto &instr : instrs)
        {
            if (instr->getType() == NodeType::Mov)
            {
                auto mov = std::dynamic_pointer_cast<Mov>(instr);
                auto newSrc = findRep(canonicalize(mov->getSrc()));
                auto newDst = findRep(canonicalize(mov->getDst()));

                if (*newSrc == *newDst)
                    continue; // Remove redundant move
                result.push_back(std::make_shared<Mov>(mov->getAsmType(), newSrc, newDst));
            }
            else
            {
                result.push_back(replaceOps(findRep, instr));
            }
        }
        return result;
    }

    void
    colorGraph(Graph &g)
    {
        // Recursively prune nodes and assign colors
        // k = number of hardregs
        int k = getK();
        // Stack to record prune order
        std::vector<std::shared_ptr<Assembly::Operand>> pruneStack;
        while (true)
        {
            // Collect remaining (unpruned) nodes
            std::vector<std::shared_ptr<Assembly::Operand>> remaining;
            for (const auto &[id, node] : g)
            {
                if (!node.pruned)
                    remaining.push_back(id);
            }
            if (remaining.empty())
            {
                // We've pruned the whole graph, so we're done
                break;
            }

            // Helper: check if a node is pruned
            auto notPruned = [&](const std::shared_ptr<Assembly::Operand> &id)
            {
                auto it = g.find(id);
                return it != g.end() && !it->second.pruned;
            };

            // Helper: degree of a node (number of unpruned neighbors)
            auto degree = [&](const Node &nd)
            {
                int deg = 0;
                for (const auto &ngh : nd.neighbors)
                {
                    if (notPruned(ngh))
                        deg++;
                }
                return deg;
            };

            // Find next node to prune: first with degree < k, else spill candidate
            std::optional<std::shared_ptr<Assembly::Operand>> nextId;
            bool foundLowDegree = false;
            for (const auto &id : remaining)
            {
                auto it = g.find(id);
                if (it != g.end() && degree(it->second) < k)
                {
                    nextId = id;
                    foundLowDegree = true;
                    break;
                }
            }
            if (!foundLowDegree)
            {
                // Need to choose a spill candidate!
                // Choose node with minimal spill metric
                double minMetric = std::numeric_limits<double>::infinity();
                for (const auto &id : remaining)
                {
                    auto it = g.find(id);
                    if (it == g.end())
                        continue;
                    const Node &nd = it->second;
                    int deg = degree(nd);
                    double metric = deg > 0 ? nd.spillCost / deg : nd.spillCost;
                    if (metric < minMetric)
                    {
                        minMetric = metric;
                        nextId = id;
                    }
                }
            }

            // Prune next node and record prune order
            if (nextId.has_value())
            {
                auto it = g.find(nextId.value());
                if (it != g.end())
                {
                    it->second.pruned = true;
                    pruneStack.push_back(nextId.value());
                }
            }
            else
            {
                break;
            }
        }

        // Now assign colors in reverse order of pruning
        // First, collect all colors
        std::vector<int> allColors;
        for (int i = 0; i < k; ++i)
            allColors.push_back(i);

        // Unprune all nodes for coloring
        for (auto &[id, node] : g)
            node.pruned = false;

        // Assign colors in reverse prune order
        for (auto it = pruneStack.rbegin(); it != pruneStack.rend(); ++it)
        {
            auto &node = g[*it];
            // Find available colors (not used by neighbors)
            std::vector<int> available = allColors;
            for (const auto &ngh : node.neighbors)
            {
                auto nghborIter = g.find(ngh);
                if (nghborIter != g.end() && nghborIter->second.color != -1)
                {
                    available.erase(std::remove(available.begin(), available.end(), nghborIter->second.color), available.end());
                }
            }
            if (available.empty())
            {
                // No available colors, leave uncolored
                continue;
            }
            // If this is a callee-saved reg, give it the highest-numbered color; otherwise lowest
            int chosenColor = -1;
            if ((*it)->getType() == Assembly::NodeType::Reg)
            {
                auto r = std::dynamic_pointer_cast<Assembly::Reg>(*it);
                auto csIter = std::find(R.callerSavedRegs.begin(), R.callerSavedRegs.end(), r->getName());
                if (csIter == R.callerSavedRegs.end())
                {
                    chosenColor = *std::max_element(available.begin(), available.end());
                }
                else
                {
                    chosenColor = *std::min_element(available.begin(), available.end());
                }
            }
            else
            {
                chosenColor = *std::min_element(available.begin(), available.end());
            }
            node.color = chosenColor;
        }
    }

    std::map<std::string, std::shared_ptr<Assembly::Reg>> makeRegisterMap(
        Graph &g,
        const std::string &fnName,
        AssemblySymbols::AsmSymbolTable &asmSymbol)
    {
        // Step 1: Build map from color indices to hardregs in a fixed order
        std::map<int, std::shared_ptr<Assembly::Reg>> colorToReg;
        int k = getK();
        for (int i = 0; i < k; ++i)
        {
            colorToReg.insert_or_assign(i, std::make_shared<Assembly::Reg>(R.allHardregs[i]));
        }

        // Step 2: Build map from pseudoregisters to hardregs, and track used callee-saved regs
        std::set<Assembly::RegName> usedCalleeSaved;
        std::map<std::string, std::shared_ptr<Assembly::Reg>> pseudoToRegMap;
        for (const auto &[id, node] : g)
        {
            if (id->getType() == Assembly::NodeType::Pseudo && node.color != -1)
            {
                auto p = std::dynamic_pointer_cast<Assembly::Pseudo>(id);
                auto it = colorToReg.find(node.color);
                if (it != colorToReg.end())
                {
                    auto hardreg = it->second;
                    // Track callee-saved usage
                    auto csIter = std::find(R.callerSavedRegs.begin(), R.callerSavedRegs.end(), hardreg->getName());
                    if (csIter == R.callerSavedRegs.end())
                    {
                        usedCalleeSaved.insert(hardreg->getName());
                    }
                    pseudoToRegMap.insert({p->getName(), hardreg});
                }
            }
        }

        // Record used callee-saved registers for this function
        asmSymbol.addCalleeSavedRegsUsed(fnName, usedCalleeSaved);
        return pseudoToRegMap;
    }

    std::vector<std::shared_ptr<Assembly::Instruction>> replacePseudoRegs(
        const std::vector<std::shared_ptr<Assembly::Instruction>> &instrs,
        const std::map<std::string, std::shared_ptr<Assembly::Reg>> &pseudoToRegMap)
    {
        // Replace pseudoregister w/ corresponding hardreg. If operand isn't a pseudo or isn't colored, don't replace it.

        auto f = [&](const std::shared_ptr<Assembly::Operand> &op) -> std::shared_ptr<Assembly::Operand>
        {
            using namespace Assembly;
            if (!op)
                return op;
            if (op->getType() == NodeType::Pseudo)
            {
                auto p = std::dynamic_pointer_cast<Pseudo>(op);
                if (p)
                {
                    auto it = pseudoToRegMap.find(p->getName());
                    if (it != pseudoToRegMap.end())
                    {
                        return it->second;
                    }
                }
            }
            return op;
        };

        std::vector<std::shared_ptr<Assembly::Instruction>> replaced;
        for (const auto &instr : instrs)
        {
            replaced.push_back(replaceOps(f, instr));
        }
        return cleanupMovs(replaced);
    }

    std::vector<std::shared_ptr<Assembly::Instruction>> allocate(
        const std::string &fnName,
        const std::set<std::string> &aliasedPseudos,
        const std::vector<std::shared_ptr<Assembly::Instruction>> &instrs,
        AssemblySymbols::AsmSymbolTable &asmSymbol,
        Settings &settings);

    // GPType: General-purpose register allocation type
    struct GPType
    {
        static constexpr const char *suffix = "gp";
        static std::vector<Assembly::RegName> allHardregs()
        {
            using Assembly::RegName;
            return {RegName::AX, RegName::BX, RegName::CX, RegName::DX,
                    RegName::DI, RegName::SI, RegName::R8, RegName::R9,
                    RegName::R12, RegName::R13, RegName::R14, RegName::R15};
        }
        static std::vector<Assembly::RegName> callerSavedRegs()
        {
            using Assembly::RegName;
            return {RegName::AX, RegName::CX, RegName::DX,
                    RegName::DI, RegName::SI, RegName::R8, RegName::R9};
        }
        static bool pseudoIsCurrentType(const std::string &p, AssemblySymbols::AsmSymbolTable &asmSymbol)
        {
            return !Assembly::isAsmDouble(asmSymbol.getType(p));
        }
        static Settings::RegAllocDebugOptions debugSettings(Settings &settings)
        {
            return settings.getRegAllocDebugOptions();
        }
    };

    // XMMType: Floating-point register allocation type
    struct XMMType
    {
        static constexpr const char *suffix = "xmm";
        static std::vector<Assembly::RegName> allHardregs()
        {
            using Assembly::RegName;
            return {RegName::XMM0, RegName::XMM1, RegName::XMM2, RegName::XMM3,
                    RegName::XMM4, RegName::XMM5, RegName::XMM6, RegName::XMM7,
                    RegName::XMM8, RegName::XMM9, RegName::XMM10, RegName::XMM11,
                    RegName::XMM12, RegName::XMM13};
        }
        static std::vector<Assembly::RegName> callerSavedRegs()
        {
            return allHardregs();
        }
        static bool pseudoIsCurrentType(const std::string &p, AssemblySymbols::AsmSymbolTable &asmSymbol)
        {
            return Assembly::isAsmDouble(asmSymbol.getType(p));
        }
        static Settings::RegAllocDebugOptions debugSettings(Settings &settings)
        {
            return settings.getRegAllocDebugOptions();
        }
    };

    std::shared_ptr<Assembly::Operand> canonicalize(const std::shared_ptr<Assembly::Operand> &op) const
    {
        if (!op)
            return op;
        if (op->getType() == Assembly::NodeType::Pseudo)
        {
            auto p = std::dynamic_pointer_cast<Assembly::Pseudo>(op);
            auto it = pseudoMap.find(p->getName());
            if (it != pseudoMap.end())
            {
                return it->second;
            }
        }
        else if (op->getType() == Assembly::NodeType::Reg)
        {
            auto r = std::dynamic_pointer_cast<Assembly::Reg>(op);
            auto it = hardRegMap.find(r->getName());
            if (it != hardRegMap.end())
                return it->second;
        }
        return op;
    }

private:
    RegType R;
    // Canonical map for pseudos
    std::map<std::string, std::shared_ptr<Assembly::Pseudo>> pseudoMap;
    // Canonical map for hardregs
    std::map<Assembly::RegName, std::shared_ptr<Assembly::Reg>> hardRegMap;
};

class LivenessAnalysis
{
public:
    // Canonical maps for operands
    const std::map<std::string, std::shared_ptr<Assembly::Pseudo>> *pseudoMapPtr = nullptr;
    const std::map<Assembly::RegName, std::shared_ptr<Assembly::Reg>> *hardregMapPtr = nullptr;

    LivenessAnalysis() = default;
    LivenessAnalysis(const std::map<std::string, std::shared_ptr<Assembly::Pseudo>> &pseudoMap,
                     const std::map<Assembly::RegName, std::shared_ptr<Assembly::Reg>> &hardRegMap)
        : pseudoMapPtr(&pseudoMap), hardregMapPtr(&hardRegMap) {}

    // Debug print function for CFG (optional, can be disabled via settings)
    static void debugPrintCfg(const std::string &extraTag,
                              const cfg::Graph<std::set<std::shared_ptr<Assembly::Operand>>, Assembly::Instruction> &cfg,
                              const RegAllocImpl::RegType &R,
                              Settings &settings)
    {
        auto dbg = R.debugSettings(settings);
        if (dbg.liveness)
        {
            // Print block live variables
            std::cout << "==== Liveness CFG (" << extraTag << ") ====" << '\n';
            for (const auto &[idx, blk] : cfg.basicBlocks)
            {
                std::cout << "Block " << idx << ": live = {";
                for (const auto &op : blk.value)
                {
                    std::cout << RegAllocImpl::showNodeId(op) << " ";
                }
                std::cout << "}\n";
            }
        }
    }

    // Meet function: computes live-out set for a block
    std::set<std::shared_ptr<Assembly::Operand>> meet(
        const cfg::Graph<std::set<std::shared_ptr<Assembly::Operand>>, Assembly::Instruction> &cfg,
        const cfg::BasicBlock<std::set<std::shared_ptr<Assembly::Operand>>, Assembly::Instruction> &block,
        const RegAllocImpl::RegType &R,
        const std::string &funName,
        AssemblySymbols::AsmSymbolTable &asmSymbol)
    {
        // Compute live-at-exit (return registers)
        std::set<std::shared_ptr<Assembly::Operand>> liveAtExit;
        {
            // Get all return regs for this function
            std::vector<Assembly::RegName> retRegs = asmSymbol.returnRegsUsed(funName);
            for (const auto &r : retRegs)
            {
                if (std::find(R.allHardregs.begin(), R.allHardregs.end(), r) != R.allHardregs.end())
                {
                    if (hardregMapPtr && hardregMapPtr->count(r))
                        liveAtExit.insert(hardregMapPtr->at(r));
                    else
                        liveAtExit.insert(std::make_shared<Assembly::Reg>(r));
                }
            }
        }

        std::set<std::shared_ptr<Assembly::Operand>> result;
        int succIdx = 0;
        for (const auto &succ : block.succs)
        {
            if (succ.kind == cfg::NodeID::Kind::Entry)
            {
                throw std::runtime_error("Internal error: malformed interference graph");
            }
            else if (succ.kind == cfg::NodeID::Kind::Exit)
            {
                for (const auto &op : liveAtExit)
                {
                    result.insert(canonicalizeOperand(op));
                }
            }
            else if (succ.kind == cfg::NodeID::Kind::Block)
            {
                auto succLive = cfg.basicBlocks.at(succ.index).value;
                for (const auto &op : succLive)
                {
                    result.insert(canonicalizeOperand(op));
                }
            }
            ++succIdx;
        }
        return result;
    }

    // Transfer function: computes live-in for each instruction in a block
    cfg::BasicBlock<std::set<std::shared_ptr<Assembly::Operand>>, Assembly::Instruction>
    transfer(const cfg::BasicBlock<std::set<std::shared_ptr<Assembly::Operand>>, Assembly::Instruction> &block,
             const std::set<std::shared_ptr<Assembly::Operand>> &endLiveRegs,
             const RegAllocImpl::RegType &R,
             AssemblySymbols::AsmSymbolTable &asmSymbol)
    {
        // Process instrs in reverse
        std::vector<std::pair<std::set<std::shared_ptr<Assembly::Operand>>, std::shared_ptr<Assembly::Instruction>>> annotatedInstrs;
        std::set<std::shared_ptr<Assembly::Operand>> currentLive;
        for (const auto &op : endLiveRegs)
            currentLive.insert(canonicalizeOperand(op));
        int instrIdx = 0;
        for (auto it = block.instructions.rbegin(); it != block.instructions.rend(); ++it, ++instrIdx)
        {
            const auto &instr = it->second;
            std::set<std::shared_ptr<Assembly::Operand>> regsUsed, regsWritten;
            RegAllocImpl(R).regsUsedAndWritten(instr, regsUsed, regsWritten, asmSymbol);

            // Canonicalize used/written operands
            std::set<std::shared_ptr<Assembly::Operand>> regsUsedCanocalized, regsWrittenCanocalized;
            for (const auto &op : regsUsed)
                regsUsedCanocalized.insert(canonicalizeOperand(op));
            for (const auto &op : regsWritten)
                regsWrittenCanocalized.insert(canonicalizeOperand(op));

            // Remove killed regs, add used regs
            std::set<std::shared_ptr<Assembly::Operand>> withoutKilled;
            for (const auto &op : currentLive)
            {
                if (regsWrittenCanocalized.find(op) == regsWrittenCanocalized.end())
                {
                    withoutKilled.insert(op);
                }
            }
            std::set<std::shared_ptr<Assembly::Operand>> newLive = withoutKilled;
            for (const auto &op : regsUsedCanocalized)
            {
                newLive.insert(op);
            }

            // Always canonicalize before storing in annotatedInstrs
            std::set<std::shared_ptr<Assembly::Operand>> canonicalCurrentLive;
            for (const auto &op : currentLive)
                canonicalCurrentLive.insert(canonicalizeOperand(op));
            annotatedInstrs.push_back({canonicalCurrentLive, instr});
            currentLive = newLive;
        }
        // Reverse back to original order
        std::reverse(annotatedInstrs.begin(), annotatedInstrs.end());

        // Build new block
        cfg::BasicBlock<std::set<std::shared_ptr<Assembly::Operand>>, Assembly::Instruction> newBlk = block;
        newBlk.instructions = annotatedInstrs;
        // Always canonicalize before storing in block value
        std::set<std::shared_ptr<Assembly::Operand>> canonicalFinalLive;
        for (const auto &op : currentLive)
            canonicalFinalLive.insert(canonicalizeOperand(op));
        newBlk.value = canonicalFinalLive;
        return newBlk;
    }

    // Analyze function: runs the backward dataflow analysis
    cfg::Graph<std::set<std::shared_ptr<Assembly::Operand>>, Assembly::Instruction>
    analyze(const std::string &funName,
            const RegAllocImpl::RegType &R,
            AssemblySymbols::AsmSymbolTable &asmSymbol,
            cfg::Graph<std::monostate, Assembly::Instruction> &cfg,
            Settings &settings)
    {
        auto debugPrinter = [&](const cfg::Graph<std::set<std::shared_ptr<Assembly::Operand>>, Assembly::Instruction> &g)
        {
            debugPrintCfg("in_progress", g, R, settings);
        };
        auto meetFn = [&](const cfg::Graph<std::set<std::shared_ptr<Assembly::Operand>>, Assembly::Instruction> &g,
                          const cfg::BasicBlock<std::set<std::shared_ptr<Assembly::Operand>>, Assembly::Instruction> &blk) -> std::set<std::shared_ptr<Assembly::Operand>>
        {
            return this->meet(g, blk, R, funName, asmSymbol);
        };
        auto transferFn = [&](const cfg::BasicBlock<std::set<std::shared_ptr<Assembly::Operand>>, Assembly::Instruction> &blk,
                              const std::set<std::shared_ptr<Assembly::Operand>> &endLiveRegs)
        {
            return this->transfer(blk, endLiveRegs, R, asmSymbol);
        };
        auto result = BackwardDataflow::analyze<std::shared_ptr<Assembly::Operand>, Assembly::Instruction>(
            debugPrinter, meetFn, transferFn, cfg);
        return result;
    }

private:
    std::shared_ptr<Assembly::Operand> canonicalizeOperand(const std::shared_ptr<Assembly::Operand> &op) const
    {
        if (!op)
            return op;
        if (pseudoMapPtr && op->getType() == Assembly::NodeType::Pseudo)
        {
            auto p = std::dynamic_pointer_cast<Assembly::Pseudo>(op);
            auto it = pseudoMapPtr->find(p->getName());
            if (it != pseudoMapPtr->end())
                return it->second;
        }
        else if (hardregMapPtr && op->getType() == Assembly::NodeType::Reg)
        {
            auto r = std::dynamic_pointer_cast<Assembly::Reg>(op);
            auto it = hardregMapPtr->find(r->getName());
            if (it != hardregMapPtr->end())
                return it->second;
        }
        return op;
    }
};

class DumpGraph
{
public:
    // Helper to write edges in the desired format
    static void dumpHelper(const std::string &filename,
                           const std::string &startGraph,
                           const std::string &endGraph,
                           std::function<void(std::ofstream &, const std::string &, const std::string &)> edgePrinter,
                           std::function<void(const std::string &)> postProcessor,
                           const std::map<std::shared_ptr<Assembly::Operand>, RegAllocImpl::Node> &g)
    {
        std::ofstream out(filename);
        if (!out.is_open())
            return;
        if (!startGraph.empty())
            out << startGraph;
        for (const auto &[nodeId, node] : g)
        {
            if (node.pruned)
                continue;
            for (const auto &nghborId : node.neighbors)
            {
                // Only print edge if neighbor is not pruned and nodeId < nghborId (to avoid duplicates)
                auto it = g.find(nghborId);
                if (it != g.end() && !it->second.pruned && nodeId < nghborId)
                {
                    edgePrinter(out, RegAllocImpl::showNodeId(nodeId), RegAllocImpl::showNodeId(nghborId));
                }
            }
        }
        if (!endGraph.empty())
            out << endGraph;
        out.close();
        postProcessor(filename);
    }

    // Dump as Graphviz DOT file and optionally convert to PNG
    static void dumpGraphviz(const std::string &basename, const std::map<std::shared_ptr<Assembly::Operand>, RegAllocImpl::Node> &g, bool doGraphviz = true)
    {
        std::cout << "dumpGraphviz\n";
        if (!doGraphviz)
            return;
        std::string dotfile = basename + ".dot";
        auto edgePrinter = [](std::ofstream &out, const std::string &a, const std::string &b)
        {
            out << "\t" << a << " -- " << b << "\n";
        };
        auto postProcessor = [](const std::string &filename)
        {
            // Convert DOT to PNG using circo, then remove DOT file
            std::string pngfile = filename.substr(0, filename.find_last_of('.')) + ".png";
            std::string cmd = "circo -Tpng " + filename + " -o " + pngfile;
            int ret = system(cmd.c_str());
            if (ret != 0)
                std::cerr << "graphviz fail: " << cmd << '\n';
            ret = system((std::string("del ") + filename).c_str());
            if (ret != 0)
                std::cerr << "failed to remove DOT file" << '\n';
        };
        dumpHelper(dotfile, "graph {\n", "}\n", edgePrinter, postProcessor, g);
    }

    // Dump as ncol format
    static void dumpNCol(const std::string &basename, const std::map<std::shared_ptr<Assembly::Operand>, RegAllocImpl::Node> &g, bool doNcol = true)
    {
        std::cout << "dumpNCol\n";
        if (!doNcol)
            return;
        std::string ncolfile = basename + ".ncol";
        auto edgePrinter = [](std::ofstream &out, const std::string &a, const std::string &b)
        {
            out << a << " " << b << "\n";
        };
        auto postProcessor = [](const std::string &) {};
        dumpHelper(ncolfile, "", "", edgePrinter, postProcessor, g);
    }
};

RegAllocImpl::Graph RegAllocImpl::buildInterferenceGraph(
    const std::string &fnName,
    const std::set<std::string> &aliasedPseudos,
    const std::vector<std::shared_ptr<Assembly::Instruction>> &instrs,
    AssemblySymbols::AsmSymbolTable &asmSymbol,
    Settings &settings)
{
    Graph graph = mkBaseGraph();
    addPseudoNodes(graph, aliasedPseudos, instrs, asmSymbol);
    auto cfgGraph = cfg::instructionsToCFG(fnName, instrs);
    LivenessAnalysis liveness(pseudoMap, hardRegMap);
    auto livenessCfg = liveness.analyze(fnName, R, asmSymbol, cfgGraph, settings);
    LivenessAnalysis::debugPrintCfg("annotated", livenessCfg, R, settings);
    addEdges(livenessCfg, graph, asmSymbol);

    return graph;
}

std::vector<std::shared_ptr<Assembly::Instruction>> RegAllocImpl::allocate(
    const std::string &fnName,
    const std::set<std::string> &aliasedPseudos,
    const std::vector<std::shared_ptr<Assembly::Instruction>> &instrs,
    AssemblySymbols::AsmSymbolTable &asmSymbol,
    Settings &settings)
{
    // Iterative coalescing loop
    auto currInstrs = instrs;
    Graph graph;
    while (true)
    {
        graph = buildInterferenceGraph(fnName, aliasedPseudos, currInstrs, asmSymbol, settings);
        auto dbg = R.debugSettings(settings);
        if (dbg.interferenceGraphviz)
            DumpGraph::dumpGraphviz(fnName + ".interference", graph, true);
        if (dbg.interferenceNcol)
            DumpGraph::dumpNCol(fnName + ".interference", graph, true);

        auto coalescedRegs = coalesce(graph, currInstrs, settings);
        // If no coalescing occurred, we're done
        if (coalescedRegs.isEmpty())
            break;
        currInstrs = rewriteCoalesced(currInstrs, coalescedRegs);
    }

    // Final graph and instructions after coalescing
    auto &coalescedGraph = graph;
    auto &coalescedInstrs = currInstrs;

    // Assign spill costs
    addSpillCosts(coalescedGraph, coalescedInstrs);
    // Color the graph
    colorGraph(coalescedGraph);

    // Build register map and record callee-saved usage
    auto pseudoToRegMap = makeRegisterMap(coalescedGraph, fnName, asmSymbol);
    // Replace pseudo registers with hardregs
    auto result = replacePseudoRegs(coalescedInstrs, pseudoToRegMap);
    return result;
}

std::shared_ptr<Assembly::Program>
allocateRegisters(
    const std::set<std::string> &aliasedPseudos,
    const std::shared_ptr<Assembly::Program> &program,
    AssemblySymbols::AsmSymbolTable &asmSymbol,
    Settings &settings)
{
    if (!program)
        return nullptr;
    std::vector<std::shared_ptr<Assembly::TopLevel>> newTls;
    for (const auto &tl : program->getTopLevels())
    {
        if (tl->getType() == Assembly::NodeType::Function)
        {
            auto fn = std::static_pointer_cast<Assembly::Function>(tl);
            const std::string &fnName = fn->getName();

            // Canonical maps for all hardregs (GP and XMM)
            std::map<Assembly::RegName, std::shared_ptr<Assembly::Reg>> canonicalHardregsMap;
            for (const auto &r : RegAllocImpl::GPType::allHardregs())
            {
                canonicalHardregsMap[r] = std::make_shared<Assembly::Reg>(r);
            }
            for (const auto &r : RegAllocImpl::XMMType::allHardregs())
            {
                canonicalHardregsMap[r] = std::make_shared<Assembly::Reg>(r);
            }

            std::map<std::string, std::shared_ptr<Assembly::Pseudo>> canonicalPseudoMap;
            for (const auto &instr : fn->getInstructions())
            {
                auto ops = RegAllocImpl::getOperandsStatic(instr);
                for (const auto &op : ops)
                {
                    if (op && op->getType() == Assembly::NodeType::Pseudo)
                    {
                        auto p = std::dynamic_pointer_cast<Assembly::Pseudo>(op);
                        if (p && canonicalPseudoMap.find(p->getName()) == canonicalPseudoMap.end())
                        {
                            canonicalPseudoMap[p->getName()] = p;
                        }
                    }
                }
            }

            RegAllocImpl::RegType gpRegType{
                "gp",
                RegAllocImpl::GPType::allHardregs(),
                RegAllocImpl::GPType::callerSavedRegs(),
                [&](const std::string &p)
                { return RegAllocImpl::GPType::pseudoIsCurrentType(p, asmSymbol); },
                [](Settings &settings)
                { return RegAllocImpl::GPType::debugSettings(settings); }};

            RegAllocImpl gpAlloc{gpRegType, canonicalPseudoMap, canonicalHardregsMap};

            RegAllocImpl::RegType xmmRegType{
                "xmm",
                RegAllocImpl::XMMType::allHardregs(),
                RegAllocImpl::XMMType::callerSavedRegs(),
                [&](const std::string &p)
                { return RegAllocImpl::XMMType::pseudoIsCurrentType(p, asmSymbol); },
                [](Settings &settings)
                { return RegAllocImpl::XMMType::debugSettings(settings); }};

            RegAllocImpl xmmAlloc{xmmRegType, canonicalPseudoMap, canonicalHardregsMap};

            auto gpAllocated = gpAlloc.allocate(fnName, aliasedPseudos, fn->getInstructions(), asmSymbol, settings);
            auto xmmAllocated = xmmAlloc.allocate(fnName, aliasedPseudos, gpAllocated, asmSymbol, settings);
            auto newFn = std::make_shared<Assembly::Function>(fnName, fn->isGlobal(), xmmAllocated);
            newTls.push_back(newFn);
        }
        else
        {
            newTls.push_back(tl);
        }
    }
    return std::make_shared<Assembly::Program>(newTls);
}
