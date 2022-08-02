#ifndef __LEXER_H__
#define __LEXER_H__

#include <string>
#include <vector>
using namespace std;
enum tokenType {
    IDENT,
    INTCONST,
    FLOATCONST,
    STRCONST,
    CONST,
    INT,
    VOID,
    FLOAT,
    IF_TK,
    ELSE_TK,
    WHILE_TK,
    BREAK_TK,
    CONTINUE_TK,
    RETURN_TK,
    COMMA,
    SEMICOLON,
    LB,
    RB,
    LC,
    RC,
    LP,
    RP,
    ASSIGN,
    PLUS,
    MINUS,
    MUL,
    DIV,
    MOD,
    LT,
    GT,
    LE,
    GE,
    EQ,
    NE,
    NOT,
    AND,
    OR,
    END
};

class tokenInfo
{
public:
    tokenType symbol;
    string name;
    int value;
    float fvalue;
    explicit tokenInfo(tokenType sym);
    tokenType getSym();
    string getName();
    [[nodiscard]] int getValue() const;
    [[nodiscard]] float getFvalue() const;
    void setName(string na);
    void setValue(int i);
    void setFvalue(float i);
    tokenInfo(tokenType symbol,string name,int value,float fvalue);
    ~tokenInfo();
};

void parseSym(FILE *in);

bool isSpace();

bool isLetter();

bool isDigit(bool hexa_flg = false);

bool isTab();

bool isNewline();

long long strToInt(bool isHex, bool isOct);

double strToDouble(bool isHex, bool isOct);

vector<tokenInfo> lexicalAnalyze(const string &file);

#endif