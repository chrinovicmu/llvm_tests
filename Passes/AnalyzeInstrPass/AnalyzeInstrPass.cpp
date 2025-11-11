
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

struct AnalyzeInstrPass : PassInfoMixin<AnalyzeInstrPass> {

    PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
        errs() << "Analyzing Function: " << F.getName() << "\n";

        for (auto &BB : F) {
            errs() << "BasicBlock: ";
            if (BB.hasName())
                errs() << BB.getName();
            errs() << "\n";

            for (auto &I : BB) {
                errs() << "  Instruction: " << I.getOpcodeName() << "\n";
                for (unsigned idx = 0; idx < I.getNumOperands(); ++idx) {
                    auto Op = I.getOperand(idx);
                    if (Op->getType()->isIntegerTy()) {
                        errs() << "    Operand " << idx << ": integer value\n";
                    } else if (Op->getType()->isPointerTy()) {
                        errs() << "    Operand " << idx << ": pointer\n";
                    } else {
                        errs() << "    Operand " << idx << ": other\n";
                    }
                }
            }
        }

        return PreservedAnalyses::all(); // analysis only
    }
};

// Register the pass plugin
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "AnalyzeInstrPass", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM,
                       ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "analyze-mops") {
                            FPM.addPass(AnalyzeInstrPass());
                            return true;
                        }
                        return false;
                    });
            }};
}
