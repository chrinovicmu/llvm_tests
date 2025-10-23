#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include <cerrno>
#include <llvm-14/llvm/ADT/StringRef.h>
#include "llvm/IR/GlobalValue.h"

struct AnalyzeInstrPass : public llvm::MachineFunctionPass {

    static char ID; 
    AnalyzeInstrPass() : llvm::MachineFunctionPass(ID) {}

    void analyzeInstr(llvm::MachineInstr &MI){
        llvm::errs() << "Instraction: " << MI; 

        for(const llvm::MachineOperand &MO : MI.operands()){
            if(MO.isReg()){
                llvm::errs() << " Register" << llvm::printReg(MO.getReg()); 
                if(MO.isDef())
                    llvm::errs() << "       (defined this register)\n"; 
                if(MO.isUse())
                    llvm::errs() << "       (uses this register)\n"; 
            }else if (MO.isImm()){
                llvm::errs() << "   Immediate: " << MO.getImm() << "\n"; 
            }else if(MO.isGlobal()){
                llvm::errs() << "   Global: " << MO.getGlobal()->getName() << "\n"; 
            }else if(MO.isMBB()){
                llvm::errs() << "   MachineBasicBlock: " << MO.getMBB()->getName() << "\n"; 
            }else if(MO.isFI()){
                llvm::errs() << " FrameIndex: " << MO.getIndex() << "\n"; 
            }
        }
        llvm::errs() << "---------------------------------------------\n"; 
    }

    bool runOnMachineFunction(llvm::MachineFunction &MF) override {

    llvm:;llvm::errs() << "\nAnalysing MachineFunction: " << MF.getName() << "\n";

        for(llvm::MachineBasicBlock &MBB: MF){
            llvm::errs() << "\nBasicBlock: " << MBB.getName() << "\n"; 

            for(llvm::MachineInstr &MI : MBB){
                analyzeInstr(MI); 
            }
        }
        return false; 
    }

    llvm::StringRef getPassName() const override{
        return "MachineInstr Operand Analyzer"; 
    }
}; 

char AnalyzeInstrPass::ID = 0;

static llvm::RegisterPass<AnalyzeInstrPass>
X("analyze-mops", "Analyze MachineInstr Operand", false, false); 

