#pragma once
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "Lexer.h"
#include "Token.h"

class Class;

class Type {
   public:
    bool isRef = false;
    enum { UNKOWN, DOUBLE, INT, LONG, LONG_DOUBLE, FLOAT, CHAR, BOOLEAN, STRING } basicType;
    int classIndex = -1;  //-1表示基本类型

    Type() : basicType(UNKOWN) {}

    bool operator!=(const Type& other) const {
        return classIndex != other.classIndex || basicType != other.basicType ||
               isRef != other.isRef;
    }
};

class Variable;
class Function;
class Class {
   public:
    std::wstring name;
    class Member {
       public:
        union {
            Variable* memVar;
            Function* memFun;
        } mem;
        enum { VAR_MEM, FUN_MEM } memType = VAR_MEM;
        enum { PUB, PRI, PTD } memAcsPermisson = PUB;
        Member() {
            memset(&mem, 0, sizeof(mem));
        }
    };
    std::wstring parentName;
    int parentIndex = -1;
    std::vector<Member> members;
    //暂时存的类体， 到后面分析member后直接clean
    std::vector<Token> body;
    ~Class();
};

class Variable {
   public:
    Type type;
    std::wstring name;
    bool isOnlyRead;

    Variable() : name(L""), isOnlyRead(0), type() {}
};

class Function {
   public:
    std::wstring name;
    std::vector<Variable> argList;
    Type retType;
    std::vector<Token> body;
    bool isDefined = false;
};

inline Class::~Class() {
    for (auto& member : members) {
        if (member.memType == Member::FUN_MEM) {
            delete member.mem.memFun;
            member.mem.memFun = nullptr;
        } else {
            delete member.mem.memVar;
            member.mem.memVar = nullptr;
        }
    }
}

class Program {
   public:
    std::vector<Token>* tokenArr;
    std::unique_ptr<llvm::Module> llvmModule;
    std::vector<Function> globalFunctionArr;
    std::vector<Class> classArr;

    Program(std::string moduleName, std::vector<Token>* _tokenArr)
        : llvmModule(std::make_unique<llvm::Module>(moduleName, globalLLVMContext)),
          tokenArr(_tokenArr) {
        if (isDebugMode) std::wcout << L"创建Module -> 完成!" << std::endl;
    }
    // 分析函数定义
    int parseFunDef(std::vector<Token>& sourceTokens, int& i, Function& fun,
                    bool registerGlobal = true);
    // 从Token流生成类数组
    int tokenArrToClsArr();
    // 从Token流生成函数数组
    int tokenArrToFunArr();

    void debugPrintFunList();
    void debugPrintClsList();
};

static int parseTypeFromTypeName(Type& type, int& index, std::vector<Token>& tokenArr,
                                 std::vector<Class>& clsArr);

#include "ParserCls.h"
#include "ParserFun.h"