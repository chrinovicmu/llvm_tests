
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineFrameInfo.h" // For CreateStackObject.
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineMemOperand.h" // For MachinePointerInfo.
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGenTypes/LowLevelType.h" // For LLT.
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h" // For ICMP_EQ.
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/LowLevelTypeImpl.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/X86/X86.h"
#include "llvm/Target/X86/X86RegisterInfo.h"
#include "llvm/Target/X86/X86InstrInfo.h"
// extern int baz();
// extern void bar(int);
// void foo(int a, int b) {
//   int var = a + b;
//   if (var == 0xFF) {
//     bar(var);
//     var = baz();
//   }
//   bar(var);
// }
//
//
llvm::MachineFunction *solutionPopulateMachineIR(llvm::MachineModuleInfo *MMI,
                                                 llvm::Function &Foo)
{
    llvm::MachineFunction &MF = MMI::getOrCreateMachineFunction(Foo); 

    /*create basics blocks */ 
    llvm::MachineBasicBlock *BB = MF.CreateMachineBasicBlock(); 
    MF.push_back(BB); 

    llvm::MachineBasicBlock *BB9 = MF.CreateMachineBasicBlock(); 
    MF.push_back(BB9); 

    llvm::MachineBasicBlock *BB12 = MF.CreateMachineBasicBlock(); 
    MF.push_back(BB12);

    /*create cinfigration for the CFG */ 

    BB->addSuccessor(BB9); 
    BB->addSuccessor(BB12); 
    BB9->addSuccessor(BB12); 

    /*bool type*/ 
    llvm::LLT I1 = llvm::LLT::scalar(1); 
    /*var type */ 
    llvm::LLT I32 = llvm::LLT::scalar(32); 
    /*64 bit pointer type */ 
    llvm::LLT VarAddrLLT = llvm::LLT::pointer(0, 64); 

    llvm::MachinePointerInfo PtrInfo; 
    llvm::Align VarStackAlign(4); 

    /*stack slot for the var */ 
    int FrameIndex = MF.getFrameInfo().CreateStackObject(32, VarStackAlign, false);

    llvm::MachineIRBuilder MIBuilder(BB, BB->end()); 

    /*get inpute arguments */ 

    llvm::Register A = MIBuilder.buildCopy(I32, llvm::X86::EDI).getReg(0);
    llvm::Register B = MIBuilder.buildCopy(I32, llvm::X86::ESI).getReg(0);

    /*virtual register of stack slot wiht frameindex*/ 
    llvm::Register VarStackAddr  = MIBuilder.buildFrameIndex(VarAddrLLT, FrameIndex).getReg(0);

    llvm::Register ResAdd = MIBuilder.buildAdd(I32, A, B).getReg(0); 

    /*write add result to var's adddress*/ 
    MIBuilder.buildStore(ResAdd, VarStackAddr, PtrInfo, VarStackAlign); 

    /*build ICMP */ 
    /*create constant */ 
    llvm::Register CstOxFF = MIBuilder.buildConstant(I32, 0xFF).getReg(0); 

    llvm::Register ReloadedVar0 = 
        MIBuilder.buildLoad(I32, VarStackAddr, PtrInfo, VarStackAlign).getReg(0); 
        
    llvm::Register Cmp = 
        MIBuilder.buildICmp(llvm::CmpInst::ICMP_EQ, I1, ReloadedVar0, CstOxFF).getReg(0); 

    /*conditional branch 
    * if trur jmp yo BB9 */ 
    MIBuilder.buildBrCond(Cmp, *BB9);
    /*else jump to BB12*/ 
    MIBuilder.buildBr(*BB12); 

    /*BB9 */

    MIBuilder.setInsertPt(BB9, BB9->end());

    llvm::Register ReloadedVar1 = 
        MIBuilder.buildLoad(I32, VarStackAddr, PtrInfo, VarStackAlign).getReg(0); 

    MIBuilder.buildCopy(llvm::X86::EAX, ReloadedVar1);

    /*fake call to bar*/ 

    MIBuilder.buildInstr(llvm::TargetOpcode::INLINEASM, {}, {})
        .addExternalSymbol("call bar")
        .addImm(0)
        .addReg(llvm::X86::RAX, llvm::RegState::Implicit); 

    MIBuilder.buildInstr(llvm::TargetOpcode::INLINEASM, {}, {})
        .addExternalSymbol("call baz")
        .addImm(0) 
        .addReg(llvm::X86::RAX, llvm::RegState::Implicit | llvm::RegState::Define);

    llvm::Register ResOfBaz = MIBuilder.buildCopy(I32, llvm::X86::RAX).getReg(0); 
    MIBuilder.buildStore(ResOfBaz, VarStackAddr, PtrInfo, VarStackAlign); 

    /*BB12 */

    MIBuilder.setInsertPt(BB12, BB12->end());

    llvm::Register ReloadedVar2 = 
        MIBuilder.buildLoad(I32, VarStackAddr, PtrInfo, VarStackAlign); 
    MIBuilder.buildCopy(llvm::X86::RAX, ReloadedVar2); 

    MIBuilder.buildInstr(llvm::TargetOpcode::INLINEASM, {}, {})
        .addExternalSymbol("call bar")
        .addImm(0)
        .addReg(llvm::X86::RAX, llvm::RegState::Implicit); 

    MIBuilder.buildInstr(llvm::TargetOpcode::INLINEASM, {}, {})
        .addExternalSymbol("ret")
        .addImm(0); 

    return &MF; 
}

int main (int argc, char *argv[]) {
    
    llvm::InitializeAllTargetInfos(); 
    llvm::InitializeAllTargets(); 
    llvm::InitializeAllTargetMCs(); 
    llvm::InitializeAllAsmParsers(); 
    llvm::InitializeAllAsmPrinters(); 

    llvm::LLVMContext Context; 
    llvm::Module M("test_module", Context); 

    llvm::FunctionType *FTy = 
        llvm::FunctionType::get(llvm::Type::getVoidTy(Context), 
                                {llvm::Type::getInt32Ty(Context), 
                                llvm::Type::getInt32Ty(Context)}, 
                                false); 

    llvm::Function *Foo = 
        llvm::Function::Create(FTy, llvm::Function::ExternalLinkage, "foo", M); 

    llvm::MachineModuleInfo MMI(&M); 

    llvm::MachineFunction *MF = solutionPopulateMachineIR(&MMI, *Foo); 

    if (!MF) {
        llvm::errs() << "Failed to generate MachineFunction.\n";
        return 1;
    }

    // 6️⃣ Open output file for writing MIR
    std::error_code EC;
    llvm::raw_fd_ostream FileOut("output.mir", EC, llvm::sys::fs::OF_Text);
    if (EC) {
        llvm::errs() << "Error opening file: " << EC.message() << "\n";
        return 1;
    }

    // 7️⃣ Print the MachineFunction (MIR) to the file
    MF->print(FileOut);

    llvm::outs() << "MIR dumped to output.mir\n";
    return 0;
}
