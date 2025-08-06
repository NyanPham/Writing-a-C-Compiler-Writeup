#include "BackwardDataflow.h"
#include "Tacky.h"
#include "Assembly.h"

// Explicit template instantiations for the types used in your project
template BackwardDataflow::AnnotatedGraph<std::string, TACKY::Instruction>
BackwardDataflow::analyze<std::string, TACKY::Instruction>(
    std::function<void(const cfg::Graph<BackwardDataflow::Annotation<std::string, TACKY::Instruction>, TACKY::Instruction> &)>,
    std::function<BackwardDataflow::Annotation<std::string, TACKY::Instruction>(
        const cfg::Graph<BackwardDataflow::Annotation<std::string, TACKY::Instruction>, TACKY::Instruction> &,
        const cfg::BasicBlock<BackwardDataflow::Annotation<std::string, TACKY::Instruction>, TACKY::Instruction> &)>,
    std::function<cfg::BasicBlock<BackwardDataflow::Annotation<std::string, TACKY::Instruction>, TACKY::Instruction>(
        const cfg::BasicBlock<BackwardDataflow::Annotation<std::string, TACKY::Instruction>, TACKY::Instruction> &,
        const BackwardDataflow::Annotation<std::string, TACKY::Instruction> &)>,
    const cfg::Graph<std::monostate, TACKY::Instruction> &);

// Instantiation for shared_ptr<Assembly::Operand> (used in RegAlloc/Liveness)
template BackwardDataflow::AnnotatedGraph<std::shared_ptr<Assembly::Operand>, Assembly::Instruction>
BackwardDataflow::analyze<std::shared_ptr<Assembly::Operand>, Assembly::Instruction>(
    std::function<void(const cfg::Graph<BackwardDataflow::Annotation<std::shared_ptr<Assembly::Operand>, Assembly::Instruction>, Assembly::Instruction> &)>,
    std::function<BackwardDataflow::Annotation<std::shared_ptr<Assembly::Operand>, Assembly::Instruction>(
        const cfg::Graph<BackwardDataflow::Annotation<std::shared_ptr<Assembly::Operand>, Assembly::Instruction>, Assembly::Instruction> &,
        const cfg::BasicBlock<BackwardDataflow::Annotation<std::shared_ptr<Assembly::Operand>, Assembly::Instruction>, Assembly::Instruction> &)>,
    std::function<cfg::BasicBlock<BackwardDataflow::Annotation<std::shared_ptr<Assembly::Operand>, Assembly::Instruction>, Assembly::Instruction>(
        const cfg::BasicBlock<BackwardDataflow::Annotation<std::shared_ptr<Assembly::Operand>, Assembly::Instruction>, Assembly::Instruction> &,
        const BackwardDataflow::Annotation<std::shared_ptr<Assembly::Operand>, Assembly::Instruction> &)>,
    const cfg::Graph<std::monostate, Assembly::Instruction> &);
