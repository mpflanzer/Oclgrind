// UBSan.h (Oclgrind)
// Copyright (c) 2015, Moritz Pflanzer,
// Imperial College London. All rights reserved.
//
// This program is provided under a three-clause BSD license. For full
// license terms please see the LICENSE file distributed with this
// source code.

#include "core/Plugin.h"

namespace oclgrind
{
    class UBSan : public Plugin
    {
        public:
            UBSan(const Context *context) : Plugin(context){};

            virtual void instructionExecuted(const WorkItem *workItem,
                    const llvm::Instruction *instruction,
                    const TypedValue& result) override;
            //virtual void kernelBegin(const KernelInvocation *kernelInvocation) override;
            //virtual void kernelEnd(const KernelInvocation *kernelInvocation) override;
            static long getSignedMinValue(const unsigned int bits);
            static long getSignedMaxValue(const unsigned int bits);

        private:
            void handleSignedAdd(const WorkItem *workItem, const llvm::Instruction *I, TypedValue result) const;
            void logArithmeticException() const;
    };
}
