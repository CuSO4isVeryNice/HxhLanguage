#pragma once
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

extern int hxMainProcToLLVMFunction(llvm::Module* module, llvm::IRBuilder<>& builder, ObjectCode& obj);

// 由于Procedure不提供具体参数，需要从main开始查找call
int hxMainProcToLLVMFunction(llvm::LLVMContext& llvmContext, llvm::Module* module, llvm::IRBuilder<>& builder,
                             ObjectCode& obj) {
    std::vector<llvm::Type*> paramTypes;
    if (obj.start >= obj.procedures.size()) {
        fwprintf(errorStream, ERR_LABEL L"入口索引超出范围：%d\n", obj.start);
        return -1;
    }
    Procedure& mainProc = obj.procedures[obj.start];

    paramTypes.push_back(llvm::Type::getInt32Ty(llvmContext));     // 添加一个int32参数
    paramTypes.push_back(llvm::PointerType::get(llvmContext, 0));  // 添加一个char*参数

    llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(llvmContext), paramTypes, false);

    llvm::Function* mainFun = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);

    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(llvmContext, "entry", mainFun);
    builder.SetInsertPoint(entryBlock);

    // 第一个指令：分配栈内存
    llvm::AllocaInst* stack =
        builder.CreateAlloca(llvm::Type::getInt32Ty(llvmContext), builder.getInt32(mainProc.stackSize), "stack");

    for (int i = 0; i < mainProc.instructions.size(); i++) {
        Instruction inst = mainProc.instructions[i];

        if (inst.opcode == OP_LOAD_CONST) {
            if (i + 1 < mainProc.instructions.size()) {
                i++;
                Instruction nextInst = mainProc.instructions[i];

                if (nextInst.opcode == OP_RET) {
                    switch (inst.params[0].type) {
                        case PARAM_TYPE_INT: {
                            int32_t value = *(int32_t*)inst.params[0].value;
                            builder.CreateRet(builder.getInt32(value));
                            break;
                        }
                        case PARAM_TYPE_FLOAT: {
                            double value = *(double*)inst.params[0].value;
                            llvm::Value* doubleConst = llvm::ConstantFP::get(builder.getDoubleTy(), value);
                            llvm::Value* intValue = builder.CreateFPToSI(doubleConst, builder.getInt32Ty());   //main必须返回int32
                            builder.CreateRet(intValue);
                            break;
                        }
                    }
                }
            }
        }
    }
    return 0;
}