#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace llvm;

namespace {
struct MyPass : public FunctionPass {
  static char ID;
  MyPass() : FunctionPass(ID) {}

  bool isFuncLogger(StringRef name) { return name == "CallFuncLog"; }

  virtual bool runOnFunction(Function &F) {
    if (isFuncLogger(F.getName())) {
      return false;
    }

    // Create IR builder, get context
    LLVMContext &Ctx = F.getContext();
    IRBuilder<> builder(Ctx);
    Type *retType = Type::getVoidTy(Ctx);

    // Construct LogginFunction class
    ArrayRef<Type *> CallFuncParams = {builder.getInt8Ty()->getPointerTo(),
                                       builder.getInt8Ty()->getPointerTo(),
                                       Type::getInt64Ty(Ctx)};
    FunctionType *FuncType =
        FunctionType::get(retType, CallFuncParams, false);
    FunctionCallee FunctionCall =
        F.getParent()->getOrInsertFunction("CallFuncLog", FuncType);

    // Insert loggers for call, binOpt and ret instructions
    for (auto &B : F) {
      for (auto &I : B) {
        Value *valueAddr =
            ConstantInt::get(builder.getInt64Ty(), (int64_t)(&I));
        if (auto *call = dyn_cast<CallInst>(&I)) {
          builder.SetInsertPoint(call);

          Function *callee = call->getCalledFunction();
          if (callee && !isFuncLogger(callee->getName())) {
            Value *CalleeName =
                builder.CreateGlobalStringPtr(callee->getName());
            Value *FuncName = builder.CreateGlobalStringPtr(F.getName());
            Value *args[] = {FuncName, CalleeName, valueAddr};
            builder.CreateCall(FunctionCall, args);
          }
        }
      }
    }
    return true;
  }
};
} // namespace

char MyPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void MultiCoveragePass(const PassManagerBuilder &,
                           legacy::PassManagerBase &PM) {
  PM.add(new MyPass());
}
static RegisterStandardPasses
    RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, MultiCoveragePass);