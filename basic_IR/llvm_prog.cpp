#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"

#include <memory>

// The goal of this function is to build a Module that
// represents the lowering of the following foo, a C function:
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
// The IR for this snippet (at O0) is:
// define void @foo(i32 %arg, i32 %arg1) {
// bb:
//   %i = alloca i32
//   %i2 = alloca i32
//   %i3 = alloca i32
//   store i32 %arg, ptr %i
//   store i32 %arg1, ptr %i2
//   %i4 = load i32, ptr %i
//   %i5 = load i32, ptr %i2
//   %i6 = add i32 %i4, %i5
//   store i32 %i6, ptr %i3
//   %i7 = load i32, ptr %i3
//   %i8 = icmp eq i32 %i7, 255
//   br i1 %i8, label %bb9, label %bb12
//
// bb9:
//   %i10 = load i32, ptr %i3
//   call void @bar(i32 %i10)
//   %i11 = call i32 @baz()
//   store i32 %i11, ptr %i3
//   br label %bb12
//
// bb12:
//   %i13 = load i32, ptr %i3
//   call void @bar(i32 %i13)
//   ret void
// }
//
// declare void @bar(i32)
// declare i32 @baz(...)
std::unique_ptr<llvm::Module> myBuildModule(llvm::LLVMContext &Ctxt) {

    llvm::Type *Int32Ty = llvm::Type::getInt32Ty(Ctxt); 
    llvm::Type *VoidTy = llvm::Type::getVoidTy(Ctxt); 

    std::unique_ptr<llvm::Module> MyModule = std::make_unique<llvm::Module>("Solution Module", Ctxt);

    llvm::FunctionType *BazTy = 
        llvm::FunctionType::get(Int32Ty, false); //int32 return type and faslse = not variadic

    llvm::Function *BazFunc =
        dyn_cast<llvm::Function>(MyModule->getOrInsertFunction("baz", BazTy).getCallee()); 

    llvm::FunctionType *BarTy = 
        llvm::FunctionType::get(VoidTy, {Int32Ty}, false); 

    llvm::Function *BarFunc = 
        dyn_cast<llvm::Function>(MyModule->getOrInsertFunction("bar", BarTy).getCallee()); 
    
    llvm::FunctionType *FooTy = 
        llvm::FunctionType::get(VoidTy, {Int32Ty, Int32Ty}, false); 

    llvm::Function *FooFunc = 
        dyn_cast<llvm::Function>(MyModule->getOrInsertFunction("foo", FooTy).getCallee()); 

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(Ctxt, "bb", FooFunc); 

    llvm::BasicBlock *BB9 = llvm::BasicBlock::Create(Ctxt, "bb9", FooFunc); 

    llvm::BasicBlock *BB12 = llvm::BasicBlock::Create(Ctxt, "bb12", FooFunc); 

    llvm::IRBuilder<> Builder(Ctxt);


    /*populate BB */

    Builder.SetInsertPoint(BB); 

    /*allocate stack space for local variables */ 
    
    llvm::Value *I = Builder.CreateAlloca(Int32Ty); 
    llvm::Value *I2 = Builder.CreateAlloca(Int32Ty); 
    llvm::Value *I3 = Builder.CreateAlloca(Int32Ty); 

    /*get function args*/ 
    llvm::Value *Arg1 = FooFunc->getArg(0); 
    llvm::Value *Arg2 = FooFunc->getArg(1); 

    /*store them in local variables */ 
    Builder.CreateStore(Arg1, I); 
    Builder.CreateStore(Arg2, I2); 

    /* reload fro, the local variables*/ 
    llvm::Value *I4 = Builder.CreateLoad(Int32Ty, I); 
    llvm::Value *I5 = Builder.CreateLoad(Int32Ty, I2); 

    /*perform add */ 
    llvm::Value *I6 = Builder.CreateAdd(I4, I5); 

    /*store result in local varaible i3 */
    Builder.CreateStore(I6, I3); 

    //reload from I3
    
    llvm::Value *I7 = Builder.CreateLoad(Int32Ty, I3); 

    llvm::Value *Cst255 = llvm::ConstantInt::get(Int32Ty, 255); 
    llvm::Value *I8 = Builder.CreateICmpEQ(I7, Cst255); 

    Builder.CreateCondBr(I8, BB9, BB12); 

    /*populate bb9 */ 

    Builder.SetInsertPoint(BB9); 

    /*reload from memory*/ 
    llvm::Value * I10 = Builder.CreateLoad(Int32Ty, I3); 

    /*call bar function */
    Builder.CreateCall(BarFunc, llvm::ArrayRef<llvm::Value*>{I10}, "callbar");

    /*cal baz */ 
    llvm::Value *I11 = Builder.CreateCall(BazFunc, llvm::ArrayRef<llvm::Value*>{}, "callbaz"); 

    Builder.CreateStore(I11, I3); 

    Builder.CreateBr(BB12); 

    Builder.SetInsertPoint(BB12); 

    llvm::Value *I13 = Builder.CreateLoad(Int32Ty, I3); 

    Builder.CreateCall(BarFunc, llvm::ArrayRef<llvm::Value*>{I13}, "bb12_callbar"); 

    Builder.CreateRetVoid(); 

    return MyModule; 
}

int main (int argc, char *argv[]) {
    
    llvm::LLVMContext Context;
    auto M = myBuildModule(Context); 

    std::error_code EC;
    llvm::raw_fd_stream OS("output.ll", EC);

    if(EC){
        llvm::errs() << "Error opening file: " << EC.message() << "\n";
        return 1;  
    }
    
    M->print(OS, nullptr); 
    llvm::outs() << "LLVM IR written output.ll"; 


    return 0;
}
