#include "llvm/ADT/APInt.h"
#include "llvm/ADT/PostOrderIterator.h" // For ReversePostOrderTraversal.
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"       // To instantiate RPOTraversal.
#include "llvm/IR/Constants.h" // For ConstantInt.
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h" // For BinaryOperator, etc.
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h" // For errs().
#include <cassert>
#include <llvm-14/llvm/ADT/STLExtras.h>
#include <llvm-14/llvm/IR/Value.h>
#include <llvm-14/llvm/Support/Casting.h>
#include <optional>

static llvm::Value *visitBinary(llvm::Instruction &Instr, llvm::LLVMContext &Ctxt, 
                                std::optional<llvm::APInt>(*Computation)
                                (const llvm::APInt &, const llvm::APInt &)){

    /*check at runtime if Instr is binary Instruction*/ 
    assert(llvm::isa<llvm::BinaryOperator>(Instr) && "This is meant for binary Instruction"); 

    auto *LHS = llvm::dyn_cast<llvm::ConstantInt>(Instr.getOperand(0)); 
    auto *RHS = llvm::dyn_cast<llvm::ConstantInt>(Instr.getOperand(1)); 

    if(!LHS || RHS)
        return nullptr; 

    std::optional<llvm::APInt> Res = Computation(LHS->getValue(), RHS->getValue());

    if(!Res.has_value())
        return nullptr; 

    auto NewConstant = llvm::ConstantInt::get(Ctxt, *Res); 

    return NewConstant; 

}

bool ConstantPropagation(llvm::Function &FOO)
{
    if(FOO.empty())
        return false; 

    llvm::LLVMContext &Ctxt = FOO.getParent()->getContext(); 

    bool MadeChanges = false; 

    llvm::ReversePostOrderTraversal<llvm::Function *> RPOT(&FOO); 

    for(llvm::BasicBlock *BB : RPOT){

        for(llvm::Instruction &Instr : llvm::make_early_inc_range(*BB)){
            llvm::Value *NewConstant = nullptr; 


            
            switch (Instr.getOpcode()) {

                case llvm::Instruction::Add:
                    NewConstant = visitBinary(
                        Instr, Ctxt,
                        [](const llvm::APInt &A, const llvm::APInt &B) -> std::optional<llvm::APInt> {
                            return A + B;
                        });
                    break;

                case llvm::Instruction::Sub:
                    NewConstant = visitBinary(
                        Instr, Ctxt,
                        [](const llvm::APInt &A, const llvm::APInt &B) -> std::optional<llvm::APInt> {
                            return A - B;
                        });
                    break;

                case llvm::Instruction::Mul:
                    NewConstant = visitBinary(
                        Instr, Ctxt,
                        [](const llvm::APInt &A, const llvm::APInt &B) -> std::optional<llvm::APInt> {
                            return A * B;
                        });
                    break;

                case llvm::Instruction::UDiv:
                    NewConstant = visitBinary(
                        Instr, Ctxt,
                        [](const llvm::APInt &A, const llvm::APInt &B) -> std::optional<llvm::APInt> {
                            if (B == 0) return std::nullopt;  // Avoid division by zero
                            return A.udiv(B);
                        });
                    break;

                case llvm::Instruction::SDiv:
                    NewConstant = visitBinary(
                        Instr, Ctxt,
                        [](const llvm::APInt &A, const llvm::APInt &B) -> std::optional<llvm::APInt> {
                            if (B == 0) return std::nullopt;
                            return A.sdiv(B);
                        });
                    break;

                case llvm::Instruction::And:
                    NewConstant = visitBinary(
                        Instr, Ctxt,
                        [](const llvm::APInt &A, const llvm::APInt &B) -> std::optional<llvm::APInt> {
                            return A & B;
                        });
                    break;

                case llvm::Instruction::Or:
                    NewConstant = visitBinary(
                        Instr, Ctxt,
                        [](const llvm::APInt &A, const llvm::APInt &B) -> std::optional<llvm::APInt> {
                            return A | B;
                        });
                    break;

                case llvm::Instruction::Xor:
                    NewConstant = visitBinary(
                        Instr, Ctxt,
                        [](const llvm::APInt &A, const llvm::APInt &B) -> std::optional<llvm::APInt> {
                            return A ^ B;
                        });
                    break;

                case llvm::Instruction::Shl:
                    NewConstant = visitBinary(
                        Instr, Ctxt,
                        [](const llvm::APInt &A, const llvm::APInt &B) -> std::optional<llvm::APInt> {
                            return A.shl(B.getZExtValue());
                        });
                    break;

                case llvm::Instruction::LShr:
                    NewConstant = visitBinary(
                        Instr, Ctxt,
                        [](const llvm::APInt &A, const llvm::APInt &B) -> std::optional<llvm::APInt> {
                            return A.lshr(B.getZExtValue());
                        });
                    break;

                case llvm::Instruction::AShr:
                    NewConstant = visitBinary(
                        Instr, Ctxt,
                        [](const llvm::APInt &A, const llvm::APInt &B) -> std::optional<llvm::APInt> {
                            return A.ashr(B.getZExtValue());
                        });
                    break;

                default:
                    NewConstant = nullptr;
                    break;
                }
                   
        }
    }
}
