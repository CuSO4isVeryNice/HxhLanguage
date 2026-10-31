#pragma once
#include <stdio.h>

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Token.h"

static inline bool isOprAndSetOPRType(int& index, std::wstring& source, Token& token) noexcept;
static inline bool isOperator(int j, std::wstring& sourceCode);
static inline int parseValueString(std::wstring& sourceCode, int& i, int line, Token& newToken);

static inline int decodeEscapes(const std::wstring& source, std::wstring& decoded, int line);

int lex(std::wstring& sourceCode, std::vector<Token>& tokenArr) {
    std::wstring keywordList[] = {L"var",
                                  L"cst" /*constant*/,
                                  L"if",
                                  L"elif",
                                  L"els",
                                  L"cls" /*class*/,
                                  L"rpt" /*repeat*/,
                                  L"none",
                                  L"pub",
                                  L"pri",
                                  L"ptd" /*protected*/};
    int line = 1;

    for (int i = 0; i < sourceCode.size(); i++) {
        Token newToken;
        newToken.line = line;
        if (sourceCode[i] == L'\n') {
            newToken.type = Token::OPR_END;
            tokenArr.push_back(newToken);
            line++;
            continue;
        }
        if (iswspace(sourceCode.at(i))) {
            continue;
        }
        if (sourceCode.at(i) == L'#') {
            for (; i < sourceCode.size(); i++) {
                if (sourceCode.at(i) == L'\n') {
                    line++;
                    break;
                }
            }
            continue;
        }

        if (i < sourceCode.size() - 1 && sourceCode.at(i) == L'-' &&
            iswdigit(sourceCode.at(i + 1)) &&
            (tokenArr.size() > 0 && tokenArr.back().type != Token::VALUE)) {  // 负数
            if (int err = parseValueString(sourceCode, i, line, newToken)) {
                return err;
            }
            tokenArr.push_back(newToken);
        } else if (sourceCode.at(i) == L'\'' || sourceCode.at(i) == L'‘' ||
                   sourceCode.at(i) == L'’') {  // 字符
            const size_t charStart = static_cast<size_t>(i + 1);
            size_t j = charStart;
            int charStartLine = line;
            for (; j < sourceCode.size(); ++j) {
                if (sourceCode.at(j) == L'\\' && j + 1 < sourceCode.size()) {
                    ++j;
                    continue;
                }
                if (sourceCode.at(j) == L'\'' || sourceCode.at(j) == L'’' ||
                    sourceCode.at(j) == L'‘')
                    break;
                if (sourceCode.at(j) == L'\n') line++;
            }
            if (j >= sourceCode.size()) {
                std::wcout << ERR_LABEL << L"zako~zako~字符少了结尾呢~" << std::endl
                           << L"\t在 " << charStartLine << L" 行喵" << std::endl;
                return 255;
            }
            std::wstring decodedValue;
            if (int err = decodeEscapes(sourceCode.substr(charStart, j - charStart), decodedValue,
                                        charStartLine)) {
                return err;
            }
            newToken.strValue = decodedValue;
            i = static_cast<int>(j);
            newToken.type = Token::VALUE;
            newToken.value.cVal = decodedValue.size() != 0 ? decodedValue.at(0) : L'\0';
            newToken.valType = Token::CHAR;
        } else if (sourceCode.at(i) == L'\"' || sourceCode.at(i) == L'“' ||
                   sourceCode.at(i) == L'”') {  // 字符串
            const size_t charStart = static_cast<size_t>(i + 1);
            size_t j = charStart;
            int charStartLine = line;
            for (; j < sourceCode.size(); ++j) {
                if (sourceCode.at(j) == L'\\' && j + 1 < sourceCode.size()) {
                    ++j;
                    continue;
                }
                if (sourceCode.at(j) == L'\"' || sourceCode.at(j) == L'“' ||
                    sourceCode.at(j) == L'”')
                    break;
                if (sourceCode.at(j) == L'\n') line++;
            }
            if (j >= sourceCode.size()) {
                std::wcout << ERR_LABEL << L"zako~zako~字符串少了结尾呢~" << std::endl
                           << L"\t在 " << charStartLine << L" 行喵" << std::endl;
                return 255;
            }
            std::wstring decodedValue;
            if (int err = decodeEscapes(sourceCode.substr(charStart, j - charStart), decodedValue,
                                        charStartLine)) {
                return err;
            }
            newToken.strValue = decodedValue;
            i = static_cast<int>(j);
            newToken.type = Token::VALUE;
            newToken.valType = Token::STRING;
            tokenArr.push_back(newToken);
        } else if (isOprAndSetOPRType(i, sourceCode, newToken)) {
            i--;
            tokenArr.push_back(newToken);
            continue;
        } else if (iswdigit(sourceCode.at(i))) {  // 数字
            if (int err = parseValueString(sourceCode, i, line, newToken)) {
                return err;
            }
            tokenArr.push_back(newToken);
        } else {
            int j = i;
            for (; j < sourceCode.size(); j++) {
                if (iswspace(sourceCode.at(j)) || isOperator(j, sourceCode) ||
                    iswdigit(sourceCode.at(j)))
                    break;
            }
            std::wstring tokenVal = sourceCode.substr(i, j - i);
            i = j;
            i--;
            if (tokenVal.size() == 0) continue;
            bool isKeyword = false;
            for (const auto& keyword : keywordList) {
                if (tokenVal == keyword) {
                    isKeyword = true;
                    break;
                }
            }
            if (isKeyword) {
                newToken.type = Token::KEYWORD;
            } else {
                newToken.type = Token::IDENT;
            }
            newToken.strValue = tokenVal;
            tokenArr.push_back(newToken);
        }
    }
    if (isDebugMode) std::wcout << L"lex:结束" << std::endl;
    return 0;
}
inline int decodeEscapes(const std::wstring& source, std::wstring& decoded, int line) {
    std::wstring result;
    const auto reportSyntaxError = [line]() {
        std::wcout << ERR_LABEL << L"喵呜！转义字符写错了喵～" << std::endl
                   << L"\t在 " << line << L" 行喵" << std::endl;
        return 255;
    };

    try {
        for (size_t i = 0; i < source.size(); ++i) {
            if (source[i] != L'\\') {
                result.push_back(source[i]);
                continue;
            }

            if (++i >= source.size()) return reportSyntaxError();
            wchar_t escapedChar = source[i];
            std::uint32_t codePoint = 0;
            switch (escapedChar) {
                case L'\'':
                    codePoint = L'\'';
                    break;
                case L'"':
                    codePoint = L'"';
                    break;
                case L'?':
                    codePoint = L'?';
                    break;
                case L'\\':
                    codePoint = L'\\';
                    break;
                case L'a':
                    codePoint = L'\a';
                    break;
                case L'b':
                    codePoint = L'\b';
                    break;
                case L'f':
                    codePoint = L'\f';
                    break;
                case L'n':
                    codePoint = L'\n';
                    break;
                case L'r':
                    codePoint = L'\r';
                    break;
                case L't':
                    codePoint = L'\t';
                    break;
                case L'v':
                    codePoint = L'\v';
                    break;
                case L'x': {  // 十六进制unicode
                    size_t digitCount = 0;
                    for (++i; i < source.size(); ++i) {
                        wchar_t digit = source[i];
                        std::uint32_t digitValue;
                        if (digit >= L'0' && digit <= L'9') {
                            digitValue = static_cast<std::uint32_t>(digit - L'0');
                        } else if (digit >= L'a' && digit <= L'f') {
                            digitValue = static_cast<std::uint32_t>(digit - L'a' + 10);
                        } else if (digit >= L'A' && digit <= L'F') {
                            digitValue = static_cast<std::uint32_t>(digit - L'A' + 10);
                        } else {
                            --i;
                            break;
                        }
                        if (codePoint > (0x10FFFF - digitValue) / 16) {
                            return reportSyntaxError();
                        }
                        codePoint = codePoint * 16 + digitValue;
                        ++digitCount;
                    }
                    if (digitCount == 0 || codePoint > 0x10FFFF ||
                        (codePoint >= 0xD800 && codePoint <= 0xDFFF)) {
                        return reportSyntaxError();
                    }
                    break;
                }
                default:
                    if (escapedChar < L'0' || escapedChar > L'7') {
                        return reportSyntaxError();
                    }
                    codePoint = static_cast<std::uint32_t>(escapedChar - L'0');
                    for (int digitCount = 1; digitCount < 3 && i + 1 < source.size() &&
                                             source[i + 1] >= L'0' && source[i + 1] <= L'7';
                         ++digitCount) {
                        codePoint = codePoint * 8 + static_cast<std::uint32_t>(source[++i] - L'0');
                    }
                    break;
            }

            if (sizeof(wchar_t) == 2 && codePoint > 0xFFFF) {
                codePoint -= 0x10000;
                result.push_back(static_cast<wchar_t>(0xD800 + (codePoint >> 10)));
                result.push_back(static_cast<wchar_t>(0xDC00 + (codePoint & 0x3FF)));
            } else {
                result.push_back(static_cast<wchar_t>(codePoint));
            }
        }
    } catch (const std::bad_alloc&) {
        std::wcout << ERR_LABEL << L"喵？！解码转义字符时崩溃了喵……" << std::endl
                   << L"\t在 " << line << L" 行喵" << std::endl;
        return -1;
    } catch (const std::length_error&) {
        std::wcout << ERR_LABEL << L"喵？！解码转义字符时崩溃了喵……" << std::endl
                   << L"\t在 " << line << L" 行喵" << std::endl;
        return -1;
    }

    decoded.swap(result);
    return 0;
}
inline int parseValueString(std::wstring& sourceCode, int& i, int line, Token& newToken) {
    int j = i;
    for (; j < sourceCode.size(); ++j) {
        if (j > i && (iswspace(sourceCode.at(j)) || isOperator(j, sourceCode))) break;
    }
#define CATCH_AFTER_NUMBER_ERR                                                      \
    catch (const std::invalid_argument&) {                                          \
        std::wcout << ERR_LABEL << L"字面量写错了喵！笨～蛋～杂～鱼～" << std::endl \
                   << L"\t在 " << line << L" 行喵" << std::endl;                    \
        return 255;                                                                 \
    }                                                                               \
    catch (const std::out_of_range&) {                                              \
        std::wcout << ERR_LABEL << L"数值太...太大了，要....溢出了" << std::endl    \
                   << L"\t在 " << line << L" 行喵" << std::endl;                    \
        return -1;                                                                  \
    }
    const std::wstring strValue = sourceCode.substr(i, j - i);
    if (strValue.find(L'.') == strValue.npos && strValue.find(L'L') == strValue.npos) {  // 整型
        try {
            newToken.value.iVal = std::stoi(strValue);
        }
        CATCH_AFTER_NUMBER_ERR;
        newToken.type = Token::VALUE;
        newToken.valType = Token::INT;
        newToken.strValue = strValue;
    } else if (strValue.find(L'.') == strValue.npos) {  // 长整型
        try {
            newToken.value.lVal = std::stol(strValue);
        }
        CATCH_AFTER_NUMBER_ERR;
        newToken.type = Token::VALUE;
        newToken.valType = Token::LONG;
        newToken.strValue = strValue;
    } else if (strValue.find(L'f') == strValue.npos &&
               strValue.find(L'L') == strValue.npos) {  // long double
        try {
            newToken.value.dVal = std::stod(strValue);
        }
        CATCH_AFTER_NUMBER_ERR;
        newToken.type = Token::VALUE;
        newToken.valType = Token::DOUBLE;
        newToken.strValue = strValue;
    } else if (strValue.find(L'f') == strValue.npos) {  // double
        try {
            newToken.value.ldVal = std::stold(strValue);
        }
        CATCH_AFTER_NUMBER_ERR;
        newToken.type = Token::VALUE;
        newToken.valType = Token::LONG_DOUBLE;
        newToken.strValue = strValue;
    } else {  // float
        try {
            newToken.value.fVal = std::stof(strValue);
        }
        CATCH_AFTER_NUMBER_ERR;
        newToken.type = Token::VALUE;
        newToken.valType = Token::FLOAT;
        newToken.strValue = strValue;
    }
    i = j - 1;
    return 0;
}
inline bool isOperator(int j, std::wstring& sourceCode) {
    if (j >= sourceCode.size()) return false;
    return (sourceCode.at(j) == L'+' || sourceCode.at(j) == L'-' || sourceCode.at(j) == L'*' ||
            sourceCode.at(j) == L'/' || sourceCode.at(j) == L'=' || sourceCode.at(j) == L'|' ||
            sourceCode.at(j) == L'!' || sourceCode.at(j) == L'&' || sourceCode.at(j) == L'÷' ||
            sourceCode.at(j) == L'×' || sourceCode.at(j) == L'>' || sourceCode.at(j) == L'<' ||
            sourceCode.at(j) == L'(' || sourceCode.at(j) == L')' || sourceCode.at(j) == L'{' ||
            sourceCode.at(j) == L'}' || sourceCode.at(j) == L';' || sourceCode.at(j) == L'；' ||
            sourceCode.at(j) == L'。' || sourceCode.at(j) == L'[' || sourceCode.at(j) == L'【' ||
            sourceCode.at(j) == L']' || sourceCode.at(j) == L'】' || sourceCode.at(j) == L'\'' ||
            sourceCode.at(j) == L'\"' || sourceCode.at(j) == L'‘' || sourceCode.at(j) == L'’' ||
            sourceCode.at(j) == L'“' || sourceCode.at(j) == L'”' || sourceCode.at(j) == L':' ||
            sourceCode.at(j) == L'：' || sourceCode.at(j) == L',' || sourceCode.at(j) == L'，');
}
inline bool isOprAndSetOPRType(int& index, std::wstring& source, Token& token) noexcept {
    if (index >= source.size()) return false;
    if (index <= source.size() - 2) {
        if (source.at(index) == L'=' && source.at(index + 1) == L'=') {
            token.type = Token::OPR_EQU;
            index += 2;
            return true;
        } else if (source.at(index) == L'<' && source.at(index + 1) == L'=') {
            token.type = Token::OPR_LESS_THAN_OR_EQU;
            index += 2;
            return true;
        } else if (source.at(index) == L'>' && source.at(index + 1) == L'=') {
            token.type = Token::OPR_GREATER_THAN_OR_EQU;
            index += 2;
            return true;
        } else if (source.at(index) == L'+' && source.at(index + 1) == L'+') {
            token.type = Token::OPR_INC;
            index += 2;
            return true;
        } else if (source.at(index) == L'-' && source.at(index + 1) == L'-') {
            token.type = Token::OPR_DEC;
            index += 2;
            return true;
        } else if (source.at(index) == L'|' && source.at(index + 1) == L'|') {
            token.type = Token::OPR_LOGIC_OR;
            index += 2;
            return true;
        } else if (source.at(index) == L'&' && source.at(index + 1) == L'&') {
            token.type = Token::OPR_LOGIC_AND;
            index += 2;
            return true;
        } else if (source.at(index) == L'!' && source.at(index + 1) == L'=') {
            token.type = Token::OPR_UNEQU;
            index += 2;
            return true;
        } else if (source.at(index) == L'-' && source.at(index + 1) == L'>') {
            token.type = Token::OPR_POINT_TO;
            index += 2;
            return true;
        }
    }
    switch (source.at(index)) {
        case L'+': {
            token.type = Token::OPR_ADD;
            index++;
            return true;
        }
        case L'-': {
            token.type = Token::OPR_MINUS;
            index++;
            return true;
        }
        case L'*':
        case L'×': {
            token.type = Token::OPR_MUL;
            index++;
            return true;
        }
        case L'/':
        case L'÷': {
            token.type = Token::OPR_DIV;
            index++;
            return true;
        }
        case L'=': {
            token.type = Token::OPR_SET;
            index++;
            return true;
        }
        case L'&': {
            token.type = Token::OPR_AND;
            index++;
            return true;
        }
        case L'|': {
            token.type = Token::OPR_OR;
            index++;
            return true;
        }
        case L'!':
        case L'！': {
            token.type = Token::OPR_NOT;
            index++;
            return true;
        }
        case L'(': {
            token.type = Token::OPR_LPARE;
            index++;
            return true;
        }
        case L')': {
            token.type = Token::OPR_RPARE;
            index++;
            return true;
        }
        case L'{': {
            token.type = Token::OPR_LBRACE;
            index++;
            return true;
        }
        case L'}': {
            token.type = Token::OPR_RBRACE;
            index++;
            return true;
        }
        case L'<': {
            token.type = Token::OPR_LESS_THAN;
            index++;
            return true;
        }
        case L'>': {
            token.type = Token::OPR_GREATER_THAN;
            index++;
            return true;
        }
        case L';':
        case L'；':
        case L'。': {
            token.type = Token::OPR_END;
            index++;
            return true;
        }
        case L'[':
        case L'【': {
            token.type = Token::OPR_LSQUARE_BRACK;
            index++;
            return true;
        }
        case L']':
        case L'】': {
            token.type = Token::OPR_RSQUARE_BRACK;
            index++;
            return true;
        }
        case L':':
        case L'：': {
            token.type = Token::OPR_CLS_MEM_ACCESS;
            index++;
            return true;
        }
        case L',':
        case L'，': {
            token.type = Token::OPR_COMMA;
            index++;
            return true;
        }
    }
    return false;
}
// 调试用
inline const wchar_t* tokenTypeName(Token::TokenType tokenType) {
    switch (tokenType) {
        case Token::UNKNOWN:
            return L"UNKNOWN";
        case Token::IDENT:
            return L"IDENT";
        case Token::KEYWORD:
            return L"KEYWORD";
        case Token::VALUE:
            return L"VALUE";
        case Token::OPR_END:
            return L"OPR_END";
        case Token::OPR_ADD:
            return L"OPR_ADD";
        case Token::OPR_MINUS:
            return L"OPR_MINUS";
        case Token::OPR_MUL:
            return L"OPR_MUL";
        case Token::OPR_DIV:
            return L"OPR_DIV";
        case Token::OPR_AND:
            return L"OPR_AND";
        case Token::OPR_OR:
            return L"OPR_OR";
        case Token::OPR_NOT:
            return L"OPR_NOT";
        case Token::OPR_CLS_MEM_ACCESS:
            return L"OPR_CLS_MEM_ACCESS";
        case Token::OPR_DEC:
            return L"OPR_DEC";
        case Token::OPR_INC:
            return L"OPR_INC";
        case Token::OPR_GREATER_THAN:
            return L"OPR_GREATER_THAN";
        case Token::OPR_LESS_THAN:
            return L"OPR_LESS_THAN";
        case Token::OPR_GREATER_THAN_OR_EQU:
            return L"OPR_GREATER_THAN_OR_EQU";
        case Token::OPR_LESS_THAN_OR_EQU:
            return L"OPR_LESS_THAN_OR_EQU";
        case Token::OPR_EQU:
            return L"OPR_EQU";
        case Token::OPR_UNEQU:
            return L"OPR_UNEQU";
        case Token::OPR_SET:
            return L"OPR_SET";
        case Token::OPR_LBRACE:
            return L"OPR_LBRACE";
        case Token::OPR_RBRACE:
            return L"OPR_RBRACE";
        case Token::OPR_LPARE:
            return L"OPR_LPARE";
        case Token::OPR_RPARE:
            return L"OPR_RPARE";
        case Token::OPR_LSQUARE_BRACK:
            return L"OPR_LSQUARE_BRACK";
        case Token::OPR_RSQUARE_BRACK:
            return L"OPR_RSQUARE_BRACK";
        case Token::OPR_LOGIC_OR:
            return L"OPR_LOGIC_OR";
        case Token::OPR_LOGIC_AND:
            return L"OPR_LOGIC_AND";
        case Token::OPR_POINT_TO:
            return L"OPR_POINT_TO";
        case Token::OPR_COMMA:
            return L"OPR_COMMA";
    }
    return L"INVALID_TOKEN_TYPE";
}

inline const wchar_t* tokenValueTypeName(Token::ValueType valueType) {
    switch (valueType) {
        case Token::DOUBLE:
            return L"DOUBLE";
        case Token::INT:
            return L"INT";
        case Token::LONG:
            return L"LONG";
        case Token::LONG_DOUBLE:
            return L"LONG_DOUBLE";
        case Token::FLOAT:
            return L"FLOAT";
        case Token::CHAR:
            return L"CHAR";
        case Token::BOOLEAN:
            return L"BOOLEAN";
        case Token::STRING:
            return L"STRING";
    }
    return L"INVALID_VALUE_TYPE";
}

inline std::wstring escapeTokenText(const std::wstring& text) {
    std::wstring escaped;
    for (wchar_t character : text) {
        switch (character) {
            case L'\\':
                escaped += L"\\\\";
                break;
            case L'\n':
                escaped += L"\\n";
                break;
            case L'\r':
                escaped += L"\\r";
                break;
            case L'\t':
                escaped += L"\\t";
                break;
            case L'\0':
                escaped += L"\\0";
                break;
            default:
                escaped += character;
                break;
        }
    }
    return escaped;
}

inline std::wstring tokenValueString(const Token& token) {
    std::wostringstream value;
    switch (token.valType) {
        case Token::DOUBLE:
            value << token.value.dVal;
            break;
        case Token::INT:
            value << token.value.iVal;
            break;
        case Token::LONG:
            value << token.value.lVal;
            break;
        case Token::LONG_DOUBLE:
            value << token.value.ldVal;
            break;
        case Token::FLOAT:
            value << token.value.fVal;
            break;
        case Token::CHAR:
            value << L"U+" << std::uppercase << std::hex << std::setw(4) << std::setfill(L'0')
                  << static_cast<std::uint32_t>(token.value.cVal) << std::dec << L" ('"
                  << escapeTokenText(std::wstring(1, token.value.cVal)) << L"')";
            break;
        case Token::BOOLEAN:
            value << (token.value.byteVal ? L"true" : L"false");
            break;
        case Token::STRING:
            value << L"\"" << escapeTokenText(token.strValue) << L"\"";
            break;
    }
    return value.str();
}

inline const wchar_t* tokenColor(Token::TokenType tokenType) {
    switch (tokenType) {
        case Token::IDENT:
            return L"\033[36m";
        case Token::KEYWORD:
            return L"\033[35m";
        case Token::VALUE:
            return L"\033[32m";
        case Token::UNKNOWN:
            return L"\033[31m";
        default:
            return L"\033[33m";
    }
}

inline void printTokens(const std::vector<Token>& tokenArray, std::wostream& output = std::wcout) {
    constexpr const wchar_t* resetColor = L"\033[0m";
    output << L"\n\033[1;34m=== Token 列表 (" << tokenArray.size() << L") ===" << resetColor
           << L"\n"
           << std::left << std::setw(6) << L"#" << std::setw(8) << L"行号" << std::setw(28)
           << L"类型" << std::setw(20) << L"值类型" << std::setw(28) << L"词素" << L"解析值\n"
           << std::wstring(100, L'-') << L'\n';

    for (size_t i = 0; i < tokenArray.size(); ++i) {
        const Token& token = tokenArray[i];
        const bool hasValue = token.type == Token::VALUE;
        const std::wstring lexeme = escapeTokenText(token.strValue);
        const std::wstring parsedValue = hasValue ? tokenValueString(token) : L"-";

        output << std::left << std::setw(6) << i << std::setw(8) << token.line
               << tokenColor(token.type) << std::setw(28) << tokenTypeName(token.type) << resetColor
               << std::setw(20) << (hasValue ? tokenValueTypeName(token.valType) : L"-")
               << std::setw(28) << (lexeme.empty() ? L"-" : lexeme) << parsedValue << L'\n';
    }
    output << std::wstring(100, L'-') << L"\n\n";
}
