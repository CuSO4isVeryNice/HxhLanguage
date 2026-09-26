#pragma once
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

extern int generateMachineCode(llvm::Module* module, const std::string& outputFilePath);

int generateMachineCode(llvm::Module* module, const std::string& outputFilePath) {
    // 初始化当前机器的目标信息
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    //获取当前系统架构
    std::string targetTripleStr = llvm::sys::getDefaultTargetTriple();
    llvm::Triple targetTriple(targetTripleStr);
    module->setTargetTriple(targetTriple);

    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        llvm::errs() << error;
        return 1;  // 找不到目标架构就直接炸掉喵
    }

    //创建目标机器配置
    auto cpu = "generic";
    auto features = "";
    llvm::TargetOptions targetOptions;
    auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, targetOptions, llvm::Reloc::PIC_);

    module->setDataLayout(targetMachine->createDataLayout());

    // 写入文件喵
    std::error_code errorCode;
    llvm::raw_fd_ostream dest(outputFilePath, errorCode, llvm::sys::fs::OF_None);

    llvm::legacy::PassManager pass;
    auto fileType = llvm::CodeGenFileType::ObjectFile;

    // 绑定生成器并执行输出
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        llvm::errs() << "杂鱼！目标机器不能发出这种类型的文件喵！";
        return 1;
    }
    pass.run(*module);
    dest.flush();
    return 0;
}