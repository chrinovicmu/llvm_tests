#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm-14/llvm/Support/Casting.h>


struct DetectIntrinsicsPass : public llvm::FunctionPass{

    static char ID; 
    DetectIntrinsicsPass() : llvm::FunctionPass(ID) {}

    bool runOnFunction(llvm::Function &F) override {
        for(auto &BB: F){
            for(auto &I : BB){
                if(auto *callInst = llvm::dyn_cast<llvm::CallInst>(&I)){
                    llvm::Function *calledFunc = callInst->getCalledFunction();
                    if(!calledFunc) continue; 

                    if(calledFunc->isIntrinsic()){
                        llvm::errs() << "Intrinsic call : " << calledFunc->getName() << "\n"; 

                        llvm::Intrinsic::ID id = calledFunc->getIntrinsicID(); 
                        if(llvm::Function::isTargetIntrinsic(id)){
                            llvm::errs() << "  -> Target-specific instrinsic\n";
                        }
                    }
                }
            }
        }
        return false; 
    }
}; 

char DetectIntrinsicsPass::ID = 0;
static llvm::RegisterPass<DetectIntrinsicsPass> X("detect-intrinsics", "Detect LLVM Intrinsics", false, false);
