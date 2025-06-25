#ifndef TACKY_GEN_H
#define TACKY_GEN_H

#include <vector>
#include <utility>
#include <memory>
#include <string>
#include "Tacky.h"
#include "AST.h"
#include "Symbols.h"
#include "Types.h"

struct PlainOperand;
struct DereferencedPointer;
struct SubObject;
using ExpResult = std::variant<PlainOperand, DereferencedPointer, SubObject>;

class TackyGen
{
public:
    TackyGen(Symbols::SymbolTable &symbolTable, TypeTableNS::TypeTable &typeTable) : _symbolTable(symbolTable), _typeTable{typeTable} {};

    int getMemberOffset(const std::string &member, const Types::DataType &type);
    int getMemberPointerOffset(const std::string &member, const Types::DataType &type);
    std::shared_ptr<TACKY::Constant> evalSize(const Types::DataType &type);
    std::string createTmp(const std::optional<Types::DataType> &type);
    std::shared_ptr<Constants::Const> mkConst(const std::optional<Types::DataType> &type, int64_t i);
    std::shared_ptr<AST::Constant> mkAstConst(const std::optional<Types::DataType> &type, int64_t i);
    std::shared_ptr<TACKY::Instruction> getCastInst(const std::shared_ptr<TACKY::Val> &src, const std::shared_ptr<TACKY::Val> &dst, const Types::DataType &srcType, const Types::DataType &dstType);
    ssize_t getPtrScale(const Types::DataType &type);

    std::string breakLabel(const std::string &id);
    std::string continueLabel(const std::string &id);

    TACKY::UnaryOp convertUnop(AST::UnaryOp op);
    TACKY::BinaryOp convertBinop(AST::BinaryOp op);

    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForDoLoop(const std::shared_ptr<AST::DoWhile> &doLoop);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForWhileLoop(const std::shared_ptr<AST::While> &whileLoop);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForForLoop(const std::shared_ptr<AST::For> &forLoop);

    std::vector<std::shared_ptr<TACKY::Instruction>> emitStringInit(const std::string &s, const std::string &dst, int offset);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitCompoundInit(const std::shared_ptr<AST::Initializer> &init, const std::string &name, ssize_t offset);

    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitDotOperator(const std::shared_ptr<AST::Dot> &dot);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitArrowOperator(const std::shared_ptr<AST::Arrow> &arrow);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitSubscript(const std::shared_ptr<AST::Subscript> &subscript);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitPointerAddition(const Types::DataType &type, const std::shared_ptr<AST::Expression> &exp1, const std::shared_ptr<AST::Expression> &exp2);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitSubtractionFromPointer(const Types::DataType &type, const std::shared_ptr<AST::Expression> &ptrExp, const std::shared_ptr<AST::Expression> &indexExp);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitPointerDiff(const Types::DataType &type, const std::shared_ptr<AST::Expression> &exp1, const std::shared_ptr<AST::Expression> &exp2);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitDereference(const std::shared_ptr<AST::Dereference> &dereference);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitAddrOf(const std::shared_ptr<AST::AddrOf> &addrOf);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitAssignment(const std::shared_ptr<AST::Expression> &lhs, const std::shared_ptr<AST::Expression> &rhs);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitCastExp(const std::shared_ptr<AST::Cast> &cast);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitFunCall(const std::shared_ptr<AST::FunctionCall> &fnCall);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitConditionalExp(const std::shared_ptr<AST::Conditional> &conditional);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitPostfix(const AST::BinaryOp &op, const std::shared_ptr<AST::Expression> &var);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitCompoundExpression(const AST::BinaryOp &op, const std::shared_ptr<AST::Expression> &lhs, const std::shared_ptr<AST::Expression> &rhs, const std::optional<Types::DataType> &resultType);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitAndExp(const std::shared_ptr<AST::Binary> &binary);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitOrExp(const std::shared_ptr<AST::Binary> &binary);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitUnaryExp(const std::shared_ptr<AST::Unary> &unary);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitBinaryExp(const std::shared_ptr<AST::Binary> &binary);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<ExpResult>> emitTackyForExp(const std::shared_ptr<AST::Expression> &exp);
    std::pair<std::vector<std::shared_ptr<TACKY::Instruction>>, std::shared_ptr<TACKY::Val>> emitTackyAndConvert(const std::shared_ptr<AST::Expression> &exp);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForSwitch(const std::shared_ptr<AST::Switch> &switchStmt);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForIfStatement(const std::shared_ptr<AST::If> &ifStmt);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForStatement(const std::shared_ptr<AST::Statement> &stmt);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitLocalDeclaration(const std::shared_ptr<AST::Declaration> &decl);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitVarDeclaration(const std::shared_ptr<AST::VariableDeclaration> &varDecl);
    std::vector<std::shared_ptr<TACKY::Instruction>> emitTackyForBlockItem(const std::shared_ptr<AST::BlockItem> &blockItem);
    std::optional<std::shared_ptr<TACKY::Function>> emitFunDeclaration(const std::shared_ptr<AST::FunctionDeclaration> &funDef);
    std::vector<std::shared_ptr<TACKY::TopLevel>> convertSymbolsToTacky();
    std::shared_ptr<TACKY::Program> gen(const std::shared_ptr<AST::Program> &prog);

private:
    Symbols::SymbolTable &_symbolTable;
    TypeTableNS::TypeTable &_typeTable;
};

#endif