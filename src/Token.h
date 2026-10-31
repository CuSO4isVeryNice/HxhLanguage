#pragma once
#include <stdio.h>

#include <string>

class Token {
   public:
    int line;
    std::wstring strValue;
    union {
        int iVal;
        long lVal;
        double dVal;
        long double ldVal;
        float fVal;
        wchar_t cVal;
        char byteVal;
    } value;
    enum ValueType { DOUBLE, INT, LONG, LONG_DOUBLE, FLOAT, CHAR, BOOLEAN, STRING } valType;
    enum TokenType {
        UNKNOWN = 0,
        IDENT,
        KEYWORD,
        VALUE,

        OPR_END,
        OPR_ADD,                  //+
        OPR_MINUS,                //-
        OPR_MUL,                  //*
        OPR_DIV,                  // /
        OPR_AND,                  // &
        OPR_OR,                   // |
        OPR_NOT,                  // !
        OPR_CLS_MEM_ACCESS,       //:
        OPR_DEC,                  // --
        OPR_INC,                  //++
        OPR_GREATER_THAN,         // >
        OPR_LESS_THAN,            // <
        OPR_GREATER_THAN_OR_EQU,  // >=
        OPR_LESS_THAN_OR_EQU,     // <=
        OPR_EQU,                  //==
        OPR_UNEQU,                //!=
        OPR_SET,                  // =
        OPR_LBRACE,               // {
        OPR_RBRACE,               // }
        OPR_LPARE,                // (
        OPR_RPARE,                // )
        OPR_LSQUARE_BRACK,        // [
        OPR_RSQUARE_BRACK,        // ]
        OPR_LOGIC_OR,             // ||
        OPR_LOGIC_AND,            // &&
        OPR_POINT_TO,             // ->
        OPR_COMMA                 //,
    } type;
    Token(std::wstring str, TokenType type) {
        this->strValue = std::wstring(str);
        this->type = type;
    }
    Token() : type(UNKNOWN), line(-1) {}
};