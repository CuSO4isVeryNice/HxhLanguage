#ifndef HXHLANG_SRC_HXC_ERROR_H
#define HXHLANG_SRC_HXC_ERROR_H
#include <locale.h>
#include <stdio.h>
#include <wchar.h>
#define ERROR_BUF_SIZE 1024
wchar_t errorMessageBuffer[ERROR_BUF_SIZE];  // 错误信息缓冲区
typedef enum ErrorType {
    ERR_GLOBAL_UNKOWN,        // 未知的全局定义
    ERR_NO_END,               // 语句没结尾
    ERR_CH_NO_END,            // 字符没结尾
    ERR_STR_NO_END,           // 字符串没结尾
    ERR_VAL,                  // 字面量写错了
    ERR_HUAKUOHAO_NOT_CLOSE,  // 花括号末正确闭合
    ERR_DEF_VAR,              // 定义变量语法错误
    ERR_VAR_REPEATED,         // 变量重复定义
    ERR_DEF_CLASS,
    ERR_DEF_CLASS_ACCESS,              // 定义类时访问权限修饰符使用错误
    ERR_DEF_CLASS_DOUBLE_DEFINED_SYM,  // 定义类时重复声明符号
    ERR_FUN,
    ERR_FUN_ARG,
    ERR_FUN_REPEATED,  // 函数重复定义
    ERR_TYPE,
    ERR_MAIN,
    ERR_COUNLD_NOT_FIND_PARENT,
    // ERR_UNKOWN_TYPE,
    ERR_NO_MAIN,
    ERR_CANNOT_FIND_SYMBOL,
    ERR_EXP,
    ERR_OUT_OF_VALUE,    // 数值溢出
    ERR_CLASS_REPEATED,  // 类重复定义
    ERR_RET,             // 返回值错误 （语法错误）
    ERR_RET_VAL,         // 返回值错误
    ERR_UNKNOWN_TYPE,    // 未知类型
    ERR_REPEAT,
    ERR_IF,
    ERROR_UNCOMPLETED_CLASS,  // 类相互包含
    ERROR_INC_OR_DEC_OP_VAR,  // 非法自增/减操作数
    ERR_FOR,                  // for语句语法错误
    ERR_NO_VAR,
    ERR_CLASS_MEMBER_ACCESS,  // 类成员访问
    ERR_CLASS_MEMBER_ACCESS_NOT_SUPPORTED,
    ERR_SYNX_LIB_REF,  // 引用动态库语法错误
} ErrorType;
void initLocale(void) noexcept {
    // 设置Locale
    if (!setlocale(LC_ALL, "zh_CN.UTF-8")) {
        if (!setlocale(LC_ALL, "en_US.UTF-8")) {
            setlocale(LC_ALL, "C.UTF-8");
        }
    }
    // 设置宽字符流的定向
    fwide(stdout, 1);  // 1 = 宽字符定向
    return;
}

#ifdef LANG_zh_TW
void setError(ErrorType e, int errorLine, const wchar_t* errCode) noexcept {
    initLocale();
    switch (e) {
        case ERR_NO_END: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]\33[0m語句缺少分號結尾喵(位於第%"
                     L"d行)\n "
                     L"\33[36m[NOTE]\33[0m後面應該是分號喵->\33[4m%"
                     L"ls\33[0m\n "
                     L"\33[36m[NOTE]\33["
                     L"0m類別體中變數或常數符號只能宣告,賦值犯規了喵。",
                     errorLine, errCode ? errCode : L" ");
            break;
        }
        case ERR_CH_NO_END: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]\33[0m字元缺少結尾喵(位於第%d行)\n "
                     L"\33[36m[NOTE]\33[0m這個字元缺結尾->%ls\n",
                     errorLine, errCode ? errCode : L" ");
            break;
        }
        case ERR_STR_NO_END: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]\33[0m雖然這個字串太短了喵，但它沒結尾喵(位於第%d行)\n "
                     L"\33[36m[NOTE]\33[0m這個字串缺結尾->%ls\n",
                     errorLine, errCode ? errCode : L" ");
            break;
        }

        case ERR_VAL: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]\33[0m常值(Literal)錯誤了喵(位於第%d行)\n "
                     L"\33[36m[NOTE]\33[0m這個常值寫錯了->%ls\n",
                     errorLine, errCode ? errCode : L" ");
            break;
        }

        case ERR_HUAKUOHAO_NOT_CLOSE: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]大括號未正確封閉喵\33[0m！(位於第%"
                     L"d行)\n "
                     L"\33[36m[NOTE]\33["
                     L"0m這個大括號沒有對應的右大括號->%ls\n",
                     errorLine, errCode ? errCode : L" ");
            break;
        }

        case ERR_DEF_VAR: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]對定義變數語法錯誤了🥵\33[0m(位於第%"
                     L"d行)\n\33[36m["
                     L"NOTE]\33[0mDefineVariable::= "
                     L"<\"var\"><\":\"><id><\":\"><kw|id>;\n     "
                     L"定義變數::= "
                     L"<\"定義變數\"><\":\"><識別碼><\",\"><"
                     L"\"類型是\"><\":\"><"
                     L"識別碼|關鍵字>;\n",
                     errorLine);
            break;
        }

        case ERR_DEF_CLASS_ACCESS: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]定義類別時存取權限修飾詞使用錯誤了喵\33["
                     L"0m(位於第%d行)"
                     L"\n\33[36m[NOTE]\33[0m 宣告類別成員::= "
                     L"\"[public\"|\"private\"|\"protected\"|"
                     L"\"公有成員\"|"
                     L"\"私有成員\"|\"受保護成員\" <\":\"> ] "
                     L"定義函式|宣告變數\n",
                     errorLine);
            break;
        }

        case ERR_DEF_CLASS_DOUBLE_DEFINED_SYM: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]定義類別時重複宣告符號，犯規了喵\33[0m("
                     L"位於第%d行)\n\33[36m["
                     L"NOTE]\33[0m 此符號被重複宣告-> %ls\n",
                     errorLine, errCode ? errCode : L" ");
            break;
        }

        case ERR_DEF_CLASS: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]定義類別的語法有誤喵\33[0m(位於第%"
                     L"d行)\n\33[36m[NOTE]\33["
                     L"0m 定義類別::= <\"定義類別\"> "
                     L"<\":\"> <id> [<\",\"> "
                     L"<\"父類別是\"> <\":\"> <id>]  <\"->\">"
                     L"<\"{\"> ... <\"}\">\n"
                     L"DefineClass ::= class: [<id> ->] <id> {...}  #<id>->中的id為父類別名\n",
                     errorLine);
            break;
        }

        case ERR_FUN: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]笨蛋！定義函式的語法犯規了喵\33[0m(位於第%"
                     L"d行)\n\33[36m["
                     L"NOTE]\33[0m DefineFunction::= "
                     L"<\"fun\"><\":\"><id><\"(\"><args><\")\">[<\":\"><"
                     L"id|kw>]<\"->\"><"
                     L"\"{\">."
                     L"..<\"}\">\n定義函式::= "
                     L"<\"定義函式\"><\"：\"><識別碼><\"(\"><參數><\")"
                     L"\">[<\",\"><"
                     L"\"返回類型是\"><\"：\"><資料類型>]|[<\",\"><"
                     L"\"無返回類型\">] <\"->\"> <"
                     L"\"{\">...<\"}\">\n"
                     L"\33[36m[NOTE]\33[0m函式體內不可定義函式喵\n",
                     errorLine);
            break;
        }

        case ERR_FUN_ARG: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]對定義函式的參數的語法犯錯誤了\33["
                     L"0m(位於第%d行)\n\33["
                     L"36m[NOTE]\33[0m Argument::= "
                     L"<id><\":\"><id|kw>\n參數::= "
                     L"<識別碼><\":\"><識別碼|關鍵字>",
                     errorLine);
            break;
        }

        case ERR_TYPE: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]對類型拼寫犯錯誤了\33[0m(擺在第%d行)"
                     L"\n\33[36m["
                     L"NOTE]\33[0m ArrayType::= "
                     L"<id|kw>[<\"[\"><\"]\">...]\n參數::= "
                     L"<識別碼|關鍵字>[<\"[\"><\"]\">...]",
                     errorLine);
            break;
        }

        case ERR_MAIN: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]對主函式犯錯誤了\33[0m(位於第%d行)"
                     L"\n\33[36m[NOTE]"
                     L"\33[0m 主函式不能多載(Overload)！\n",
                     errorLine);
            break;
        }

        case ERR_COUNLD_NOT_FIND_PARENT: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]找不到父類別喵\33[0m(位於第%d行)"
                     L"\n\33[36m[NOTE]\33["
                     L"0m 這個父類別找不到->%ls\n",
                     errorLine, errCode);
            break;
        }

        case ERR_UNKOWN_TYPE: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]類型？不知道哦\33[0m(位於第%d行)"
                     L"\n\33[36m[NOTE]\33[0m "
                     L"似乎沒有這個類型喵->%ls\n",
                     errorLine, errCode);
            break;
        }

        case ERR_NO_MAIN: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE, L"\33[31m[ERR]沒有主函式喵\33[0m\n");
            break;
        }

        case ERR_CANNOT_FIND_SYMBOL: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE, L"\33[31m[ERR]找不到符號(%ls)喵\33[0m\n",
                     errCode ? errCode : L"(null)");
            break;
        }

        case ERR_EXP: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE, L"\33[31m[ERR]運算式錯誤(%ls)了喵\33[0m\n",
                     errCode ? errCode : L"(null)");
            break;
        }

        case ERR_OUT_OF_VALUE: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]啊♡~數值太......太大了......"
                     L"要溢出來了♡......(%ls)"
                     L"\33[0m\n",
                     errCode ? errCode : L"？？？？");
            break;
        }

        case ERR_GLOBAL_UNKOWN: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE, L"\33[31m[ERR]未知的全域定義！\33[0m(位於第%d行)\n", errorLine);
            break;
        }

        case ERR_FUN_REPEATED: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE, L"\33[31m[ERR]函式重複定義了喵\33[0m(位於第%d行)\n", errorLine);
            break;
        }

        case ERR_CLASS_REPEATED: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE, L"\33[31m[ERR]類別重複定義了喵\33[0m(位於第%d行)\n", errorLine);
            break;
        }

        case ERR_RET: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]笨蛋！返回語句語法錯誤了喵\33[0m(位於第%"
                     L"d行)\n\33[36m["
                     L"NOTE]\33[0m 返回::= ret:exp | 返回：exp\n",
                     errorLine);
            break;
        }
        case ERR_UNKNOWN_TYPE: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]這是什麼類型喵？  "
                     L"%ls\33[0m(位於第%d行)\n",
                     errCode ? errCode : L" ", errorLine);
            break;
        }
        case ERR_RET_VAL: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE, L"\33[31m[ERR]回傳值錯誤了喵\33[0m(位於第%d行)\n", errorLine);
            break;
        }
        case ERR_REPEAT: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]迴圈語句錯誤了喵\33[0m(位於第%d行)"
                     L"\n\33[36m[NOTE]"
                     L"\33[0m 迴圈 ::= repeat-> 語句|區塊 [until(exp)]\n",
                     errorLine);
            break;
        }

        case ERR_IF: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]對條件判斷語句犯錯誤了喵～\33["
                     L"0m(位於第%d行)\n\33["
                     L"36m[NOTE]\33[0m 條件判斷 ::= if: 運算式 -> "
                     L"語句|區塊\n",
                     errorLine);
            break;
        }

        case ERR_VAR_REPEATED: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]變數不能重複定義的喵～笨蛋！\33["
                     L"0m(擺在第%d行)\n",
                     errorLine);
            break;
        }
        case ERROR_UNCOMPLETED_CLASS: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]雜魚~你的類別互相包含了喵~("
                     L"如：類別A裡有類別B類型成員，而類別B裡又有類別A類型成員)"
                     L"如果不報錯，佔的記憶體就太......太大了。電腦會壞..."
                     L"...壞掉了...."
                     L".\33[0m(位於第%d行)\n",
                     errorLine);
            break;
        }
    }
    return;
}
#else
void setError(ErrorType e, int errorLine, const wchar_t* errCode) noexcept {
    initLocale();
    switch (e) {
        case ERR_NO_END: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]\33[0m哈？杂鱼连分号都能忘？语句根本没结束喵！(位于第%"
                     L"d行)\n "
                     L"\33[36m[NOTE]\33[0m后面乖乖加上分号啦喵->\33[4m%"
                     L"ls\33[0m\n "
                     L"\33[36m[NOTE]\33["
                     L"0m类里面的变量或者常量只能声明，随便赋值是犯规的喵！笨蛋杂鱼！",
                     errorLine, errCode ? errCode : L" ");
            break;
        }
        case ERR_CH_NO_END: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]\33[0m噗嗤，杂鱼写的字符连个单引号结尾都没有喵～(位于第%d行)\n "
                     L"\33[36m[NOTE]\33[0m喏，就是这个缺了结尾哦->%ls喵～\n",
                     errorLine, errCode ? errCode : L" ");
            break;
        }
        case ERR_STR_NO_END: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]\33[0m诶～双引号都配不对？这串乱码连结尾都没有呢，杂鱼真是丢人喵～(位于第%d行)\n "
                     L"\33[36m[NOTE]\33[0m缺结尾的在这->%ls喵～\n",
                     errorLine, errCode ? errCode : L" ");
            break;
        }

        case ERR_VAL: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]\33[0m呜哇，这个数值或文本（字面量）全写错了喵！杂鱼大叔的脑子还好吗？(位于第%d行)\n "
                     L"\33[36m[NOTE]\33[0m看看你写的什么蠢东西->%ls喵！\n",
                     errorLine, errCode ? errCode : L" ");
            break;
        }

        case ERR_HUAKUOHAO_NOT_CLOSE: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]杂鱼～花括号不闭合，里面的代码都要掉出来了喵\33[0m！(位于第%"
                     L"d行)\n "
                     L"\33[36m[NOTE]\33["
                     L"0m找不到右边另一半的孤儿括号在这哦->%ls喵～\n",
                     errorLine, errCode ? errCode : L" ");
            break;
        }

        case ERR_DEF_VAR: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]定义个变量都能写错，杂鱼果然只有这种程度喵？(￢_￢)\33[0m(位于第%"
                     L"d行)\n\33[36m["
                     L"NOTE]\33[0m 听好了，正确写法是喵：\n     "
                     L"var:变量名:类型名;\n     "
                     L"或者中文版喵::= "
                     L"<\"定义变量\"><\":\"><变量名><\",\"><"
                     L"\"类型是\"><\":\"><"
                     L"类型名>;\n",
                     errorLine);
            break;
        }

        case ERR_DEF_CLASS_ACCESS: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]杂鱼！类的权限控制不是让你乱搞的喵\33["
                     L"0m(位于第%d行)"
                     L"\n\33[36m[NOTE]\33[0m 声明类成员的正确姿势::= "
                     L"\"[public\"|\"private\"|\"protected\"|"
                     L"\"公有成员\"|"
                     L"\"私有成员\"|\"受保护成员\" <\":\"> ] "
                     L"定义函数|声明变量\n",
                     errorLine);
            break;
        }

        case ERR_DEF_CLASS_DOUBLE_DEFINED_SYM: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]哈？同一个名字你想在类里用几次？重复声明大犯规喵\33[0m("
                     L"位于第%d行)\n\33[36m["
                     L"NOTE]\33[0m 被我抓到的重复名字是这个哦-> %ls喵！\n",
                     errorLine, errCode ? errCode : L" ");
            break;
        }

        case ERR_DEF_CLASS: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]连建个类都不会，杂鱼还是早点放弃写代码吧喵～\33[0m(位于第%"
                     L"d行)\n\33[36m[NOTE]\33["
                     L"0m 给我看清楚格式喵::= <\"定义类\"> "
                     L"<\":\"> <类名> [<\",\"> "
                     L"<\"父类是\"> <\":\"> <父类名>]  <\"->\">"
                     L"<\"{\"> ... <\"}\">\n"
                     L"英文版喵 ::= class: [<子类名> ->] <父类名> {...}  #别再写错惹喵！\n",
                     errorLine);
            break;
        }

        case ERR_FUN: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]笨蛋杂鱼！定义函数的语法写得乱七八糟，根本看不懂喵\33[0m(位于第%"
                     L"d行)\n\33[36m["
                     L"NOTE]\33[0m 英文版喵::= "
                     L"[lib:] <\"fun\"><\":\"><函数名><\"(\"><参数们><\")\">[<\":\"><"
                     L"返回类型>]<\"->\"><"
                     L"\"{\">."
                     L"..<\"}\">\n中文版喵::= "
                     L"[原生库：] <\"定义函数\"><\"：\"><函数名><\"(\"><参数们><\")"
                     L"\">[<\",\"><"
                     L"\"返回类型是\"><\"：\"><返回类型>]|[<\",\"><"
                     L"\"无返回类型\">] <\"->\"> <"
                     L"\"{\">...<\"}\">"
                     L"\n\33[36m[NOTE]\33[0m函数里面不能再套函数哦，笨蛋喵！\n"
                     L"\n\33[36m[NOTE]\33[0m如果是原生库函数，就别画蛇添足写大括号里的内容了喵！\n",
                     errorLine);
            break;
        }

        case ERR_FUN_ARG: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]参数写得跟狗啃的一样，杂鱼大叔真恶心喵～\33["
                     L"0m(位于第%d行)\n\33["
                     L"36m[NOTE]\33[0m 乖乖写成喵::= "
                     L"<参数名><\":\"><参数类型>\n",
                     errorLine);
            break;
        }

        case ERR_TYPE: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]这是什么外星类型？杂鱼是想自己发明语言吗喵\33[0m(位于第%d行)"
                     L"\n\33[36m["
                     L"NOTE]\33[0m 数组类型要写清楚喵::= "
                     L"<类型名>[<\"[\"><\"]\">...]\n",
                     errorLine);
            break;
        }

        case ERR_MAIN: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]杂鱼！主函数(Main)是唯一的，居然敢重载，想死吗喵\33[0m(位于第%d行)"
                     L"\n\33[36m[NOTE]"
                     L"\33[0m 主函数绝对不可以有多个版本喵！\n",
                     errorLine);
            break;
        }

        case ERR_COUNLD_NOT_FIND_PARENT: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]找～不～到～哦！你要继承的父类根本不存在喵\33[0m(位于第%d行)"
                     L"\n\33[36m[NOTE]\33["
                     L"0m 失踪的父类叫这个->%ls喵～\n",
                     errorLine, errCode);
            break;
        }

        case ERR_NO_MAIN: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]诶？连程序入口(Main函数)都没有，杂鱼是想让电脑猜你要运行什么吗喵？笑死人了～\33[0m\n");
            break;
        }

        case ERR_CANNOT_FIND_SYMBOL: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]喵呜？根本没有(%ls)这个东西嘛！杂鱼又在无中生有了喵！\33[0m\n",
                     errCode ? errCode : L"(null)");
            break;
        }

        case ERR_EXP: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE, L"\33[31m[ERR]表达式完全不通！杂鱼的逻辑简直是灾难喵～(%ls)\33[0m\n",
                     errCode ? errCode : L"(null)");
            break;
        }

        case ERR_OUT_OF_VALUE: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]呜呀♡~杂鱼塞进来的数值太......太大了......"
                     L"马上就要溢出来了♡......(%ls)"
                     L"笨蛋！算术都不会吗喵！\33[0m\n",
                     errCode ? errCode : L"？？？？");
            break;
        }

        case ERR_GLOBAL_UNKOWN: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]这是什么鬼全局定义？杂鱼不要把垃圾随便扔在外面喵！\33[0m(位于第%d行)\n", errorLine);
            break;
        }

        case ERR_FUN_REPEATED: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]名字一样的函数写了两次？杂鱼的脑容量只有金鱼级别吗喵？\33[0m(位于第%d行)\n", errorLine);
            break;
        }

        case ERR_CLASS_REPEATED: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]哈？这个类已经存在了哦！杂鱼连自己写过什么都不记得了吗喵？\33[0m(位于第%d行)\n", errorLine);
            break;
        }

        case ERR_RET: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]笨蛋杂鱼！返回值语法错啦！连送个东西都不会喵？\33[0m(位于第%"
                     L"d行)\n\33[36m["
                     L"NOTE]\33[0m 应该是喵::= ret:返回值 | 返回：返回值\n",
                     errorLine);
            break;
        }
        case ERR_UNKNOWN_TYPE: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]喵喵喵？这是什么奇怪的类型？  "
                     L"%ls\33[0m杂鱼的妄想产物吗喵？(位于第%d行)\n",
                     errCode ? errCode : L" ", errorLine);
            break;
        }
        case ERR_NO_VAR: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]盯—— 这是什么符号喵？（好奇）  "
                     L"%ls\33[0m杂鱼写出了一堆乱码呢～完全不认识喵！(位于第%d行)\n",
                     errCode ? errCode : L" ", errorLine);
            break;
        }
        case ERR_RET_VAL: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]返回的东西根本不对嘛！杂鱼又在拿假货骗人喵？\33[0m(位于第%d行)\n", errorLine);
            break;
        }
        case ERR_REPEAT: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]转呀转呀转晕了喵！杂鱼的循环语句写得烂透了！\33[0m(位于第%d行)"
                     L"\n\33[36m[NOTE]"
                     L"\33[0m 乖乖用喵 ::= repeat-> 语句或者大括号 [until(条件)]\n",
                     errorLine);
            break;
        }

        case ERR_IF: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]如果杂鱼是笨蛋，那你的 if 语句也是错的喵！\33["
                     L"0m(位于第%d行)\n\33["
                     L"36m[NOTE]\33[0m 这样写才对喵 ::= if: 条件 -> "
                     L"语句或者大括号\n",
                     errorLine);
            break;
        }

        case ERR_VAR_REPEATED: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]杂鱼杂鱼～同一个变量名用两次是犯规的喵！笨蛋大叔！\33["
                     L"0m(位于第%d行)\n",
                     errorLine);
            break;
        }
        case ERROR_UNCOMPLETED_CLASS: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]呜哇~ 杂鱼写的类居然互相包含了喵~("
                     L"比如A里面有B，B里面又有A)"
                     L"再这样套娃下去，内存就要被杂鱼撑坏了喵......坏掉了啦♡"
                     L"\33[0m(位于第%d行)\n",
                     errorLine);
            break;
        }
        case ERROR_INC_OR_DEC_OP_VAR: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]哈？这种类型也想自增自减？杂鱼大叔在做梦喵！完全不支持哦笨蛋！\33["
                     L"0m(位于第%d行)\n",
                     errorLine);
            break;
        }
        case ERR_FOR: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]遍历都不会写，杂鱼真是没救了喵！\33["
                     L"0m(位于第%d行)\n\33["
                     L"36m[NOTE]\33[0m 给我记死在脑子里喵 ::= for: 每次拿出来的变量名:数组名 -> "
                     L"语句或者大括号\n 或者中文喵 ::= 遍历： 数组名，中间变量：每次拿出来的变量名",
                     errorLine);
            break;
        }
        case ERR_CLASS_MEMBER_ACCESS: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]杂鱼！乱摸别人的成员是不行的喵！语法犯规了笨蛋！\33["
                     L"0m(位于第%d行)\n"
                     L"\33[36m[NOTE]\33[0m 要用 名字(类):成员 才可以访问，或者这个类根本没有这个成员喵！\n",
                     errorLine);
            break;
        }
        case ERR_CLASS_MEMBER_ACCESS_NOT_SUPPORTED: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]诶～这个东西根本没有成员给你访问哦，杂鱼不要随便发情乱碰喵！\33[0m在第%d行\n", errorLine);
            break;
        }
        case ERR_SYNX_LIB_REF: {
            swprintf(errorMessageBuffer, ERROR_BUF_SIZE,
                     L"\33[31m[ERR]连引个动态库都能搞砸，杂鱼就是杂鱼喵～\33[0m在第%d行\n\33[36m[NOTE]\33[0m "
                     L"这么简单的都不会吗喵::= "
                     L"lib:库名|原生库：库名",
                     errorLine);
            break;
        }
    }
    return;
}
#endif
#endif