// ArithmeticExceptions.h (Oclgrind)
// Copyright (c) 2016, Moritz Pflanzer,
// Imperial College London. All rights reserved.
//
// This program is provided under a three-clause BSD license. For full
// license terms please see the LICENSE file distributed with this
// source code.

// The recommendations how to detect undefined behaviour haven been adopted from:
// https://www.securecoding.cert.org/confluence/display/c/INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow
// https://www.securecoding.cert.org/confluence/display/c/INT33-C.+Ensure+that+division+and+remainder+operations+do+not+result+in+divide-by-zero+errors
// Overflow in unary minus operator cnnot be checked in LLVM IR
// Oversize shifts are well-defined in OpenCL

#include "core/common.h"
#include "core/Context.h"
#include "core/WorkItem.h"

#include <sstream>

#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"

#include "ArithmeticExceptions.h"

#include "core/Kernel.h"
#include "core/KernelInvocation.h"

using namespace oclgrind;
using namespace std;

static long getSignedMinValue(const unsigned int bits)
{
    return -(1L << (bits - 2)) - (1L << (bits - 2));
}

static long getSignedMaxValue(const unsigned int bits)
{
    return ((1L << (bits - 2)) - 1L) + (1L << (bits - 2));
}

void ArithmeticExceptions::instructionExecuted(
  const WorkItem *workItem, const llvm::Instruction *instruction,
  const TypedValue& result)
{
    switch(instruction->getOpcode())
    {
        case llvm::Instruction::Add:
        {
            const auto&& BinOp = llvm::dyn_cast<llvm::BinaryOperator>(instruction);

            // Check for signed overflow
            // Report an exception iff LLVM would create a poisoned value on overflow
            // Other than that it is not possible to differentiate between signed/unsigned values
            if(BinOp->hasNoSignedWrap())
            {
                const TypedValue LHS = workItem->getOperand(instruction->getOperand(0));
                const TypedValue RHS = workItem->getOperand(instruction->getOperand(1));
                const unsigned int ResultWidth = BinOp->getType()->getScalarSizeInBits();

                for(unsigned int i = 0; i < result.num; ++i)
                {
                    if(((RHS.getSInt(i) > 0) && (LHS.getSInt(i) > (getSignedMaxValue(ResultWidth) - RHS.getSInt(i)))) ||
                       ((RHS.getSInt(i) < 0) && (LHS.getSInt(i) < (getSignedMinValue(ResultWidth) - RHS.getSInt(i)))))
                    {
                        logArithmeticException();
                    }
                }
            }

            break;
        }

        //case llvm::Instruction::FAdd:
        //case llvm::Instruction::FDiv:
        //case llvm::Instruction::FMul:
        //case llvm::Instruction::FPExt:
        //case llvm::Instruction::FPToSI:
        //case llvm::Instruction::FPToUI:
        //case llvm::Instruction::FPTrunc:
        //case llvm::Instruction::FRem:
        //case llvm::Instruction::FSub:

        case llvm::Instruction::Mul:
        {
            const auto&& BinOp = llvm::dyn_cast<llvm::BinaryOperator>(instruction);

            // Check for signed overflow
            // Report an exception iff LLVM would create a poisoned value on overflow
            // Other than that it is not possible to differentiate between signed/unsigned values
            if(BinOp->hasNoSignedWrap())
            {
                const TypedValue LHS = workItem->getOperand(instruction->getOperand(0));
                const TypedValue RHS = workItem->getOperand(instruction->getOperand(1));
                const unsigned int ResultWidth = BinOp->getType()->getScalarSizeInBits();

                for(unsigned int i = 0; i < result.num; ++i)
                {
                    if(LHS.getSInt(i) > 0) {  /* LHS is positive */
                        if(RHS.getSInt(i) > 0) {  /* LHS and RHS are positive */
                            if(LHS.getSInt(i) > (getSignedMaxValue(ResultWidth) / RHS.getSInt(i)))
                            {
                                logArithmeticException();
                            }
                        } else { /* LHS positive, RHS nonpositive */
                            if(RHS.getSInt(i) < (getSignedMinValue(ResultWidth) / LHS.getSInt(i)))
                            {
                                logArithmeticException();
                            }
                        }
                    } else { /* LHS is nonpositive */
                        if(RHS.getSInt(i) > 0) { /* LHS is nonpositive, RHS is positive */
                            if(LHS.getSInt(i) < (getSignedMinValue(ResultWidth) / RHS.getSInt(i)))
                            {
                                logArithmeticException();
                            }
                        } else { /* LHS and RHS are nonpositive */
                            if((LHS.getSInt(i) != 0) && (RHS.getSInt(i) < (getSignedMaxValue(ResultWidth) / LHS.getSInt(i))))
                            {
                                logArithmeticException();
                            }
                        }
                    }
                }
            }

            break;
        }
        case llvm::Instruction::PtrToInt:
        {
            const auto&& PtrToIntInst = llvm::dyn_cast<llvm::PtrToIntInst>(instruction);

            // Check for signed overflow and division by zero
            const TypedValue PtrValue = workItem->getOperand(PtrToIntInst->getPointerOperand());
            const unsigned int ResultWidth = PtrToIntInst->getDestTy()->getScalarSizeInBits();

            for(unsigned int i = 0; i < result.num; ++i)
            {
                //TODO: Is this check ok?
                if((uintptr_t)PtrValue.getPointer(i) > (uintptr_t)getSignedMaxValue(ResultWidth))
                {
                    logArithmeticException();
                }
            }

            break;
        }
        case llvm::Instruction::SDiv:
        {
            const auto&& BinOp = llvm::dyn_cast<llvm::BinaryOperator>(instruction);

            // Check for signed overflow and division by zero
            const TypedValue LHS = workItem->getOperand(instruction->getOperand(0));
            const TypedValue RHS = workItem->getOperand(instruction->getOperand(1));
            const unsigned int ResultWidth = BinOp->getType()->getScalarSizeInBits();

            for(unsigned int i = 0; i < result.num; ++i)
            {
                if((RHS.getSInt(i) == 0) || ((LHS.getSInt(i) == getSignedMinValue(ResultWidth)) && (RHS.getSInt(i) == -1)))
                {
                    logArithmeticException();
                }
            }

            break;
        }

        //case llvm::Instruction::SIToFP:

        case llvm::Instruction::SRem:
        {
            const auto&& BinOp = llvm::dyn_cast<llvm::BinaryOperator>(instruction);

            // Check for signed overflow and division by zero
            const TypedValue LHS = workItem->getOperand(instruction->getOperand(0));
            const TypedValue RHS = workItem->getOperand(instruction->getOperand(1));
            const unsigned int ResultWidth = BinOp->getType()->getScalarSizeInBits();

            for(unsigned int i = 0; i < result.num; ++i)
            {
                if((RHS.getSInt(i) == 0) || ((LHS.getSInt(i) == getSignedMinValue(ResultWidth)) && (RHS.getSInt(i) == -1)))
                {
                    logArithmeticException();
                }
            }

            break;
        }
        case llvm::Instruction::Sub:
        {
            const auto&& BinOp = llvm::dyn_cast<llvm::BinaryOperator>(instruction);

            // Check for signed overflow
            // Report an exception iff LLVM would create a poisoned value on overflow
            // Other than that it is not possible to differentiate between signed/unsigned values
            if(BinOp->hasNoSignedWrap())
            {
                const TypedValue LHS = workItem->getOperand(instruction->getOperand(0));
                const TypedValue RHS = workItem->getOperand(instruction->getOperand(1));
                const unsigned int ResultWidth = BinOp->getType()->getScalarSizeInBits();

                for(unsigned int i = 0; i < result.num; ++i)
                {

                    if((RHS.getSInt(i) > 0 && LHS.getSInt(i) < getSignedMinValue(ResultWidth) + RHS.getSInt(i)) ||
                       (RHS.getSInt(i) < 0 && LHS.getSInt(i) > getSignedMaxValue(ResultWidth) + RHS.getSInt(i)))
                    {
                        logArithmeticException();
                    }
                }
            }

            break;
        }
        case llvm::Instruction::UDiv:
        {
            // Check for division by zero
            const TypedValue RHS = workItem->getOperand(instruction->getOperand(1));

            for(unsigned int i = 0; i < result.num; ++i)
            {
                if(RHS.getSInt(i) == 0)
                {
                    logArithmeticException();
                }
            }

            break;
        }

        //case llvm::Instruction::UIToFP:

        case llvm::Instruction::URem:
        {
            // Check for division by zero
            const TypedValue RHS = workItem->getOperand(instruction->getOperand(1));

            for(unsigned int i = 0; i < result.num; ++i)
            {
                if(RHS.getSInt(i) == 0)
                {
                    logArithmeticException();
                }
            }

            break;
        }
        case llvm::Instruction::Unreachable:
            FATAL_ERROR("Encountered unreachable instruction");
    }
}

void ArithmeticExceptions::logArithmeticException() const
{
  Context::Message msg(WARNING, m_context);
  msg << "Undefined behaviour due to an arithmetic exception" << endl
      << msg.INDENT
      << "Kernel: " << msg.CURRENT_KERNEL << endl
      << "Entity: " << msg.CURRENT_ENTITY << endl
      << msg.CURRENT_LOCATION << endl;
  msg.send();
}

