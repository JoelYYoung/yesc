#include "lexer.h"

tokenInfo::tokenInfo(tokenType sym) {
    this->symbol = sym;
    this->value = 0;
}

tokenType tokenInfo::getSym() {
    return symbol;
}

string tokenInfo::getName() {
    return name;
}

void tokenInfo::setName(string na) {
    this->name = std::move(na);
}

int tokenInfo::getValue() const {
    return this->value;
}

void tokenInfo::setValue(int i) {
    this->value = i;
}

float tokenInfo::getFvalue() const {
    return this->fvalue;
}

void tokenInfo::setFvalue(float i) {
    this->fvalue = i;
}

tokenInfo::tokenInfo(tokenType symbol,string name,int value,float fvalue)
{
    this->symbol = symbol;
    this->name = name;
    this->value = value;
    this->fvalue = fvalue;
}

tokenInfo::~tokenInfo()
{

}