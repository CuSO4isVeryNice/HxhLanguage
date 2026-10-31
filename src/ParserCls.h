#pragma once

static inline int parseClsBody(std::vector<Class::Member>& memList, std::vector<Token>& tokenArr,
                               Program& program);
static inline int setParentIndex(std::vector<Class>& clsList);

inline void Program::debugPrintClsList() {
    const wchar_t* resetColor = L"\033[0m";
    const auto getTypeName = [this](const Type& type) {
        std::wstring name;
        switch (type.basicType) {
            case Type::DOUBLE:
                name = L"double";
                break;
            case Type::INT:
                name = L"int";
                break;
            case Type::LONG:
                name = L"long";
                break;
            case Type::LONG_DOUBLE:
                name = L"long double";
                break;
            case Type::FLOAT:
                name = L"float";
                break;
            case Type::CHAR:
                name = L"char";
                break;
            case Type::BOOLEAN:
                name = L"bool";
                break;
            case Type::STRING:
                name = L"str";
                break;
            case Type::UNKOWN:
                if (type.classIndex >= 0 &&
                    static_cast<size_t>(type.classIndex) < classArr.size()) {
                    name = classArr.at(type.classIndex).name;
                } else {
                    name = L"<unknown>";
                }
                break;
        }
        if (type.isRef) name += L"&";
        return name;
    };
    const auto getAccessName = [](int access) {
        switch (access) {
            case Class::Member::PUB:
                return L"public";
            case Class::Member::PRI:
                return L"private";
            case Class::Member::PTD:
                return L"protected";
        }
        return L"unknown";
    };

    std::wcout << L"\n\033[1;34m╔══════════════════════════════════════════════════════════╗\n"
               << L"║                    类列表 (" << classArr.size()
               << L")                     ║\n"
               << L"╚══════════════════════════════════════════════════════════╝" << resetColor
               << std::endl;
    if (classArr.empty()) {
        std::wcout << L"  （当前没有类）" << std::endl;
        return;
    }

    for (size_t classIndex = 0; classIndex < classArr.size(); ++classIndex) {
        const Class& cls = classArr.at(classIndex);
        std::wcout << L"\n\033[1;36m[" << classIndex + 1 << L"] " << cls.name << resetColor
                   << std::endl
                   << L"  类名     : " << cls.name << std::endl
                   << L"  父类     : ";
        if (cls.parentIndex >= 0 && static_cast<size_t>(cls.parentIndex) < classArr.size()) {
            std::wcout << classArr.at(cls.parentIndex).name << L" (索引 " << cls.parentIndex
                       << L")";
        } else if (!cls.parentName.empty()) {
            std::wcout << cls.parentName << L" (未解析)";
        } else {
            std::wcout << L"无";
        }
        std::wcout << std::endl
                   << L"  成员数量 : " << cls.members.size() << std::endl
                   << L"  成员列表 :" << std::endl;

        if (cls.members.empty()) {
            std::wcout << L"    （无成员）" << std::endl;
            continue;
        }
        for (size_t memberIndex = 0; memberIndex < cls.members.size(); ++memberIndex) {
            const Class::Member& member = cls.members.at(memberIndex);
            std::wcout << L"    " << std::setw(2) << std::right << memberIndex + 1 << L". "
                       << L"\033[1;33m[" << getAccessName(member.memAcsPermisson) << L"]"
                       << resetColor << L" ";
            if (member.memType == Class::Member::VAR_MEM) {
                if (member.mem.memVar == nullptr) {
                    std::wcout << L"<空字段指针>" << std::endl;
                    continue;
                }
                const Variable& variable = *member.mem.memVar;
                std::wcout << L"字段 " << variable.name << L" : " << getTypeName(variable.type)
                           << L"  " << (variable.isOnlyRead ? L"\033[1;35m只读" : L"可写")
                           << resetColor << std::endl;
            } else if (member.memType == Class::Member::FUN_MEM) {
                if (member.mem.memFun == nullptr) {
                    std::wcout << L"<空方法指针>" << std::endl;
                    continue;
                }
                const Function& function = *member.mem.memFun;
                std::wcout << L"方法 " << function.name << L"(";
                for (size_t argumentIndex = 0; argumentIndex < function.argList.size();
                     ++argumentIndex) {
                    if (argumentIndex != 0) std::wcout << L", ";
                    const Variable& argument = function.argList.at(argumentIndex);
                    std::wcout << argument.name << L" : " << getTypeName(argument.type);
                }
                std::wcout << L") -> " << getTypeName(function.retType) << L"  "
                           << (function.isDefined ? L"\033[1;32m已定义" : L"\033[1;34m仅声明")
                           << resetColor << std::endl;
            } else {
                std::wcout << L"<未知成员类型>" << std::endl;
            }
        }
    }
    std::wcout << std::endl;
}

int Program::tokenArrToClsArr() {
    for (int index = 0; index < this->tokenArr->size(); index++) {
        //<定义类> ::= cls : <id> [-> id] = "{" [类体] "}"
        if (this->tokenArr->at(index).strValue == L"cls") {
            Class cls;

            if (index + 1 >= this->tokenArr->size()) {
                std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                return 255;
            }
            index++;
            // :
            if (this->tokenArr->at(index).type != Token::OPR_CLS_MEM_ACCESS) {
                std::wcout << ERR_LABEL << L"笨蛋～连定义类都能写错呢～真是个杂鱼呢～" << std::endl
                           << INFO_LABEL
                           << L"看好了， <定义类> ::= cls : <id> [-> id] = \"{\" [类体] \"}\""
                           << std::endl;
                return 255;
            }
            if (index + 1 >= this->tokenArr->size()) {
                std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                return 255;
            }
            index++;
            // 类名
            if (this->tokenArr->at(index).type != Token::IDENT) {
                std::wcout << ERR_LABEL << L"笨蛋～连定义类都能写错呢～真是个杂鱼呢～" << std::endl
                           << INFO_LABEL
                           << L"看好了， <定义类> ::= cls : <id> [-> id] = \"{\" [类体] \"}\""
                           << std::endl;
                return 255;
            }
            cls.name = this->tokenArr->at(index).strValue;
            if (isDebugMode) std::wcout << L"分析类定义 -> 解析到类名 : " << cls.name << std::endl;

            for (const auto& tmp : this->classArr) {
                if (tmp.name == cls.name) {
                    std::wcout << ERR_LABEL << L"笨蛋！一个类只能定义一次喵！管管 " << cls.name
                               << L"喵!" << std::endl;
                    return 255;
                }
            }

            if (index + 1 >= this->tokenArr->size()) {
                std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                return 255;
            }
            index++;

            // 继承父类
            if (this->tokenArr->at(index).type == Token::OPR_POINT_TO) {
                if (index + 1 >= this->tokenArr->size()) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                    return 255;
                }
                index++;
                // 父类类名
                if (this->tokenArr->at(index).type != Token::IDENT) {
                    std::wcout << ERR_LABEL << L"笨蛋～连定义类都能写错呢～真是个杂鱼呢～"
                               << std::endl
                               << INFO_LABEL
                               << L"看好了， <定义类> ::= cls : <id> [-> id] = \"{\" [类体] \"}\""
                               << std::endl;
                    return 255;
                }
                cls.parentName = this->tokenArr->at(index).strValue;
                if (isDebugMode)
                    std::wcout << L"分析类定义 -> 解析到父类名 : " << cls.parentName << std::endl;

                if (index + 1 >= this->tokenArr->size()) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                    return 255;
                }
                index++;
            }

            if (this->tokenArr->at(index).type != Token::OPR_SET) {
                std::wcout << ERR_LABEL << L"笨蛋～连定义类都能写错呢～真是个杂鱼呢～" << std::endl
                           << INFO_LABEL
                           << L"看好了， <定义类> ::= cls : <id> [-> id] = \"{\" [类体] \"}\""
                           << std::endl;
                return 255;
            }
            if (index + 1 >= this->tokenArr->size()) {
                std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                return 255;
            }
            index++;
            if (this->tokenArr->at(index).type != Token::OPR_LBRACE) {
                std::wcout << ERR_LABEL << L"笨蛋～连定义类都能写错呢～真是个杂鱼呢～" << std::endl
                           << INFO_LABEL
                           << L"看好了， <定义类> ::= cls : <id> [-> id] = \"{\" [类体] \"}\""
                           << std::endl;
                return 255;
            }
            if (index + 1 >= this->tokenArr->size()) {
                std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                return 255;
            }
            index++;
            // 存类体，获取所有类后再分析
            int bodyBegin = index;
            int lBrace = 1;
            int rBrace = 0;
            for (; index < this->tokenArr->size(); index++) {
                if (this->tokenArr->at(index).type == Token::OPR_LBRACE) lBrace++;
                if (this->tokenArr->at(index).type == Token::OPR_RBRACE) rBrace++;
                if (lBrace == rBrace) break;
            }
            if (lBrace != rBrace) {
                std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                return 255;
            }
            int bodyEnd = index - 1;
            for (; bodyBegin < bodyEnd + 1; bodyBegin++) {
                cls.body.push_back(this->tokenArr->at(bodyBegin));
            }
            this->classArr.push_back(cls);
            index++;
        }
    }  // for

    if (int setParentErr = setParentIndex(this->classArr)) return setParentErr;

    for (auto& cls : this->classArr) {
        if (int parseBodyErr = parseClsBody(cls.members, cls.body, *this)) return parseBodyErr;
        cls.body.clear();
        cls.body.shrink_to_fit();
    }
    return 0;
}
static inline int parseClsBody(std::vector<Class::Member>& memList, std::vector<Token>& tokenArr,
                               Program& program) {
    int index = 0;
    // 类体::= {[ [访问修饰符] <定义函数|定义变量|定义常量> ]}
    for (index; index < tokenArr.size(); index++) {
        Class::Member mem;
        // 访问权限
        if (tokenArr.at(index).type == Token::KEYWORD) {
            const std::wstring& tokenStrVal = tokenArr.at(index).strValue;
            if (tokenStrVal == L"pub") {
                mem.memAcsPermisson = Class::Member::PUB;
                if (index + 1 >= tokenArr.size()) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                    return 255;
                }
                index++;
            } else if (tokenStrVal == L"pri") {
                mem.memAcsPermisson = Class::Member::PRI;
                if (index + 1 >= tokenArr.size()) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                    return 255;
                }
                index++;
            } else if (tokenStrVal == L"ptd") {
                mem.memAcsPermisson = Class::Member::PTD;
                if (index + 1 >= tokenArr.size()) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                    return 255;
                }
                index++;
            }
        }
        // 定义
        if (tokenArr.at(index).type == Token::IDENT) {
            mem.memType = Class::Member::FUN_MEM;
            mem.mem.memFun = new Function;
            memList.push_back(mem);
            int parseFunErr =
                program.parseFunDef(tokenArr, index, *(memList.back().mem.memFun), false);
            if (parseFunErr) return parseFunErr;
        } else {
            // var | cst <id> : <id>      #类型必须声明
            if (tokenArr.at(index).strValue == L"var" || tokenArr.at(index).strValue == L"cst") {
                Variable newVar;
                if (tokenArr.at(index).strValue == L"cst") newVar.isOnlyRead = true;
                if (index + 1 >= tokenArr.size()) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                    return 255;
                }
                index++;
                if (tokenArr.at(index).type != Token::IDENT) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 连定义变量都能写错，真是个笨蛋呢~"
                               << std::endl
                               << INFO_LABEL << L"<定义变量> ::= var <id> : <id> <end>" << std::endl
                               << L"<定义常量> ::= cst <id> : <id> <end>" << std::endl;
                    return 255;
                }
                newVar.name = tokenArr.at(index).strValue;
                if (index + 2 >= tokenArr.size()) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                    return 255;
                }
                index++;
                //:
                if (tokenArr.at(index).type != Token::OPR_CLS_MEM_ACCESS) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 连定义变量都能写错，真是个笨蛋呢~"
                               << std::endl
                               << INFO_LABEL << L"<定义变量> ::= var <id> : <id> <end>" << std::endl
                               << L"<定义常量> ::= cst <id> : <id> <end>" << std::endl;
                    return 255;
                }
                index++;
                if (tokenArr.at(index).type != Token::IDENT) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 连定义变量都能写错，真是个笨蛋呢~"
                               << std::endl
                               << INFO_LABEL << L"<定义变量> ::= var <id> : <id> <end>" << std::endl
                               << L"<定义常量> ::= cst <id> : <id> <end>" << std::endl;
                    return 255;
                }
                int parseTypeErr =
                    parseTypeFromTypeName((newVar.type), index, tokenArr, program.classArr);
                if (parseTypeErr) return parseTypeErr;
                mem.memType = Class::Member::VAR_MEM;
                mem.mem.memVar = new Variable(newVar);
                memList.push_back(mem);
            }
        }
    }
    return 0;
}
static inline int setParentIndex(std::vector<Class>& clsList) {
    for (int i = 0; i < clsList.size(); i++) {
        if (clsList.at(i).parentName.size() != 0) {
            if (isDebugMode) std::wcout << L"查找：" << clsList.at(i).parentName << std::endl;
            bool isFound = false;
            for (int j = 0; j < clsList.size(); j++) {
                if (clsList.at(j).name == clsList.at(i).parentName) {
                    clsList.at(i).parentIndex = j;
                    isFound = true;
                    break;
                }
            }
            if (!isFound) {
                std::wcout << L"zako~zako~ 杂鱼定义的 " << clsList.at(i).name
                           << L"的父类看上去是一个慈祥的老奶奶呢" << std::endl;
                return 255;
            }
        }
    }
    return 0;
}