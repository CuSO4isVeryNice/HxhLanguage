#include "TestLib.h"
#include <stdio.h>
#include <wchar.h>
#include <cstdlib>

extern "C" {
    _OpStack printString(SharedLibFunArg args) {
        //fwprintf(stdout, L"argsListSize: %d\n", args.args.size());
        if (args.args.size() >= 1) {
            wchar_t* str = (wchar_t*)((void*)(args.args.at(0).value.addressValue));
            wprintf(L"%ls", str);
        }
        _OpStack retVal;
        retVal.type = TYPE_INT;
        return retVal;
    }
    _OpStack printInt(SharedLibFunArg args) {
        //fwprintf(stdout, L"argsListSize: %d\n", args.args.size());
        if (args.args.size() >= 1) {
            int32_t value = args.args.at(0).value.i32Value;
            fwprintf(stdout, L"%d", value);
        } else {
            fwprintf(stdout, L"printInt: argsListSize < 1\n");
        }
        _OpStack retVal;
        retVal.type = TYPE_INT;
        return retVal;
    }
}
