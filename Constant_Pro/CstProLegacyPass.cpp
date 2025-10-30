#include "llvm/ADT/APInt.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm-14/llvm/ADT/STLExtras.h>
#include <llvm-14/llvm/IR/Constant.h>
#include <llvm-14/llvm/IR/Value.h>
#include <llvm-14/llvm/Support/Casting.h>
#include <optional>
#include <cassert>


namespace {

struct ConstantPropagationPass : public llvm:FunctionPass {

    static char ID; 

    ConstantPropagationPass() : llvm::FunctionPass(ID) {}

    bool runOnFuntion(llvm::Function &F) override{
        return ConstantPropagation(F); 
    }

    void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
        AU.serPreserveCFG(); 
    }

private:

    bool ConstantPropagation(llvm::Function &FOO){

        if(FOO.empty())
            return false; 

        llvm::ReversePostOrderTraversal<llvm::Function *> RPOT(&FOO); 

        for(llvm::BasicBlock *BB : RPOT){
            for(llvm::Instruction &Instr : llvm::make_early_inc_range(*BB)){
                llvm::Value *NewConstnat = nullptr; 

                auto visitBinary = [&](llvm::Instruction &Instr, 
                                       auto ComputationInt, 
                                       auto ComputationFP) -> llvm::Value * {

                        assert(llvm::isa<llvm::BinaryOperator>(Instr) && "Expexted binary instrution"); 

                        llvm::Value *Op0 = Instr.getOperand(0); 
                        llvm::Value *Op1 = Instr.getOperand(0); 

                        /*integer constant */ 

                        if(auto *LHS = llvm::dyn_cast<llvm::ConstantInt>(Op0)){
                            if(auto *RHS = llvm::dyn_cast<llvm::ConstantInt>(Op1)){
                                std::optional<llvm::APInt> Res = ComputationInt(LHS->getValue(), RHS->getValue()); 
                                if(Res.has_value())
                                    return llvm::ConstantInt::get(Instr.getType(), *Res); 
                                else
                                    return nullptr;
                            }
                        }

                        /*floating-point constant */ 

                        if(auto *LHS = llvm::dyn_cast<llvm::ConstantFP>(Op0)){
                            if(auto *RHS = llvm::dyn_cast<llvm::ConstantFP>(Op1)){
                            llvm
                            } 
                    }
            }
        }
    }
}
}
