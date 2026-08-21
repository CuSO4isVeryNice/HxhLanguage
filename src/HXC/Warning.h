#pragma once
#include <locale.h>
#include <stdio.h>
#include <wchar.h>
#define warningStream stdout 

extern void initLocale(void) noexcept;

typedef enum WarningType {
    WARN_NATIVE_LIB_FUN_HAS_BODY,  // 原生库函数不应该有函数体
} WarningType;
extern void setWarning(WarningType w, int warningLine, const wchar_t* warnCode) noexcept;

void setWarning(WarningType w, int warningLine, const wchar_t* warnCode) noexcept {
    initLocale();
    switch (w) {
        case WARN_NATIVE_LIB_FUN_HAS_BODY: {
            fwprintf(warningStream, L"\33[33m[WARN]\33[0m原生库函数不应该有函数体喵(位于第%"
                     L"d行)\n "
                     L"\33[36m[NOTE]\33[0m原生库函数不应该有函数体喵->\33[4m%"
                     L"ls\33[0m\n",
                     warningLine, warnCode ? warnCode : L" ");
            break;
        }
    }
    return;
}