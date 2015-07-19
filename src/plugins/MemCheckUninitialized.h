// MemCheckUninitialized.h (Oclgrind)
// Copyright (c) 2015, Moritz Pflanzer
// Imperial College London. All rights reserved.
//
// This program is provided under a three-clause BSD license. For full
// license terms please see the LICENSE file distributed with this
// source code.

#include "core/Plugin.h"
#include "llvm/IR/Function.h"

namespace oclgrind
{
  class MemCheckUninitialized : public Plugin
  {
  public:
      MemCheckUninitialized(const Context *context);

      virtual void kernelBegin(const KernelInvocation *kernelInvocation) override;
      virtual void instructionExecuted(const WorkItem *workItem,
                                       const llvm::Instruction *instruction,
                                       const TypedValue& result) override;

  private:
    llvm::LLVMContext *llvmContext;
    std::map<unsigned, Memory*> ShadowMem;
    mutable MemoryPool m_pool;
    TypedValueMap ShadowMap;
    std::map<const llvm::Function*, std::map<unsigned, TypedValue> > FunctionArgumentMap;
    std::vector<const llvm::CallInst*> CallInstructions;

    TypedValue getCleanShadow(const llvm::Value *V);
    TypedValue getCleanShadow(llvm::Type *Ty);
    TypedValue getPoisonedShadow(const llvm::Value *V);
    TypedValue getPoisonedShadow(llvm::Type *Ty);
    TypedValue getShadow(const llvm::Value *V);
    void setShadow(const llvm::Value *V, TypedValue SV);
    //llvm::Type *getShadowTy(const llvm::Value *V);
    //llvm::Type *getShadowTy(llvm::Type *OrigTy);

    Memory *getMemory(unsigned addrSpace);

    void setShadowMem(unsigned addrSpace, size_t address, TypedValue SM);
    void getShadowMem(unsigned addrSpace, size_t address, TypedValue &SM);

    void SimpleOr(const llvm::Instruction *I);

    void dumpFunctionArgumentMap();
    void dumpShadowMap();
    void dumpShadowMem(unsigned addrSpace);
    void logError(unsigned int addrSpace, size_t address) const;
  };
}