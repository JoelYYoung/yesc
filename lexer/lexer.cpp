#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "lexer.h"
#include <unistd.h>
using namespace std;
char c;
string token;
std::vector<tokenInfo> tokenInfoList;
std::unordered_map<string, tokenType>
        reverseTable{ // NOLINT
        {"int",      INT},
        {"float", FLOAT},
        {"const",    CONST},
        {"void",     VOID},
        {"if",       IF_TK},
        {"else",     ELSE_TK},
        {"while",    WHILE_TK},
        {"break",    BREAK_TK},
        {"continue", CONTINUE_TK},
        {"return",   RETURN_TK},
        {",",        COMMA},
        {";",        SEMICOLON},
        {"[",        LB},
        {"]",        RB},
        {"{",        LC},
        {"}",        RC},
        {"(",        LP},
        {")",        RP},
        {"=",        ASSIGN},
        {"+",        PLUS},
        {"-",        MINUS},
        {"!",        NOT},
        {"*",        MUL},
        {"/",        DIV},
        {"%",        MOD},
        {"<",        LT},
        {">",        GT},
        {"<=",       LE},
        {">=",       GE},
        {"==",       EQ},
        {"!=",       NE},
        {"&&",       AND},
        {"||",       OR}
};

void getOther(FILE *in);
void getAnnotation(FILE *in){
    c = fgetc(in);
    if (c == '*') {
        while (true) {
            c = fgetc(in);
            if (c == '*') {
                c = fgetc(in);
                if (c == '/') {
                    break;
                }
            }
        }
    } else if(c == '/'){
        while (true) {
            c = fgetc(in);
            if (c == '\n' || c == EOF) {
                break;
            }
        }
    } else {  //not an annotation maybe DIV
	    fseek(in, -1L, 1);
	    c = '/';
        getOther(in);
    }
} 

void getNumber(FILE *in){
    bool hex_flag = false;
    bool oct_flag = false; 
    if (c == '0') {
        c = fgetc(in);
        if (c == 'x' || c == 'X') {
            token.push_back('0');
            token.push_back(c);
            hex_flag = true;  // Hex
            c = fgetc(in);
        } else if(c>='0' and c <='7') {  // c in '0'~'7'
            token.push_back('0');
            oct_flag = true;  // Octal
        } else {  // c == '.' or c == ' '
            fseek(in, -1L, 1);
            c = '0';
        }
    }

    while (isDigit(hex_flag)) {  // digit-sequence
        token.push_back(c);
        c = fgetc(in);
    }

    //detect float
    if(c == '.'){
        token.push_back(c);
        c = fgetc(in);
        while (isDigit(hex_flag)) {  // digit-sequence
            token.push_back(c);
            c = fgetc(in);
        }

        if((!hex_flag&&(c == 'e' || c == 'E')) || (hex_flag&&(c=='p'||c=='P'))){
            token.push_back(c);
            c = fgetc(in);
            if(c == '+' || c == '-'){  // sign opt
                token.push_back(c);
                c = fgetc(in);
            }
            while (isDigit(false)) {  // digit-sequence
                token.push_back(c);
                c = fgetc(in);
            }
        }

        if(c == 'F' || c == 'L' || c == 'f' || c == 'l'){  // suffix
            token.push_back(c);
            c = fgetc(in);
        }

        double doubleLit = strToDouble(hex_flag, oct_flag);
        fseek(in, -1L, 1);
        tokenInfo tk = tokenInfo(FLOATCONST);
        tk.setName(token);
        if(false && tokenInfoList[tokenInfoList.size() - 1].getSym() == MINUS){
            tokenInfoList.pop_back();
            tk.setFvalue(-doubleLit);
        } else {
            tk.setFvalue(doubleLit);
        }
        tokenInfoList.push_back(tk);

    }else if(c == 'e' || c == 'E'){
        token.push_back(c);
        c = fgetc(in);
        if(c == '+' || c == '-'){  // sign opt
            token.push_back(c);
            c = fgetc(in);
        }
        while (isDigit(false)) {  // digit-sequence
            token.push_back(c);
            c = fgetc(in);
        }
        if(c == 'F' || c == 'L' || c == 'f' || c == 'l'){  // suffix
            token.push_back(c);
            c = fgetc(in);
        }

        double doubleLit = strToDouble(hex_flag, oct_flag);
        fseek(in, -1L, 1);
        tokenInfo tk = tokenInfo(FLOATCONST);
        tk.setName(token);
        if(false && tokenInfoList[tokenInfoList.size() - 1].getSym() == MINUS){
            tokenInfoList.pop_back();
            tk.setFvalue(-doubleLit);
        } else {
            tk.setFvalue(doubleLit);
        }
        tokenInfoList.push_back(tk);
    }else{  //not float
        long long integer = strToInt(hex_flag, oct_flag);
        fseek(in, -1L, 1);
        tokenInfo tk = tokenInfo(INTCONST);
        tk.setName(token);
        if (integer == 2147483648 && tokenInfoList[tokenInfoList.size() - 1].getSym() == MINUS) {
            tokenInfoList.pop_back();
            tk.setValue(-integer);
        } else {
            tk.setValue(integer);
        }
        tokenInfoList.push_back(tk);
    }
}

void getLetter(FILE *in){
    while ((isLetter() || isDigit())) {
        token.push_back(c);
        c = fgetc(in);
    }
    fseek(in, -1L, 1);
    string name(token);
    if (reverseTable.count(token) != 0) {
        tokenInfo tk = tokenInfo(reverseTable[token]);
        tk.setName(name);
        tokenInfoList.push_back(tk);
    } else {
        tokenInfo tk = tokenInfo(IDENT);
        tk.setName(token);
        tokenInfoList.push_back(tk);
    }
}

void getStr(FILE *in){
    while (true) {
        c = fgetc(in);
        token.push_back(c);
        if (c == '"') {
            break;
        }
    }
    tokenInfo tk = tokenInfo(STRCONST);
    tk.setName(token);
    tokenInfoList.push_back(tk);
}

void getOther(FILE *in){
    switch (c) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case ',':
        case ';': 
        case ':':
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        {
            tokenInfo tk = tokenInfo(reverseTable[string(1, c)]);
            tk.setName(string(1, c));
            tokenInfoList.push_back(tk);
            break;
        }
        case '|':
        case '&': {
            fgetc(in);
            tokenInfo tk = tokenInfo(reverseTable[string(2, c)]);
            tk.setName(string(2, c));
            tokenInfoList.push_back(tk);
            break;
        }
        case '<':
        case '>':
        case '!':
        case '=': {
            token.push_back(c);
            c = fgetc(in);
            if (c == '=') {
                token.push_back(c);
            } else {
                fseek(in, -1L, 1);
            }
            tokenInfo tk = tokenInfo(reverseTable.at(token));
            tk.setName(token);
            tokenInfoList.push_back(tk);
            break;
        }
        default:
            break;
    }
}

bool isSpace() {
    return c == ' ';
}

bool isLetter() {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
}

bool isDigit(bool hexa_flg) {
    if(hexa_flg == true){
        return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
    }else {
        return (c >= '0' && c <= '9');
    }
}

bool isTab() {
    return c == '\t';
}

bool isNewline() {
    return c == '\n';
}

long long strToInt(bool isHex, bool isOct) {
    long long integer;
    if (isHex) {
        sscanf(token.c_str(), "%llx", &integer);
    } else if (isOct) {
        sscanf(token.c_str(), "%llo", &integer);
    } else {
        sscanf(token.c_str(), "%lld", &integer);
    }
    return integer;
}

double strToDouble(bool isHex, bool isOct) {
    double doubleLit;
    if (isHex) {
        sscanf(token.c_str(), "%lfx", &doubleLit);
    } else if (isOct) {
        sscanf(token.c_str(), "%lfo", &doubleLit);
    } else {
        sscanf(token.c_str(), "%lfd", &doubleLit);
    }
    return doubleLit;
}


void parseSym(FILE *in){
    while(c!=EOF)
    {
        //cout << "next token: " << token << endl;
        token.clear();
        c = fgetc(in);
        //cout << "next char: " << c << endl;
        //sleep(1);
        while (!isNewline() && c < 32) {
            c = fgetc(in);
            if (c == EOF) {
                break;
            }
        }
        while (isSpace() || isNewline() || isTab()) {
            c = fgetc(in);
        }
        if(c=='/')
        {
            getAnnotation(in);
        }
        else if(isLetter())
        {
            getLetter(in);
        }
        else if(isDigit() || c == '.')
        {
            getNumber(in);
        }
        else if(c=='"')
        {
            getStr(in);
        }
        else
        {
            getOther(in);
        }
    }
}

vector<tokenInfo> lexicalAnalyze(const string &file){
    FILE *in = fopen(file.c_str(), "r");
    if (!in) {
        printf("error\n");
        return tokenInfoList;
    }
    parseSym(in);
    fclose(in);
    return tokenInfoList;
}
