#ifndef HXHLANG_SRC_HXVM_OBJECT_READER_H
#define HXHLANG_SRC_HXVM_OBJECT_READER_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <wchar.h>

#include <string>

#include "HxVector.h"
typedef uint8_t Opcode;
enum {
    OP_NOP = 0,
    OP_LOAD_CONST,                   // 加载常量至栈顶 OP_LOAD_CONST <paramType> <paramValue> 或 OP_LOAD_CONST <constantIndex>
    OP_LOAD_VAR,                     // 加载变量至栈顶  LOAD_VAR <offest(u32)> <size(u32)(type为压栈后槽位标记的类型))>
    OP_STORE_ARRAY_ELEMENT,          // 将栈顶值存入数组元素, 索引用栈顶  STORE_ARRAY_ELEMENT <offest(u32)> <size(按u32读>
    OP_LOAD_ELEMENT_FROM_ARRAY,      // 加载数组元素至栈顶， 索引用栈顶  LOAD_ELEMENT_FROM_ARRAY <offest(u32)>
                                     // <size(按u32读，type为压栈后槽位标记的类型)>
    OP_LOAD_VARIABLE_FROM_ADDRESS,   // 1,读取并弹出次栈顶中的地址，2、加上偏移量(param[0]) 3、压栈
    OP_STORE_VARIABLE_FROM_ADDRESS,  // 1,读取并弹出栈顶中的值，2、读取params[0],作为次栈值存的地址的偏移,
    // 3、读params[1]的value(u32),作为size4、传值给(次栈值存的地址+params[0].value)
    OP_POP,        // 弹出
    OP_STORE_VAR,  // 将栈顶值存入变量  OP_STORE_VAR <offest(u32)>
    // <copySize(u32, type表示栈顶应转换的类型)>

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,   // 次栈顶除栈顶
    OP_EQU,   // 必须知道两参数的类型
    OP_NEQU,  // 必须知道两参数的类型
    OP_GT,    // 次栈顶 > 栈顶  -> true
    OP_LT,    // 次栈顶 < 栈顶  -> true
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_AND_LOGIC,
    OP_OR_LOGIC,
    OP_NOT_LOGIC,
    OP_INC,  // INC <offest> <varType>
    OP_DEC,

    OP_JMP,            // OP_JMP <instAddr(u32)>
    OP_JMP_CONDITION,  // JMP_CONDITION <栈顶为真时跳转的地址(index u32)> <为假时跳转的地址(index u32)>
    OP_CAL,            // CAL <procIndex>(u32) <paramCount>(u32)
    OP_RET,
    OP_PRINT_STRING,
    // 类型转换
    OP_CHAR_TO_INT,
    OP_INT_TO_CHAR,
    OP_INT_TO_FLOAT,
    OP_CHAR_TO_FLOAT,
    OP_CHAR_TO_STRING,
    OP_FLOAT_TO_INT,
    OP_INT_TO_STRING,
    // 连接字符串
    OP_STRING_CONCAT,

    OP_HEAP_ALLOC,  // OP_HEAP_ALLOC size(u32)：分配内存的地址放栈顶
    OP_CAL_NATIVE,  // OP_CAL_NATIVE funName(常量池索引，u32) argCount(u32)
};
typedef uint8_t ParamType;
enum {
    PARAM_TYPE_INT = 0,
    PARAM_TYPE_FLOAT,  // double
    PARAM_TYPE_CHAR,
    PARAM_TYPE_BOOL,
    PARAM_TYPE_STRING,
    PARAM_TYPE_ADDRESS,
    PARAM_TYPE_INDEX,   // uint32_t 索引常量池或过程表
    PARAM_TYPE_SIZE,    // u32
    PARAM_TYPE_OFFEST,  // u32
};
typedef struct Param {
    ParamType type;  // char
    uint8_t size;
    char value[8];
    uint32_t offest;  // 偏移量
} Param;
// 指令
typedef struct Instruction {
    Opcode opcode;  // char
    Param params[3];
} Instruction;
// 过程,用索引访问
typedef struct Procedure {
    uint32_t instructionSize;
    HxVector<Instruction> instructions;
    uint32_t stackSize;     // 栈大小
    uint32_t localVarSize;  // 局部变量数量
} Procedure;
//------------------------------------
// 常量池
enum ConstantType {
    CONST_STRING,
    CONST_ASCII_STRING,
};
typedef struct Constant {
    ConstantType type;  // char, 1字节
    uint32_t size;      // 真实大小，不是字符串长度
    union {
        wchar_t* stringValue;
        char* asciiString;
    } value;
} Constant;
typedef struct ConstantPool {
    uint32_t size;
    Constant* constants;
    HxVector<char*> libNameList;  // 所需动态库
} ConstantPool;
//-----------------------------------
typedef struct SharedLibFile {
    char* asciiName;  // 指向常量池中的字符串
    uint64_t size;
    char* data;  // 在解释阶段实际上没啥用
} SharedLibFile;
//----------------------------------
typedef struct ObjectCodeHeader {
    char magic[4];  // 魔数 "HXOC"
    float version;
} ObjectCodeHeader;
//--------------------------------------
typedef struct ObjectCode {
    ObjectCodeHeader header;
    unsigned char isLibPacked;
    ConstantPool constantPool;
    uint32_t procedureSize;
    HxVector<Procedure> procedures;
    int32_t start;  // 入口索引

    HxVector<SharedLibFile> sharedLibFileList;
} ObjectCode;

// 平台检测函数
static int isAndroidPlatform(void) {
    // Android 环境通常设置 ANDROID_ROOT 环境变量
    return getenv("ANDROID_ROOT") != NULL;
}
inline static char* readString(FILE* file);
inline static SharedLibFile readPackedLib(FILE* file) {
    SharedLibFile lib = {};
    lib.asciiName = readString(file);
    if (fread(&lib.size, sizeof(uint64_t), 1, file) != 1) {
        fwprintf(errorStream, ERR_LABEL L"读动态库大小时发生错误！\n");
        return lib;
    }
    lib.data = (char*)malloc(lib.size);
    if (!lib.data) {
        fwprintf(errorStream, ERR_LABEL L"分配动态库数据内存失败！\n");
        return lib;
    }
    if (fread(lib.data, 1, lib.size, file) != lib.size) {
        fwprintf(errorStream, ERR_LABEL L"读动态库数据时发生错误！\n");
        free(lib.data);
        lib.data = nullptr;
        return lib;
    }
    return lib;
}

inline static wchar_t* readWstring(FILE* file) {
    uint32_t byteLen;
    if (fread(&byteLen, sizeof(byteLen), 1, file) != 1) {
        fwprintf(errorStream, ERR_LABEL L"读字符串时发生错误！\n");
        return nullptr;
    }
    if (byteLen == 0) return nullptr;

    uint32_t charCount = byteLen / sizeof(uint16_t);

    uint16_t* buf = (uint16_t*)malloc(byteLen + sizeof(uint16_t));
    if (!buf) return nullptr;

    if (fread(buf, sizeof(uint16_t), charCount, file) != charCount) {
        free(buf);
        return nullptr;
    }

    wchar_t* wstr = (wchar_t*)calloc(charCount + 1, sizeof(wchar_t));
    if (!wstr) {
        free(buf);
        return nullptr;
    }

    for (uint32_t i = 0; i < charCount; i++) {
        wstr[i] = (wchar_t)buf[i];
    }

    free(buf);
    return wstr;
}
inline static char* readString(FILE* file) {
    uint32_t byteLen;
    if (fread(&byteLen, sizeof(byteLen), 1, file) != 1) {
        fwprintf(errorStream, ERR_LABEL L"读字符串时发生错误！\n");
        return nullptr;
    }
    if (byteLen == 0) return nullptr;

    uint32_t charCount = byteLen / sizeof(char);

    char* buf = (char*)calloc(charCount, sizeof(char));
    if (!buf) return nullptr;

    if (fread(buf, sizeof(char), charCount, file) != charCount) {
        free(buf);
        return nullptr;
    }

    return buf;
}
// 读取指令
inline static int readInstruction(Instruction& instr, FILE* file) {
    // 读取 Opcode
    if (fread(&(instr.opcode), sizeof(Opcode), 1, file) != 1) return -1;
    // 读取 3 个参数
    for (int i = 0; i < 3; i++) {
        char typeChar;
        if (fread(&typeChar, sizeof(char), 1, file) != 1) return -1;
        instr.params[i].type = (ParamType)typeChar;

        if (fread(&(instr.params[i].size), sizeof(uint8_t), 1, file) != 1) return -1;
        if (fread(instr.params[i].value, 1, 8, file) != 8) return -1;
        if (fread(&(instr.params[i].offest), sizeof(uint32_t), 1, file) != 1) return -1;
    }
    return 0;
}
inline int readObjectCode(FILE* file, ObjectCode& obj) {
    if (!file) return -1;

    // 验证头信息 "HXOC"
    char magic[4];
    if (fread(magic, 1, 4, file) != 4) {
        fwprintf(errorStream, ERR_LABEL L"读魔数时发生错误！\n");
        fclose(file);
        return -1;
    }
    if (magic[0] != 'H' || magic[1] != 'X' || magic[2] != 'O' || magic[3] != 'C') {
        fclose(file);
        return -1;
    }
    float version = 0.0f;
    if (fread(&version, sizeof(float), 1, file) != 1) {
        fwprintf(errorStream, ERR_LABEL L"读版本号时发生错误！\n");
        fclose(file);
        return -1;
    }
    if (version > HXVM_VERSION) {
        fwprintf(errorStream, ERR_LABEL L"本虚拟机版本过低，文件要求：%f\n", version);
        fclose(file);
        return -1;
    }

    // isLibPacked
    if (fread(&obj.isLibPacked, sizeof(unsigned char), 1, file) != 1) {
        fwprintf(errorStream, ERR_LABEL L"读打包原生库标记时发生错误！\n");
        fclose(file);
        return -1;
    }

    // 读取常量池
    if (fread(&(obj.constantPool.size), sizeof(uint32_t), 1, file) != 1) {
        fwprintf(errorStream, ERR_LABEL L"读常量池时发生错误！\n");
        fclose(file);
        return -1;
    }
    obj.constantPool.constants = (Constant*)malloc(sizeof(Constant) * obj.constantPool.size);
    for (uint32_t i = 0; i < obj.constantPool.size; i++) {
        char typeChar;
        if (fread(&typeChar, sizeof(char), 1, file) != 1) {
            fwprintf(errorStream, ERR_LABEL L"读常量池的type时发生错误！\n");
            fclose(file);
            return -1;
        }

        obj.constantPool.constants[i].type = (ConstantType)typeChar;
        if (obj.constantPool.constants[i].type == CONST_STRING) {
            // 将 u16 序列读入并转为 wchar_t*
            obj.constantPool.constants[i].value.stringValue = readWstring(file);
            // 重新记录当前平台的真实字节大小
            if (obj.constantPool.constants[i].value.stringValue) {
                obj.constantPool.constants[i].size =
                    (uint32_t)(wcslen(obj.constantPool.constants[i].value.stringValue) * sizeof(wchar_t));
            }
        } else if (obj.constantPool.constants[i].type == CONST_ASCII_STRING) {
            obj.constantPool.constants[i].value.asciiString = readString(file);
        }
    }
    // 读动态库路径
    if (!obj.isLibPacked) {
        uint32_t libNameListSize = 0;
        if (fread(&libNameListSize, sizeof(uint32_t), 1, file) != 1) {
            fclose(file);
            return -1;
        }
#ifdef HX_DEBUG
        wprintf(LOG_LABEL L"libNameListSize: %u\n", libNameListSize);
#endif
        for (int i = 0; i < libNameListSize; i++) {
            char* libName = readString(file);
            if (libName) obj.constantPool.libNameList.push_back(libName);
        }
#ifdef HX_DEBUG
        wprintf(LOG_LABEL L"libNameList.size(): %u\n", obj.constantPool.libNameList.size());
#endif
    }
    // 读取过程
    uint32_t procCount;
    if (fread(&procCount, sizeof(uint32_t), 1, file) != 1) {
        fclose(file);
        return -1;
    }
    obj.procedureSize = procCount;
    obj.procedures.resize(procCount);
    for (uint32_t i = 0; i < procCount; i++) {
        Procedure& proc = obj.procedures[i];
        if (fread(&(proc.instructionSize), sizeof(uint32_t), 1, file) != 1) {
            fclose(file);
            return -1;
        }
        for (uint32_t j = 0; j < proc.instructionSize; j++) {
            Instruction instr;
            if (readInstruction(instr, file) != 0) {
                fclose(file);
                return -1;
            }
            proc.instructions.push_back(instr);
        }
        // 读取运行栈
        if (fread(&(proc.stackSize), sizeof(uint32_t), 1, file) != 1) {
            fclose(file);
            return -1;
        }
    }
    // 读取入口
    if (fread(&(obj.start), sizeof(uint32_t), 1, file) != 1) {
        fclose(file);
        return -1;
    }
    if (obj.isLibPacked) {
        uint32_t sharedLibCount = 0;
        if (fread(&sharedLibCount, sizeof(uint32_t), 1, file) != 1) {
            fclose(file);
            return -1;
        }
        for (uint32_t i = 0; i < sharedLibCount; i++) {
            SharedLibFile lib = readPackedLib(file);
            // obj.sharedLibFileList.push_back(lib);
            if (isAndroidPlatform()) {
                // 必须是Termux环境
                const char* homeDir = getenv("HOME");
                if (!homeDir) {
                    fwprintf(errorStream, ERR_LABEL L"无法获取HOME环境变量！\n");
                    fclose(file);
                    return -1;
                }
                char* tmpPath =
                    (char*)calloc(strlen(homeDir) + strlen("/HxlangTmpSharedLib/") + strlen(lib.asciiName) + 2, sizeof(char));
                strcpy(tmpPath, homeDir);
                strcat(tmpPath, "/HxlangTmpSharedLib/");
                #ifdef _WIN32
                mkdir(tmpPath);
                #else
                mkdir(tmpPath, 0755);
                #endif
                strcat(tmpPath, lib.asciiName);
                FILE* tmpFile = fopen(tmpPath, "wb");
                if (!tmpFile) {
                    fwprintf(errorStream, ERR_LABEL L"无法创建临时动态库文件：%s\n", tmpPath);
                    free(tmpPath);
                    fclose(file);
                    return -1;
                }
                if (fwrite(lib.data, 1, lib.size, tmpFile) != lib.size) {
                    fwprintf(errorStream, ERR_LABEL L"写入临时动态库文件失败：%s\n", tmpPath);
                    free(tmpPath);
                    fclose(tmpFile);
                    fclose(file);
                    return -1;
                }
                fclose(tmpFile);
                free(lib.data);
                lib.data = nullptr;
                obj.sharedLibFileList.push_back(lib);
            } else {
                FILE* tmpFile = fopen(lib.asciiName, "wb");
                if (!tmpFile) {
                    fwprintf(errorStream, ERR_LABEL L"无法创建动态库文件：%s\n", lib.asciiName);
                    fclose(file);
                    return -1;
                }
                if (fwrite(lib.data, 1, lib.size, tmpFile) != lib.size) {
                    fwprintf(errorStream, ERR_LABEL L"写入动态库文件失败：%s\n", lib.asciiName);
                    fclose(tmpFile);
                    fclose(file);
                    return -1;
                }
                fclose(tmpFile);
                free(lib.data);
                lib.data = nullptr;
                obj.sharedLibFileList.push_back(lib);
            }
        }
    }
    fclose(file);
    return 0;
}
inline void freeObjectCode(ObjectCode& obj) {
    // 释放常量池里的每个字符串
    if (obj.constantPool.constants != nullptr) {
        for (uint32_t i = 0; i < obj.constantPool.size; i++) {
            if (obj.constantPool.constants[i].type == CONST_STRING) {
                if (obj.constantPool.constants[i].value.stringValue != nullptr) {
                    free(obj.constantPool.constants[i].value.stringValue);
                    obj.constantPool.constants[i].value.stringValue = nullptr;
                }
            } else if (obj.constantPool.constants[i].type == CONST_ASCII_STRING) {
                if (obj.constantPool.constants[i].value.asciiString != nullptr) {
                    free(obj.constantPool.constants[i].value.asciiString);
                    obj.constantPool.constants[i].value.asciiString = nullptr;
                }
            }
        }
        free(obj.constantPool.constants);
        obj.constantPool.constants = nullptr;
    }
    return;
}
#endif