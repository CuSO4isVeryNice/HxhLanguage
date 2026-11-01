#include <locale.h>
#include <wchar.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

bool isDebugMode = true;
#define ERR_LABEL L"\33[1;31m[E]\33[0m "
#define INFO_LABEL L"\33[1;34m[I]\33[0m "

static llvm::LLVMContext globalLLVMContext;
static llvm::IRBuilder<> builder(globalLLVMContext);

#include "Lexer.h"
#include "Scanner.h"
#include "Token.h"
#include "Parser.h"

// 词法分析
extern int lex(std::wstring& sourceCode, std::vector<Token>& tokenArr);

int main(int argc, char** argv) {
    std::locale::global(std::locale("en_US.UTF-8"));
    std::wcout << L"[START]" << std::endl;
    std::string sourcePath = "../Test/test.hxh";

    //读文件
    std::wstring sourceCode;
    wchar_t* sourceCodeWcharPtr = nullptr;
    int scanError = readSourceFile(fopen(sourcePath.c_str(), "r"), &sourceCodeWcharPtr);
    if (scanError || sourceCodeWcharPtr == nullptr) {
        std::wcout << ERR_LABEL << L"文件读不了喵~" << std::endl;
        return (int)'e';
    }
    sourceCode = std::wstring(sourceCodeWcharPtr);
    free(sourceCodeWcharPtr);
    sourceCodeWcharPtr = nullptr;
    std::wcout << INFO_LABEL << L"来，让我看看  \33[31m--不要！（即答）\33[0m" << std::endl;
    if (isDebugMode) std::wcout << sourceCode << std::endl;

    //词法分析
    std::vector<Token> tokenArray;
    int lexerError = lex(sourceCode, tokenArray);
    if (lexerError == -1) {
        std::wcout << ERR_LABEL << L"词法分析出错了喵~" << std::endl;
        return (int)'e';
    } else if(lexerError == 255) {   //语法错误
        return (int)'s';
    }
    if (isDebugMode) printTokens(tokenArray);

    int fileNameStartIndex = sourcePath.find_last_of(L'/')+1;
    if(fileNameStartIndex-1 == sourcePath.npos) fileNameStartIndex = sourcePath.find_last_of(L'\\')+1;
    if(fileNameStartIndex-1 == sourcePath.npos) fileNameStartIndex = 0;
    std::string fileName = sourcePath.substr(fileNameStartIndex, sourcePath.size() - fileNameStartIndex);
    Program program(fileName, &tokenArray);

    if(int parseClsErr = program.tokenArrToClsArr()) return parseClsErr;
    if (isDebugMode) program.debugPrintClsList();
    if(int parseFunErr = program.tokenArrToFunArr()) return parseFunErr;
    if (isDebugMode) program.debugPrintFunList();

    return 0;
}
