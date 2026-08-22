//
// Created by cuso4 on 2026/8/22.
//
#include "TestLib.h"

#include <stdio.h>
#include <wchar.h>
extern "C" {
_OpStack libFunTest(SharedLibFunArg args) {
    fwprintf(stdout, L"Hello from C++ shared lib!\n");
    _OpStack retVal;
    retVal.type = TYPE_INT;
    return retVal;
}
}