// UBSan.h (Oclgrind)
// Copyright (c) 2015, Moritz Pflanzer,
// Imperial College London. All rights reserved.
//
// This program is provided under a three-clause BSD license. For full
// license terms please see the LICENSE file distributed with this
// source code.

#include "core/common.h"
#include "core/Context.h"
#include "core/WorkItem.h"

#include <sstream>

#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Type.h"

#include "UBSan.h"

#include "core/Kernel.h"
#include "core/KernelInvocation.h"

using namespace oclgrind;
using namespace std;

long UBSan::getSignedMinValue(const unsigned int bits)
{
    return -(1 << (bits - 1));
}

long UBSan::getSignedMaxValue(const unsigned int bits)
{
    return (1 << (bits - 1)) - 1;
}

void UBSan::handleSignedAdd(const WorkItem *workItem, const llvm::Instruction *I, TypedValue result) const
{
    const TypedValue LHS = workItem->getOperand(I->getOperand(0));
    const TypedValue RHS = workItem->getOperand(I->getOperand(1));
    const unsigned int ResultWidth = I->getType()->getScalarSizeInBits();

    for(int i = 0; i < result.num; ++i)
    {
        if(((RHS.getSInt(i) > 0) && (LHS.getSInt(i) > (UBSan::getSignedMaxValue(ResultWidth) - RHS.getSInt(i)))) ||
           ((RHS.getSInt(i) < 0) && (LHS.getSInt(i) < (UBSan::getSignedMinValue(ResultWidth) - RHS.getSInt(i)))))
        {
            logArithmeticException();
        }
    }
}

void UBSan::instructionExecuted(
  const WorkItem *workItem, const llvm::Instruction *instruction,
  const TypedValue& result)
{
    switch(instruction->getOpcode())
    {
        //case llvm::Instruction::Add:
        //case llvm::Instruction::Alloca:
        //case llvm::Instruction::And:
        //case llvm::Instruction::AShr:
        //case llvm::Instruction::BitCast:
        //case llvm::Instruction::Br:
        case llvm::Instruction::Call:
        {
            if(auto *II = llvm::dyn_cast<llvm::IntrinsicInst>(instruction))
            {
                switch (II->getIntrinsicID())
                {
                    case llvm::Intrinsic::sadd_with_overflow:
                    {
                        handleSignedAdd(workItem, II, result);
                        break;
                    }
                }
            }

            break;
        }
        //case llvm::Instruction::ExtractElement:
        //case llvm::Instruction::ExtractValue:
        //case llvm::Instruction::FAdd:
        //case llvm::Instruction::FCmp:
        //case llvm::Instruction::FDiv:
        //case llvm::Instruction::FMul:
        //case llvm::Instruction::FPExt:
        //case llvm::Instruction::FPToSI:
        //case llvm::Instruction::FPToUI:
        //case llvm::Instruction::FPTrunc:
        //case llvm::Instruction::FRem:
        //case llvm::Instruction::FSub:
        //case llvm::Instruction::GetElementPtr:
        //case llvm::Instruction::ICmp:
        //case llvm::Instruction::InsertElement:
        //case llvm::Instruction::InsertValue:
        //case llvm::Instruction::IntToPtr:
        //case llvm::Instruction::Load:
        //case llvm::Instruction::LShr:
        //case llvm::Instruction::Mul:
        //case llvm::Instruction::Or:
        //case llvm::Instruction::PHI:
        //case llvm::Instruction::PtrToInt:
        //case llvm::Instruction::Ret:
        //case llvm::Instruction::SDiv:
        //case llvm::Instruction::Select:
        //case llvm::Instruction::SExt:
        //case llvm::Instruction::Shl:
        //case llvm::Instruction::ShuffleVector:
        //case llvm::Instruction::SIToFP:
        //case llvm::Instruction::SRem:
        //case llvm::Instruction::Store:
        //case llvm::Instruction::Sub:
        //case llvm::Instruction::Switch:
        //case llvm::Instruction::Trunc:
        //case llvm::Instruction::UDiv:
        //case llvm::Instruction::UIToFP:
        //case llvm::Instruction::URem:
        case llvm::Instruction::Unreachable:
            FATAL_ERROR("Encountered unreachable instruction");
        //case llvm::Instruction::Xor:
        //case llvm::Instruction::ZExt:
        //default:
        //    FATAL_ERROR("Unsupported instruction: %s", instruction->getOpcodeName());
    }
}

void UBSan::logArithmeticException() const
{
  Context::Message msg(WARNING, m_context);
  msg << "Undefined behaviour due to an arithmetic exception" << endl
      << msg.INDENT
      << "Kernel: " << msg.CURRENT_KERNEL << endl
      << "Entity: " << msg.CURRENT_ENTITY << endl
      << msg.CURRENT_LOCATION << endl;
  msg.send();
}
