#pragma once
/************************
***对象内存布局：
*公有变量成员
*私
*受保护
*******---------
****目标与运行时方法存全局表
*************************/
#include "Error.h"
#include "IR.h"
#include "Lexer.h"
#include "ObjectCode.h"
#include "Parser.h"
#include "SymbolTable.h"

static int getVarSize(IR_DataType type, std::vector<IR_Class*>& class_table);

// 调用此函数必须先调用setClassVarMemSize()
extern int setClassVarMemOffset(IR_Class* cls, std::vector<IR_Class*>& classTable);
extern int setClassVarMemSize(IR_Class* cls, std::vector<IR_Class*>& classTable);
extern int addHeapAllocInst(std::vector<Procedure*>& procTable, std::vector<IR_Class*>& classTable,
                           std::vector<std::vector<SymbolTable>>& symbols);
extern PackedClassVarMem* findVarMemberInClass(const wchar_t* varMemName, IR_Class* cls, std::vector<IR_Class*>& classTable);
extern PackedClassFunMem* findFunInClass(IR_Function& fun, IR_Class* cls, std::vector<IR_Class*>& classTable);
extern IR_Class* getClassByName(const wchar_t* className, std::vector<IR_Class*> classTable);

static int addClassAllocInst(IR_Class* cls, std::vector<Procedure*>& procTable, std::vector<IR_Class*>& classTable,
                             std::vector<std::vector<SymbolTable>>& symbols);
static int generateInstanceAllocInst(int writeStackOffest /*递归调用时设为-1*/, Symbol& instance, IR_Class* cls,
                                     std::vector<IR_Class*>& classTable, std::vector<Instruction>& instList,
                                     int writeHeapOffest=-1);                             
int addHeapAllocInst(std::vector<Procedure*>& procTable, std::vector<IR_Class*>& classTable,
                     std::vector<std::vector<SymbolTable>>& symbols) {
    for (int i = 0; i < classTable.size(); i++) {
        IR_Class* cls = classTable.at(i);
        if (!cls) return -1;
        int err = addClassAllocInst(cls, procTable, classTable, symbols);
        if (err) return -1;
    }
    return 0;
}
static int addClassAllocInst(IR_Class* cls, std::vector<Procedure*>& procTable, std::vector<IR_Class*>& classTable,
                             std::vector<std::vector<SymbolTable>>& symbols) {
    if (!cls) return -1;
    for (int i = 0; i < cls->indexList.size(); i++) {
        SymbolIndex& symbolIndex = cls->indexList.at(i);
        if (symbolIndex.procIndex >= procTable.size() || symbolIndex.procIndex >= symbols.size()) return -1;
        Procedure* proc = procTable.at(symbolIndex.procIndex);
        std::vector<SymbolTable>& funTable = symbols.at(symbolIndex.procIndex);
        if (symbolIndex.symbolTableIndex >= funTable.size()) return -1;
        SymbolTable& localSymbolTable = funTable.at(symbolIndex.symbolTableIndex);
        if (symbolIndex.symbolIndex >= localSymbolTable.vars.size()) return -1;
        Symbol& instance = localSymbolTable.vars.at(symbolIndex.symbolIndex);
        if (instance.isUsed) {
            std::vector<Instruction> insertInstList;
            int err = generateInstanceAllocInst(instance.offest, instance, cls, classTable, insertInstList);
            if (err) return -1;
            // 将insertInstList插入到proc->instructions的开头
            proc->instructions.insert(proc->instructions.begin(), insertInstList.begin(), insertInstList.end());
        }
    }
    return 0;
}
static int generateInstanceAllocInst(int writeStackOffest /*递归调用时设为-1*/, Symbol& instance, IR_Class* cls,
                                     std::vector<IR_Class*>& classTable, std::vector<Instruction>& instList,
                                     int writeHeapOffest) {
    if (!cls) return -1;
    if (writeHeapOffest == -1 && writeStackOffest >= 0) {
        // 1,分配整个类的堆内存
        uint32_t firstAllocSize = (uint32_t)(cls->size);
        Instruction firstHeapAllocInst = {};
        firstHeapAllocInst.opcode = OP_HEAP_ALLOC;
        firstHeapAllocInst.params[0].type = PARAM_TYPE_SIZE;
        firstHeapAllocInst.params[0].size = sizeof(uint32_t);
        memcpy(firstHeapAllocInst.params[0].value, &firstAllocSize, sizeof(uint32_t));
        instList.push_back(firstHeapAllocInst);

        Instruction writeAddrToStackInst = {};
        uint32_t stackOffest = (uint32_t)writeStackOffest;
        writeAddrToStackInst.opcode = OP_STORE_VAR;
        writeAddrToStackInst.params[0].type = PARAM_TYPE_OFFEST;
        writeAddrToStackInst.params[0].size = sizeof(uint32_t);
        memcpy(writeAddrToStackInst.params[0].value, &stackOffest, sizeof(uint32_t));
        uint32_t copySize = 8U;
        writeAddrToStackInst.params[1].type = PARAM_TYPE_ADDRESS;
        writeAddrToStackInst.params[1].size = sizeof(uint32_t);
        memcpy(writeAddrToStackInst.params[1].value, &copySize, sizeof(uint32_t));
        instList.push_back(writeAddrToStackInst);

    } else {
        // 1,压入对象的堆地址
        Instruction pushHeapAddrInst = {};
        pushHeapAddrInst.opcode = OP_LOAD_VAR;
        uint32_t heapOffest = (uint32_t)(instance.offest);
        uint32_t copySize = 8U;
        pushHeapAddrInst.params[0].type = PARAM_TYPE_OFFEST;
        pushHeapAddrInst.params[0].size = sizeof(uint32_t);
        memcpy(pushHeapAddrInst.params[0].value, &heapOffest, sizeof(uint32_t));
        pushHeapAddrInst.params[1].type = PARAM_TYPE_ADDRESS;
        pushHeapAddrInst.params[1].size = sizeof(uint32_t);
        memcpy(pushHeapAddrInst.params[1].value, &copySize, sizeof(uint32_t));
        instList.push_back(pushHeapAddrInst);
        // 2,为类成员分配堆内存
        Instruction memberHeapAllocInst = {};
        memberHeapAllocInst.opcode = OP_HEAP_ALLOC;
        uint32_t memberAllocSize = (uint32_t)(cls->size);
        memberHeapAllocInst.params[0].type = PARAM_TYPE_SIZE;
        memberHeapAllocInst.params[0].size = sizeof(uint32_t);
        memcpy(memberHeapAllocInst.params[0].value, &memberAllocSize, sizeof(uint32_t));
        instList.push_back(memberHeapAllocInst);
        // 3,将分配的堆地址写入对象的成员变量中
        Instruction writeMemberAddrToHeapInst = {};
        writeMemberAddrToHeapInst.opcode = OP_STORE_VARIABLE_FROM_ADDRESS;
        writeMemberAddrToHeapInst.params[0].type = PARAM_TYPE_OFFEST;
        writeMemberAddrToHeapInst.params[0].size = sizeof(uint32_t);
        uint32_t memberHeapOffest = (uint32_t)writeHeapOffest;
        memcpy(writeMemberAddrToHeapInst.params[0].value, &memberHeapOffest, sizeof(uint32_t));
        writeMemberAddrToHeapInst.params[1].type = PARAM_TYPE_SIZE;
        writeMemberAddrToHeapInst.params[1].size = sizeof(uint32_t);
        memcpy(writeMemberAddrToHeapInst.params[1].value, &memberAllocSize, sizeof(uint32_t));
        instList.push_back(writeMemberAddrToHeapInst);
    }
    // 若类的成员中有类类型的，为其分配内存
    for (int i = 0; i < cls->body.publicMembers.size(); i++) {
        if (cls->body.publicMembers.at(i).type != IR_CM_VARIABLE) continue;
        if (cls->body.publicMembers.at(i).data.variable->type.kind == IR_DT_CUSTOM) {
            IR_Class* memberClass =
                getClassByName(cls->body.publicMembers.at(i).data.variable->type.customTypeName, classTable);
            if (!memberClass) return -1;
            int heapOffest = cls->body.publicMembers.at(i).varMemOffest;
            generateInstanceAllocInst(-1, instance, memberClass, classTable, instList, heapOffest);
        }
    }
    for(int i = 0; i < cls->body.privateMembers.size(); i++) {
        if (cls->body.privateMembers.at(i).type != IR_CM_VARIABLE) continue;
        if (cls->body.privateMembers.at(i).data.variable->type.kind == IR_DT_CUSTOM) {
            IR_Class* memberClass =
                getClassByName(cls->body.privateMembers.at(i).data.variable->type.customTypeName, classTable);
            if (!memberClass) return -1;
            int heapOffest = cls->body.privateMembers.at(i).varMemOffest;
            generateInstanceAllocInst(-1, instance, memberClass, classTable, instList, heapOffest);
        }
    }
    for(int i = 0; i < cls->body.protectedMembers.size(); i++) {
        if (cls->body.protectedMembers.at(i).type != IR_CM_VARIABLE) continue;
        if (cls->body.protectedMembers.at(i).data.variable->type.kind == IR_DT_CUSTOM) {
            IR_Class* memberClass =
                getClassByName(cls->body.protectedMembers.at(i).data.variable->type.customTypeName, classTable);
            if (!memberClass) return -1;
            int heapOffest = cls->body.protectedMembers.at(i).varMemOffest;
            generateInstanceAllocInst(-1, instance, memberClass, classTable, instList, heapOffest);
        }
    }

    return 0;
}
IR_Class* getClassByName(const wchar_t* className, std::vector<IR_Class*> classTable) {
    if (className == nullptr) return nullptr;
    for (int i = 0; i < classTable.size(); i++) {
        if (classTable.at(i)->name != nullptr && wcscmp(classTable.at(i)->name, className) == 0) return classTable.at(i);
    }
    return nullptr;
}
int setClassVarMemSize(IR_Class* cls, std::vector<IR_Class*>& classTable) {
    if (!cls) return -1;
#ifdef HX_DEBUG
    log("类：%ls", cls->name);
#endif
    for (int i = 0; i < cls->body.publicMembers.size(); i++) {
        if (cls->body.publicMembers.at(i).type == IR_CM_VARIABLE) {
            cls->body.publicMembers.at(i).varMemSize =
                getVarSize(cls->body.publicMembers.at(i).data.variable->type, classTable);
#ifdef HX_DEBUG
            log(L"setClassVarMemSize->计算变量成员%ls大小：%d", cls->body.publicMembers.at(i).data.variable->name,
                cls->body.publicMembers.at(i).varMemSize);
#endif
        }
    }
    for (int i = 0; i < cls->body.privateMembers.size(); i++) {
        if (cls->body.privateMembers.at(i).type == IR_CM_VARIABLE) {
            cls->body.privateMembers.at(i).varMemSize =
                getVarSize(cls->body.privateMembers.at(i).data.variable->type, classTable);
#ifdef HX_DEBUG
            log(L"setClassVarMemSize->计算变量成员%ls大小：%d", cls->body.privateMembers.at(i).data.variable->name,
                cls->body.privateMembers.at(i).varMemSize);
#endif
        }
    }
    for (int i = 0; i < cls->body.protectedMembers.size(); i++) {
        if (cls->body.protectedMembers.at(i).type == IR_CM_VARIABLE) {
            cls->body.protectedMembers.at(i).varMemSize =
                getVarSize(cls->body.protectedMembers.at(i).data.variable->type, classTable);
#ifdef HX_DEBUG
            log(L"setClassVarMemSize->计算变量成员%ls大小：%d", cls->body.protectedMembers.at(i).data.variable->name,
                cls->body.protectedMembers.at(i).varMemSize);
#endif
        }
    }
    return 0;
}

int setClassVarMemOffset(IR_Class* cls, std::vector<IR_Class*>& classTable) {
    if (!cls) return -1;
#ifdef HX_DEBUG
    log("类：%ls", cls->name);
#endif
    int offest = 0;
    for (int i = 0; i < cls->body.publicMembers.size(); i++) {
        if (cls->body.publicMembers.at(i).type == IR_CM_VARIABLE) {
            cls->body.publicMembers.at(i).varMemOffest = offest;
#ifdef HX_DEBUG
            log(L"setClassVarMemOffset->计算变量成员%ls偏移量：%d", cls->body.publicMembers.at(i).data.variable->name,
                cls->body.publicMembers.at(i).varMemOffest);
#endif
            offest += cls->body.publicMembers.at(i).varMemSize;
        }
    }
    for (int i = 0; i < cls->body.privateMembers.size(); i++) {
        if (cls->body.privateMembers.at(i).type == IR_CM_VARIABLE) {
            cls->body.privateMembers.at(i).varMemOffest = offest;
#ifdef HX_DEBUG
            log(L"setClassVarMemOffset->计算变量成员%ls偏移量：%d", cls->body.privateMembers.at(i).data.variable->name,
                cls->body.privateMembers.at(i).varMemOffest);
#endif
            offest += cls->body.privateMembers.at(i).varMemSize;
        }
    }
    for (int i = 0; i < cls->body.protectedMembers.size(); i++) {
        if (cls->body.protectedMembers.at(i).type == IR_CM_VARIABLE) {
            cls->body.protectedMembers.at(i).varMemOffest = offest;
#ifdef HX_DEBUG
            log(L"setClassVarMemOffset->计算变量成员%ls偏移量：%d", cls->body.protectedMembers.at(i).data.variable->name,
                cls->body.protectedMembers.at(i).varMemOffest);
#endif
            offest += cls->body.protectedMembers.at(i).varMemSize;
        }
    }
    return 0;
}

static void generateInstructionsFromAST(std::vector<Instruction>& instructions, int* inst_index, int* inst_size, ASTNode* node,
                                        std::vector<IR_Class*>& classTable, ConstantPool* constantPool,
                                        std::vector<SymbolTable>& symbols, int procIndex, int* err);

static int getVarSize(IR_DataType type, std::vector<IR_Class*>& class_table);

PackedClassFunMem* findFunInClass(IR_Function& fun, IR_Class* cls, std::vector<IR_Class*>& classTable) {
#ifdef HX_DEBUG
    log(L"查找类的成员函数%ls", fun.name ? fun.name : L"(null)");
#endif
    if (cls == nullptr) {
        return nullptr;
    }
#ifdef HX_DEBUG
    log(L"- 类：%ls", cls->name);
    log(L"- fatherIndex: %d", cls->fatherIndex);
#endif
    PackedClassFunMem* packedClassFunMem = new PackedClassFunMem;
    for (int i = 0; i < cls->body.publicMembers.size(); i++) {
        if (cls->body.publicMembers.at(i).type == IR_CM_FUNCTION) {
            IR_Function* funInClass = cls->body.publicMembers.at(i).data.function;
#ifdef HX_DEBUG
            fwprintf(logStream, L"  member %d: type=%d, funPtr=%p\n", i, cls->body.publicMembers.at(i).type, (void*)funInClass);
            if (funInClass) {
                fwprintf(logStream, L"    fun->name=%ls\n", funInClass->name ? funInClass->name : L"(null)");
            }
#endif
            if (funInClass == nullptr) continue;
            if (funInClass->name != nullptr && fun.name != nullptr && wcscmp(funInClass->name, fun.name) == 0) {
                if (funInClass->paramCount == fun.paramCount) {
                    bool isMatch = true;

                    for (int j = 0; j < fun.paramCount; j++) {
                        // 如果类型不同，直接宣告不匹配喵
                        if (fun.params[j].type.kind != funInClass->params[j].type.kind) {
                            isMatch = false;
                            break;
                        }
                        // 如果是自定义类型，还要额外检查类名
                        if (fun.params[j].type.kind == IR_DT_CUSTOM || fun.params[j].type.kind == IR_DT_CUSTOM_ARR) {
                            if (wcscmp(fun.params[j].type.customTypeName, funInClass->params[j].type.customTypeName) != 0) {
                                isMatch = false;
                                break;
                            }
                        }
                    }
                    if (isMatch) {
                        packedClassFunMem->accessPermission = PackedClassFunMem::FUN_PUBLIC;
                        packedClassFunMem->cls = cls;
                        packedClassFunMem->irFun = funInClass;
                        return packedClassFunMem;
                    }
                }
            }
        }
    }
    for (int i = 0; i < cls->body.privateMembers.size(); i++) {
        if (cls->body.privateMembers.at(i).type == IR_CM_FUNCTION) {
            IR_Function* funInClass = cls->body.privateMembers.at(i).data.function;
#ifdef HX_DEBUG
            fwprintf(logStream, L"  member %d: type=%d, funPtr=%p\n", i, cls->body.publicMembers.at(i).type, (void*)funInClass);
            if (funInClass) {
                fwprintf(logStream, L"    fun->name=%ls\n", funInClass->name ? funInClass->name : L"(null)");
            }
#endif
            if (funInClass == nullptr) continue;
            if (funInClass->name != nullptr && fun.name != nullptr && wcscmp(funInClass->name, fun.name) == 0) {
                if (funInClass->paramCount == fun.paramCount) {
                    bool isMatch = true;

                    for (int j = 0; j < fun.paramCount; j++) {
                        // 如果类型不同，直接宣告不匹配喵
                        if (fun.params[j].type.kind != funInClass->params[j].type.kind) {
                            isMatch = false;
                            break;
                        }
                        // 如果是自定义类型，还要额外检查类名
                        if (fun.params[j].type.kind == IR_DT_CUSTOM || fun.params[j].type.kind == IR_DT_CUSTOM_ARR) {
                            if (wcscmp(fun.params[j].type.customTypeName, funInClass->params[j].type.customTypeName) != 0) {
                                isMatch = false;
                                break;
                            }
                        }
                    }
                    if (isMatch) {
                        packedClassFunMem->accessPermission = PackedClassFunMem::FUN_PUBLIC;
                        packedClassFunMem->cls = cls;
                        packedClassFunMem->irFun = funInClass;
                        return packedClassFunMem;
                    }
                }
            }
        }
    }
    for (int i = 0; i < cls->body.protectedMembers.size(); i++) {
        if (cls->body.protectedMembers.at(i).type == IR_CM_FUNCTION) {
            IR_Function* funInClass = cls->body.protectedMembers.at(i).data.function;
#ifdef HX_DEBUG
            fwprintf(logStream, L"  member %d: type=%d, funPtr=%p\n", i, cls->body.publicMembers.at(i).type, (void*)funInClass);
            if (funInClass) {
                fwprintf(logStream, L"    fun->name=%ls\n", funInClass->name ? funInClass->name : L"(null)");
            }
#endif
            if (funInClass == nullptr) continue;
            if (funInClass->name != nullptr && fun.name != nullptr && wcscmp(funInClass->name, fun.name) == 0) {
                if (funInClass->paramCount == fun.paramCount) {
                    bool isMatch = true;

                    for (int j = 0; j < fun.paramCount; j++) {
                        // 如果类型不同，直接宣告不匹配喵
                        if (fun.params[j].type.kind != funInClass->params[j].type.kind) {
                            isMatch = false;
                            break;
                        }
                        // 如果是自定义类型，还要额外检查类名
                        if (fun.params[j].type.kind == IR_DT_CUSTOM || fun.params[j].type.kind == IR_DT_CUSTOM_ARR) {
                            if (wcscmp(fun.params[j].type.customTypeName, funInClass->params[j].type.customTypeName) != 0) {
                                isMatch = false;
                                break;
                            }
                        }
                    }
                    if (isMatch) {
                        packedClassFunMem->accessPermission = PackedClassFunMem::FUN_PUBLIC;
                        packedClassFunMem->cls = cls;
                        packedClassFunMem->irFun = funInClass;
                        return packedClassFunMem;
                    }
                }
            }
        }
    }
    // 没找到就查找父类
    if (cls->fatherIndex == -1) {
#ifdef HX_DEBUG
        log(L"没找到");
#endif
        delete packedClassFunMem;
        return nullptr;
    }
#ifdef HX_DEBUG
    log(L"查找父类");
#endif
    return findFunInClass(fun, classTable.at(cls->fatherIndex), classTable);
}
PackedClassVarMem* findVarMemberInClass(const wchar_t* varMemName, IR_Class* cls, std::vector<IR_Class*>& classTable) {
#ifdef HX_DEBUG
    log(L"查找类的成员%ls", varMemName ? varMemName : L"(null)");
#endif
    if (!varMemName || !cls) {
        return NULL;
    }
#ifdef HX_DEBUG
    log(L"- 类：%ls", cls->name);
    log(L"- fatherIndex: %d", cls->fatherIndex);
#endif
    PackedClassVarMem* packedClassVarMem = new PackedClassVarMem;

    for (int i = 0; i < cls->body.publicMembers.size(); i++) {
        if (cls->body.publicMembers.at(i).type == IR_CM_VARIABLE) {
            if (cls->body.publicMembers.at(i).data.variable->name != nullptr &&
                wcscmp(cls->body.publicMembers.at(i).data.variable->name, varMemName) == 0) {
                packedClassVarMem->cls = cls;
                packedClassVarMem->accessPermission = PackedClassVarMem::VAR_PUBLIC;
                packedClassVarMem->irVar = cls->body.publicMembers.at(i).data.variable;
                return packedClassVarMem;
            }
        }
    }
    for (int i = 0; i < cls->body.privateMembers.size(); i++) {
        if (cls->body.privateMembers.at(i).type == IR_CM_VARIABLE) {
            if (cls->body.privateMembers.at(i).data.variable->name != nullptr &&
                wcscmp(cls->body.privateMembers.at(i).data.variable->name, varMemName) == 0) {
                packedClassVarMem->cls = cls;
                packedClassVarMem->accessPermission = PackedClassVarMem::VAR_PRIVATE;
                packedClassVarMem->irVar = cls->body.privateMembers.at(i).data.variable;
                return packedClassVarMem;
            }
        }
    }
    for (int i = 0; i < cls->body.protectedMembers.size(); i++) {
        if (cls->body.protectedMembers.at(i).type == IR_CM_VARIABLE) {
            if (cls->body.protectedMembers.at(i).data.variable->name != nullptr &&
                wcscmp(cls->body.protectedMembers.at(i).data.variable->name, varMemName) == 0) {
                packedClassVarMem->cls = cls;
                packedClassVarMem->accessPermission = PackedClassVarMem::VAR_PROTECTED;
                packedClassVarMem->irVar = cls->body.protectedMembers.at(i).data.variable;
                return packedClassVarMem;
            }
        }
    }
    if (cls->fatherIndex == -1) {
#ifdef HX_DEBUG
        log(L"没找到");
#endif
        delete packedClassVarMem;
        return nullptr;
    }
#ifdef HX_DEBUG
    log(L"查找父类");
#endif
    return findVarMemberInClass(varMemName, classTable.at(cls->fatherIndex), classTable);
}
