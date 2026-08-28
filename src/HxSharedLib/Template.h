#pragma once
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include <vector>
/*
#include "HXVM/ObjectReader.h"
#include "HXVM/Interpreter.h"
*/
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
    void* memPtr;
    union {
        int32_t i32Value;
        char byteValue;
        float floatValue;
        double doubleValue;
        char boolValue;
        uint16_t unicodeValue;
        void* addressValue;
    } value;
    enum { TYPE_I32, TYPE_BYTE, TYPE_FLOAT, TYPE_DOUBLE, TYPE_BOOL, TYPE_UNI_CHAR, TYPE_ADDR } type;
    _OpStack opStackParam;
} ArgSym;

typedef struct SharedLibFunArg {
    std::vector<ArgSym> args;
} SharedLibFunArg;
typedef _OpStack (*SharedLibFun)(SharedLibFunArg& arg);

}  // namespace LibFun

extern LibFun::SharedLibFun loadSharedLibFunction(const char* libPath, const char* funcName);

LibFun::SharedLibFun loadSharedLibFunction(void* handle, const char* funcName) {
    if (!handle) {
        return nullptr;
    }

    void* funcPtr = dlsym(handle, funcName);
    if (!funcPtr) {
        return nullptr;
    }

    return reinterpret_cast<LibFun::SharedLibFun>(funcPtr);
}
