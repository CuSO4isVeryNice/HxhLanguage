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

    paramTypes.push_back(llvm::Type::getInt32Ty(llvmContext));                            // 添加一个int32参数
    paramTypes.push_back(llvm::PointerType::get(llvm::Type::getInt8Ty(llvmContext), 0));  // 添加一个char*参数

    llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(llvmContext), paramTypes, false);

    llvm::Function* mainFun = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);

    llvm::AllocaInst* frame = builder.CreateAlloca(
        llvm::Type::getInt32Ty(llvmContext), llvm::ConstantInt::get(llvmContext, llvm::APInt(32, mainProc.stackSize), "frame"));

    return 0;
}