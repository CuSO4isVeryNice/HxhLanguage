#include <wchar.h>

#include <string>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#ifndef errorStream
#define errorStream stdout
#endif
#define logStream stdout
#define ERR_LABEL L"\33[1;31m[E]\33[0m"
#define LOG_LABEL L"\33[1;33m[LOG]\33[0m"
#define INFO_LABEL L"\33[1;34m[INFO]\33[0m"
#define HXVM_VERSION 0.114f

#include "../HXVM/ObjectReader.h"
#include "HxProcToLLVMFunction.h"

extern void initLocale(void);

int main(int argc, char* argv[]) {
    initLocale();
    std::string inputFilePath = "";
    if (argc > 1) {
        inputFilePath = argv[1];
    } else {
        fwprintf(errorStream, ERR_LABEL L"连文件路径都没有喵，真是个杂鱼呢～\n");
        return 1;
    }

    ObjectCode obj = {};
    if (readObjectCode(fopen(inputFilePath.c_str(), "rb"), obj) != 0) {
        fwprintf(errorStream, ERR_LABEL L"读取文件失败了喵：%s\n", inputFilePath.c_str());
        return 1;
    }
    fwprintf(logStream, LOG_LABEL L"读取文件成功喵：%s\n", inputFilePath.c_str());

    llvm::LLVMContext llvmContext;
    llvm::Module* HxModule = new llvm::Module("hxvm_aot_module", llvmContext);
    llvm::IRBuilder<> Builder(llvmContext);

    int err = hxMainProcToLLVMFunction(llvmContext, HxModule, Builder, obj);

    freeObjectCode(obj);
    return 0;
}
void initLocale(void) {
    // 设置Locale
    if (!setlocale(LC_ALL, "zh_CN.UTF-8")) {
        if (!setlocale(LC_ALL, "en_US.UTF-8")) {
            setlocale(LC_ALL, "C.UTF-8");
        }
    }
    // 设置宽字符流的定向
    fwide(stdout, 1);  // 1 = 宽字符定向
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
#endif
    return;
}