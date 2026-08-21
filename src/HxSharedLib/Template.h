#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <dlfcn.h>
#include <wchar.h>

#include "HXVM/ObjectReader.h"
#include "HXVM/Interpreter.h"

#ifndef errorStream
#define errorStream stdout
#endif
#ifndef ERR_LABEL
#define ERR_LABEL L"\33[1;31m[E]\33[0m"
#endif
#ifndef LOG_LABEL
#define LOG_LABEL L"\33[1;33m[LOG]\33[0m"
#endif
#ifndef INFO_LABEL
#define INFO_LABEL L"\33[1;34m[INFO]\33[0m"
#endif

namespace LibFun {

typedef struct ArgSym {
    // 可为null
    void* memPtr;
    union {
        int32_t i32Value;
        char byteValue;
        float floatValue;
        double doubleValue;
        char boolValue;
        uint16_t unicode;
    };
    enum { TYPE_I32, TYPE_BYTE, TYPE_FLOAT, TYPE_DOUBLE, TYPE_BOOL, TYPE_UNI_CHAR };
    _OpStack* opStackParam; //指向栈中参数
} ArgSym;

typedef struct SharedLibFunArg {
    std::vector<ArgSym> args;
} SharedLibFunArg;

typedef _OpStack (*SharedLibFun)(SharedLibFunArg& arg);

}  // namespace LibFun

extern LibFun::SharedLibFun loadSharedLibFunction(const char* libPath, const char* funcName);

LibFun::SharedLibFun loadSharedLibFunction(const char* libPath, const char* funcName) {
    void* handle = dlopen(libPath, RTLD_LAZY);
    if (!handle) {
        fwprintf(errorStream, ERR_LABEL L"共享库被玩坏了喵: %s\n  err: %s", libPath, dlerror());
        return nullptr;
    }

    void* funcPtr = dlsym(handle, funcName);
    if (!funcPtr) {
        fwprintf(errorStream, ERR_LABEL L"杂鱼～你不会连这个函数都没有吧～: %s\n  err: %s", funcName, dlerror());
        dlclose(handle);
        return nullptr;
    }

    return reinterpret_cast<LibFun::SharedLibFun>(funcPtr);
}