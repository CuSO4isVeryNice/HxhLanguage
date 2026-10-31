#pragma once

inline void Program::debugPrintFunList() {
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
                    static_cast<size_t>(type.classIndex) < this->classArr.size()) {
                    name = this->classArr.at(type.classIndex).name;
                } else {
                    name = L"<unknown>";
                }
                break;
        }
        if (type.isRef) name += L"&";
        return name;
    };

    const std::ios::fmtflags oldFlags = std::wcout.flags();
    const wchar_t oldFill = std::wcout.fill();
    std::wcout << L"\n\033[1;34m╔══════════════════════════════════════════════════════════╗\n"
               << L"║                   函数列表 (" << globalFunctionArr.size()
               << L")                    ║\n"
               << L"╚══════════════════════════════════════════════════════════╝" << resetColor
               << std::endl;

    if (globalFunctionArr.empty()) {
        std::wcout << L"  （当前没有函数）" << std::endl;
    }

    for (size_t functionIndex = 0; functionIndex < globalFunctionArr.size(); ++functionIndex) {
        const Function& function = globalFunctionArr.at(functionIndex);
        std::wcout << L"\n\033[1;36m[" << functionIndex + 1 << L"] " << function.name << resetColor
                   << std::endl
                   << L"  返回类型 : " << getTypeName(function.retType) << std::endl
                   << L"  参数数量 : " << function.argList.size() << std::endl
                   << L"  状态     : "
                   << (function.isDefined ? L"\033[1;32m已定义" : L"\033[1;33m仅声明") << resetColor
                   << std::endl
                   << L"  参数列表 :" << std::endl;

        if (function.argList.empty()) {
            std::wcout << L"    （无参数）" << std::endl;
        } else {
            for (size_t argumentIndex = 0; argumentIndex < function.argList.size();
                 ++argumentIndex) {
                const Variable& argument = function.argList.at(argumentIndex);
                std::wcout << L"    " << std::setw(2) << std::right << argumentIndex + 1 << L". "
                           << argument.name << L" : " << getTypeName(argument.type);
                if (argument.isOnlyRead) std::wcout << L"  \033[2m只读\033[0m";
                std::wcout << std::endl;
            }
        }

        std::wcout << L"  函数体   :" << std::endl;
        if (function.body.empty()) {
            std::wcout << L"    （无函数体）" << std::endl;
        } else {
            printTokens(function.body);
        }
    }

    std::wcout.flags(oldFlags);
    std::wcout.fill(oldFill);
    std::wcout << std::endl;
}

// 从类型名解析类型
static int parseTypeFromTypeName(Type& type, int& index, std::vector<Token>& tokenArr,
                                 std::vector<Class>& clsArr);
// 解析函数定义中的参数列表
static int parseFunDefArgList(std::vector<Variable>& argList, int& index,
                              std::vector<Token>& tokenArr, std::vector<Class>& clsArr);
int Program::tokenArrToFunArr() {
    for (int i = 0; i < this->tokenArr->size(); ++i) {
        if (this->tokenArr->at(i).type == Token::KEYWORD &&
            this->tokenArr->at(i).strValue == L"cls") {
            int openBrace = i + 1;
            while (openBrace < this->tokenArr->size() &&
                   this->tokenArr->at(openBrace).type != Token::OPR_LBRACE) {
                ++openBrace;
            }
            if (openBrace < this->tokenArr->size()) {
                int braceDepth = 0;
                for (i = openBrace; i < this->tokenArr->size(); ++i) {
                    if (this->tokenArr->at(i).type == Token::OPR_LBRACE) ++braceDepth;
                    if (this->tokenArr->at(i).type == Token::OPR_RBRACE && --braceDepth == 0) {
                        break;
                    }
                }
            }
            continue;
        }
        // 函数定义：
        //   <id> ( [argList]|"none" ) [-> retType] = <block|statement|exp> <end>    eg: f(x) = x+1;
        if (i + 2 < this->tokenArr->size() && this->tokenArr->at(i).type == Token::IDENT &&
            this->tokenArr->at(i + 1).type == Token::OPR_LPARE) {
            Function fun;
            int parseFunErr = this->parseFunDef(*(this->tokenArr), i, fun);
            if (parseFunErr) return parseFunErr;
        }
    }  // for
    return 0;
}
// 参数 ::= id [ : type]
static int parseFunDefArgList(std::vector<Variable>& argList, int& index,
                              std::vector<Token>& tokenArr, std::vector<Class>& clsArr) {
    for (; index < tokenArr.size(); index++) {
        Variable newVar;
        newVar.type.basicType = Type::UNKOWN;
        newVar.type.classIndex = -1;
        if (tokenArr.at(index).type == Token::IDENT) {
            newVar.name = tokenArr.at(index).strValue;
            if (isDebugMode)
                std::wcout << L"解析函数定义-> 解析到参数名：" << newVar.name << std::endl;

            index++;
            if (index >= tokenArr.size()) {
                std::wcout << ERR_LABEL << L"zako~zako~ 定义函数怎么连参数列表都不写全呀～"
                           << std::endl
                           << L"在第" << tokenArr.back().line << L"行喵" << std::endl;
                return 255;
            }
            //:
            if (tokenArr.at(index).type == Token::OPR_CLS_MEM_ACCESS) {
                index++;
                if (index >= tokenArr.size()) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 定义函数怎么连参数列表都不写全呀～"
                               << std::endl
                               << L"在第" << tokenArr.back().line << L"行喵" << std::endl;
                    return 255;
                }
                if (tokenArr.at(index).type != Token::IDENT) {
                    std::wcout << ERR_LABEL
                               << L"zako~zako~ 怎么连参数列表都写不对呀~ 真是个笨蛋呢～"
                               << std::endl
                               << L"在第" << tokenArr.back().line << L"行喵" << std::endl
                               << INFO_LABEL
                               << L"勉为其难告诉你吧：<参数> ::= id [ : id]    "
                                  L"#第一个id是参数名喵，第二个id就是类型喵"
                               << std::endl;
                    return 255;
                }
                if (int parseTypeErr = parseTypeFromTypeName(newVar.type, index, tokenArr, clsArr))
                    return parseTypeErr;
                if (index >= tokenArr.size()) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                    return 255;
                }
            }
            argList.push_back(newVar);
            // ,
            if (tokenArr.at(index).type == Token::OPR_COMMA) {
                continue;
            }
            // )
            if (tokenArr.at(index).type == Token::OPR_RPARE) break;
        } else if (tokenArr.at(index).type == Token::OPR_RPARE)
            break;
        else {
            std::wcout << tokenArr.at(index).strValue;
            std::wcout << ERR_LABEL << L"zako~zako~ 怎么连参数列表都写不对呀~ 真是个笨蛋呢～"
                       << std::endl
                       << L"在第" << tokenArr.back().line << L"行喵" << std::endl
                       << INFO_LABEL
                       << L"勉为其难告诉你吧：<参数> ::= id [ : id]    "
                          L"#第一个id是参数名喵，第二个id就是类型喵"
                       << std::endl
                       << L"<参数列表> ::= (  [{参数 [, 参数] }]  )" << std::endl;
            return 255;
        }
    }
    index++;
    return 0;
}
static int parseTypeFromTypeName(Type& type, int& index, std::vector<Token>& tokenArr,
                                 std::vector<Class>& clsArr) {
    if (index >= tokenArr.size()) {
        std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
        return 255;
    }
    if (tokenArr.at(index).type != Token::IDENT) {
        std::wcout << ERR_LABEL << L"zako~zako~怎么连类型名都写不对呀~ 真是个笨蛋呢～" << std::endl
                   << L"在第" << tokenArr.back().line << L"行喵" << std::endl
                   << INFO_LABEL
                   << L"勉为其难告诉你吧：<类型> ::= id | \"int\" | \"long\" | \"float\" | "
                      L"\"char\" | \"str\" "
                      L"| \"double\" | \"long double\" | \"bool\" "

                   << std::endl;
        return 255;
    }

    const std::wstring& typeStr = tokenArr.at(index).strValue;
    ++index;
    if (typeStr == L"int") {
        type.basicType = Type::INT;
        type.classIndex = -1;
    } else if (typeStr == L"long") {
        type.basicType = Type::LONG;
        type.classIndex = -1;
        // long double
        if (index < tokenArr.size() && tokenArr.at(index).type == Token::IDENT &&
            tokenArr.at(index).strValue == L"double") {
            type.basicType = Type::LONG_DOUBLE;
            ++index;
        }
    } else if (typeStr == L"float") {
        type.basicType = Type::FLOAT;
        type.classIndex = -1;
    } else if (typeStr == L"char") {
        type.basicType = Type::CHAR;
        type.classIndex = -1;
    } else if (typeStr == L"str") {
        type.basicType = Type::STRING;
        type.classIndex = -1;
    } else if (typeStr == L"double") {
        type.basicType = Type::DOUBLE;
        type.classIndex = -1;
    } else if (typeStr == L"bool") {
        type.basicType = Type::BOOLEAN;
        type.classIndex = -1;
    } else {  // 自定义类型
        type.basicType = Type::UNKOWN;
        type.classIndex = -1;
        for (size_t clsIndex = 0; clsIndex < clsArr.size(); ++clsIndex) {
            if (clsArr.at(clsIndex).name == typeStr) {
                type.classIndex = static_cast<int>(clsIndex);
            }
        }
    }
    // &
    if (index < tokenArr.size() && tokenArr.at(index).type == Token::OPR_AND) {
        type.isRef = true;
        ++index;
    }
    return 0;
}

int Program::parseFunDef(std::vector<Token>& sourceTokens, int& i, Function& fun,
                         bool registerGlobal) {
    if (i + 2 < sourceTokens.size() && sourceTokens.at(i).type == Token::IDENT &&
        sourceTokens.at(i + 1).type == Token::OPR_LPARE) {
        fun.name = std::wstring(sourceTokens.at(i).strValue);
        if (isDebugMode) std::wcout << L"分析函数定义 -> " << fun.name << std::endl;
        i += 2;

        std::vector<Variable> argList;
        if (int parseArgListErr =
                parseFunDefArgList(fun.argList, i, sourceTokens, this->classArr))
            return parseArgListErr;                          // 此时指向右括号的下一Token
        if (sourceTokens.at(i).type == Token::OPR_END) {  // 声明
            if (!registerGlobal) return 0;
            bool isAlreadyExistsInTable = false;
            for (const auto& tmp : this->globalFunctionArr) {
                if (tmp.name == fun.name) {
                    if (tmp.argList.size() == fun.argList.size()) {
                        bool sameSignature = true;
                        for (size_t j = 0; j < tmp.argList.size(); ++j) {
                            if (tmp.argList.at(j).type != fun.argList.at(j).type) {
                                sameSignature = false;
                                break;
                            }
                        }
                        if (sameSignature) {
                            isAlreadyExistsInTable = true;
                            break;
                        }
                    }
                }
            }
            if (!isAlreadyExistsInTable) this->globalFunctionArr.push_back(fun);
            return 0;
        }
        // 返回类型
        if (sourceTokens.at(i).type == Token::OPR_POINT_TO) {
            if (++i >= sourceTokens.size()) {
                std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                return 255;
            }
            if (sourceTokens.at(i).type == Token::IDENT) {
                if (int parseTypeErr =
                        parseTypeFromTypeName(fun.retType, i, sourceTokens, this->classArr))
                    return parseTypeErr;
                if (i >= sourceTokens.size()) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                    return 255;
                }
            } else {
                std::wcout << ERR_LABEL << L"zako~zako~ 怎么连定义函数都写不对呀~ 真是个笨蛋呢～"
                           << std::endl
                           << L"在第" << sourceTokens.at(i).line << L"行喵" << std::endl
                           << INFO_LABEL
                           << L"勉为其难告诉你吧：<定义函数> ::= id (参数列表) [-> 类型] = <块 "
                              L"| 表达式 | 语句> <结束符>"
                           << std::endl;
                return 255;
            }
        }
        if (sourceTokens.at(i).type == Token::OPR_SET) {  // 定义
            int defLine = sourceTokens.at(i).line;
            if (i++ >= sourceTokens.size()) {
                std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                return 255;
            }
            if (sourceTokens.at(i).type == Token::OPR_LBRACE) {
                int beginIndex = i;
                int lBrace = 0;
                int rBrace = 0;
                int j = i;
                for (; j < sourceTokens.size(); j++) {
                    Token& tmp = sourceTokens.at(j);
                    if (tmp.type == Token::OPR_LBRACE) lBrace++;
                    if (tmp.type == Token::OPR_RBRACE) rBrace++;
                    if (rBrace == lBrace) break;
                }

                if (rBrace != lBrace) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~" << std::endl;
                    return 255;
                }

                int endIndex = j;

                for (i = beginIndex; i <= endIndex; i++) {
                    fun.body.push_back(sourceTokens.at(i));
                }
                fun.isDefined = true;
            } else {  // 单条语句，其后可能紧跟代码块
                int beginIndex = i;
                for (; i < sourceTokens.size(); i++) {
                    if (sourceTokens.at(i).type == Token::OPR_LBRACE) {
                        int lBrace = 0;
                        int rBrace = 0;
                        for (; i < sourceTokens.size(); i++) {
                            Token& tmp = sourceTokens.at(i);
                            if (tmp.type == Token::OPR_LBRACE) lBrace++;
                            if (tmp.type == Token::OPR_RBRACE) rBrace++;
                            if (rBrace == lBrace) break;
                        }
                        if (rBrace != lBrace) {
                            std::wcout << ERR_LABEL << L"zako~zako~ 代码没写完就跑了呢~"
                                       << std::endl;
                            return 255;
                        }
                    }
                    if (sourceTokens.at(i).type == Token::OPR_END) break;
                }
                for (beginIndex; beginIndex < i; beginIndex++) {
                    fun.body.push_back(sourceTokens.at(beginIndex));
                }
                fun.isDefined = true;
            }
            if (registerGlobal) {
                bool isAlreadyDefinedInTable = false;
                bool hasMatchingDeclaration = false;
                for (auto& tmp : this->globalFunctionArr) {
                    if (tmp.name == fun.name && tmp.argList.size() == fun.argList.size()) {
                        bool sameSignature = true;
                        for (size_t j = 0; j < tmp.argList.size(); ++j) {
                            const Type& existingType = tmp.argList.at(j).type;
                            const Type& newType = fun.argList.at(j).type;
                            if (existingType.basicType != newType.basicType ||
                                existingType.classIndex != newType.classIndex ||
                                existingType.isRef != newType.isRef) {
                                sameSignature = false;
                                break;
                            }
                        }
                        if (sameSignature) {
                            hasMatchingDeclaration = true;
                            if (tmp.isDefined) {
                                isAlreadyDefinedInTable = true;
                            } else {
                                tmp = fun;
                            }
                            break;
                        }
                    }
                }
                if (isAlreadyDefinedInTable) {
                    std::wcout << ERR_LABEL << L"zako~zako~ 函数" << fun.name
                               << L"被重复定义了喵～" << std::endl
                               << L"在第" << defLine << L"行喵" << std::endl;
                    return 255;
                }
                if (!hasMatchingDeclaration) this->globalFunctionArr.push_back(fun);
            }
        }  // 定义 结束
        return 0;
    }
    return 0;
}