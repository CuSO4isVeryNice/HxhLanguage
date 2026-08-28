//
// Created by cuso4 on 2026/8/22.
//
#include <stdint.h>

#include <cstdlib>
#include <vector>
#ifndef HXHLANGUAGE_TESTLIB_H
#define HXHLANGUAGE_TESTLIB_H
typedef enum OpStackType {
    TYPE_INT = 1,
    TYPE_FLOAT,  // double
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_ADDRESS,  // size = 4
} StackType;
typedef struct _OpStack {
    OpStackType type;
    int size;
    char value[8];   // type为string时存wchar_t*
    char isTmpHeap;  // 是否是临时分配的堆內存
} _OpStack;
typedef struct Symbol {
    OpStackType type;
    void* address;
} Symbol;
typedef struct ArgSym {
    // 可为null
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
    _OpStack opStackParam;  // 栈中参数
} ArgSym;

typedef struct SharedLibFunArg {
    std::vector<ArgSym> args;
} SharedLibFunArg;

#endif  // HXHLANGUAGE_TESTLIB_H