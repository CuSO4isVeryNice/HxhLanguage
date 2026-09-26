#pragma once
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"

extern int optimizeLLVMModule(llvm::Module* module, llvm::FunctionAnalysisManager& FAM, llvm::LoopAnalysisManager& LAM,
                              llvm::CGSCCAnalysisManager& CGAM, llvm::ModuleAnalysisManager& MAM);

int optimizeLLVMModule(llvm::Module* module, llvm::FunctionAnalysisManager& FAM, llvm::LoopAnalysisManager& LAM,
                       llvm::CGSCCAnalysisManager& CGAM, llvm::ModuleAnalysisManager& MAM) {
    // 创建优化器
    llvm::PassBuilder passBuilder;

    // 注册并交叉引用分析代理喵
    passBuilder.registerModuleAnalyses(MAM);
    passBuilder.registerCGSCCAnalyses(CGAM);
    passBuilder.registerFunctionAnalyses(FAM);
    passBuilder.registerLoopAnalyses(LAM);
    passBuilder.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    llvm::ModulePassManager modulePassManager = passBuilder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
    
    modulePassManager.run(*module, MAM);
    return 0;
}
