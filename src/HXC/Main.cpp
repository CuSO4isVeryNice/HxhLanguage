/********************************/
/**一千万以内最好的编程语言
***哇哇哇这个这个哇
*********************************/

#include "config.h"

#define HXC_VERSION 0.114f
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <wchar.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include <cstdlib>
#include <string>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define log(msg, ...) fwprintf(logStream, L"\33[33m[DEB]\33[0m" msg L"\n", ##__VA_ARGS__)
FILE* outputStream = NULL;
FILE* logStream = NULL;
FILE* errorStream = NULL;
inline void hxFree(void* ptr) {
    if (ptr == nullptr) return;
    free(ptr);
    ptr = nullptr;
}
std::string getDirectory(const std::string& path) {
    size_t pos = path.find_last_of("/\\"); // 兼容 Windows 和 Linux
    if (pos == std::string::npos) {
#ifdef _WIN32
        return ".\\";
#else
        return "./";
#endif
    }
#ifdef _WIN32
    return path.substr(0, pos)+"\\";
#else
    return path.substr(0, pos)+"/";
#endif
}
#include "Error.h"
#include "Generator.h"
#include "IR.h"
#include "Lexer.h"
#include "Scanner.h"

typedef struct CompileFlag {
    std::string outPath;
    std::string sourcePath;
    unsigned char printVersion;
    unsigned char packSharedLib;
} CompileFlag;

int parseFlags(int argc, char* argv[], CompileFlag& flag) {
    if (argc == 1) {
        return  0;
    } else {
        if (!argv) return -1;
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-version") == 0) {
                flag.printVersion = (unsigned char)1;
            } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0 || strcmp(argv[i], "-output") == 0) {
                if (i+1 < argc) flag.outPath = argv[++i];
                else return -1;
            } else if (strcmp(argv[i], "-pack") == 0 || strcmp(argv[i], "--pack") == 0) {
                flag.packSharedLib = (unsigned char)1;
            } else {
                flag.sourcePath = argv[i];
            }
        }
    }
    return  0;
}

int main(int argc, char* argv[]) {
    try {
        initLocale();
        clock_t start, end;
        start = clock();
        outputStream = stdout;
        logStream = stdout;
        errorStream = stderr;
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_U16TEXT);
        _setmode(_fileno(stderr), _O_U16TEXT);
#endif

        std::string path = "";
        std::string objPath = "out.hxo";
        unsigned char isPackSharedLibInObjFile = 0;
#ifdef HX_DEBUG
        isPackSharedLibInObjFile = 1;
        path = "../test/test.hxl";
        objPath = "../test/out.hxo";
#else
        CompileFlag flag = {};
        if (parseFlags(argc, argv, flag)) {
            fwprintf(errorStream, L"\33[31m[ERR]\33[0m解析编译参数时出错了喵～（X_X)绝对不是hxc的问题！绝对不是！\n");
        }
        path = flag.sourcePath;
        objPath = flag.outPath;
        if (flag.printVersion) {
            fwprintf(outputStream, L"\33[1;34m[INFO]\33[0m hxc的版本是%.2f喵～\n", HXC_VERSION);
            return 0;
        }
        isPackSharedLibInObjFile = flag.packSharedLib;
#endif
        // 读取源文件
        wchar_t* src = NULL;
        FILE* sourceFile = fopen(path.c_str(), "r");
        int scannerError = readSourceFile(sourceFile, &src);
        if (scannerError) {
            fwprintf(outputStream, L"\33[31m[ERR]\33[0m在读取源文件时出错了！\n");
            fwprintf(outputStream, L"\33[31m[ERR]\33[0m编译失败。\n");
            return -1;
        }
#ifdef HX_DEBUG
        fwprintf(outputStream, L"源文件：\n%ls\n", src);
#endif
        if (wcslen(src) == 0) {
            fwprintf(outputStream, L"\33[31m[ERR]\33[0m你的源文件里空空如也！\n");
            fwprintf(outputStream, L"\33[31m[ERR]\33[0m编译失败。\n");
            return 255;
        }
        // 词法分析
        int lexerError = 0;
        fwprintf(outputStream, L"\33[1;34m[INFO]\33[0m正在进行词法分析\n");
        Tokens* tokens = lex(src, &lexerError);
        if (lexerError == 255) {
            fwprintf(errorStream, L"%ls\n", errorMessageBuffer);
            freeTokens(&tokens);
            fwprintf(outputStream, L"\33[31m[ERR]\33[0m编译失败。\n");
            return 255;
        }
        if (lexerError == -1) {
            fwprintf(errorStream, L"\33[31m[ERR]\33[0m内存分配失败！\n");
            freeTokens(&tokens);
            return -1;
        }
        free(src);
        src = NULL;
        fwprintf(outputStream, L"\33[1;34m[INFO]\33[0m词法分析完成\n");
#ifdef HX_DEBUG
        showTokens(tokens);
#endif
        // 中间表示生成
        IR_Program* program = NULL;
        int irError = 0;
        program = generateIR(tokens, &irError);

        if (irError == 255) {
            fwprintf(errorStream, L"%ls\n", errorMessageBuffer);
            fwprintf(outputStream, L"\33[31m[ERR]\33[0m编译失败。\n");
            return 255;
        }
        if (irError == -1) {
            fwprintf(errorStream, L"\33[31m[ERR]\33[0m内存分配失败！\n");
            return -1;
        }
        fwprintf(outputStream, L"\33[1;34m[INFO]\33[0m中间表示生成完成\n");
#ifdef HX_DEBUG
        showIRProgramInfo(program);
#endif

        // 目标代码生成
        fwprintf(outputStream, L"\33[1;34m[INFO]\33[0m正在生成目标代码\n");
        int genError = 0;
        ObjectCode* objCode = generateObjectCode(program, &genError);
        objCode->isLibPacked = isPackSharedLibInObjFile;

        if (genError == 255) {
            fwprintf(errorStream, L"%ls\n", errorMessageBuffer);
            freeIRProgram(&program);
            fwprintf(outputStream, L"\33[31m[ERR]\33[0m编译失败。\n");
            return 255;
        }
        if (genError == -1) {
            fwprintf(errorStream, L"\33[31m[ERR]\33[0m内存分配失败！\n");
            freeIRProgram(&program);
            return -1;
        }
        for (int i = 0; i < program->libPathList.size(); i++) {
            if (program->libPathList[i]) objCode->constantPool.libNameList.push_back(program->libPathList[i]);
        }
#ifdef HX_DEBUG
        log(L"libListSize:%d", objCode->constantPool.libNameList.size());
#endif

        FILE* objFile = fopen(objPath.c_str(), "wb");
        if(writeObjectCode(getDirectory(objPath), objFile, *objCode)) {
            fwprintf(errorStream, L"\33[31m[ERR]\33[0m写入目标文件时出错了喵～（X_X)绝对不是hxc的问题！绝对不是！\n");
            freeIRProgram(&program);
            freeTokens(&tokens);
            freeObjectCode(&objCode);
            return -1;
        }
        freeIRProgram(&program);
        freeTokens(&tokens);
        freeObjectCode(&objCode);
        end = clock();
        fwprintf(outputStream, L"\33[1;34m[INFO]\33[0m编译完成。共耗时%lfs\n", (double)(end - start) / CLOCKS_PER_SEC);
        return 0;
    } catch (std::bad_alloc& e) {
        fwprintf(errorStream, L"\33[31m[ERR]\33[0m内存分配失败！\n");
    } catch (std::exception& e) {
        fwprintf(errorStream, L"\33[31m[ERR]\33[0m标准库抛异常力！\n");
        fwprintf(errorStream, L"\33[31m[ERR]\33[0m异常信息：%hs\n", e.what());
    }
}
//(≧▽≦)