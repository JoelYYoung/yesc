#include <string>
#include <unordered_map>

#include "IRItem.h"

using namespace std;

unordered_map<IRItem::IRItemType, string> ItemStr = {
    {IRItem::FLOAT, "FLOAT"},  {IRItem::INT, "INT"},  {IRItem::IR_ID, "IR_ID"},     
    {IRItem::IVAR, "IVAR"}, {IRItem::FVAR, "FVAR"},
    {IRItem::PLT, "PLT"},       {IRItem::RETURN, "RETURN"},
    {IRItem::SYMBOL, "SYMBOL"}, {IRItem::VAL, "VAL"}, {IRItem::PC, "PC"},
    {IRItem::STRING, "STRING"}, {IRItem::SVAR, "SVAR"}
};

IRItem::IRItem(IRItemType type) {
  this->type = type;
  this->symbol = nullptr;
}

IRItem::IRItem(IRItemType type, IR *ir) {
  this->type = type;
  this->ir = ir;
  this->symbol = nullptr;
}

IRItem::IRItem(IRItemType type, Symbol *symbol) {
  this->type = type;
  this->symbol = symbol;
}

IRItem::IRItem(IRItemType type, float fVal) {
  this->type = type;
  this->fVal = fVal;
  this->symbol = nullptr;
}

IRItem::IRItem(IRItemType type, int iVal) {
  this->type = type;
  this->iVal = iVal;
  this->symbol = nullptr;
}

IRItem::IRItem(IRItemType type, const string &sVal) {
  this->type = type;
  this->name = sVal;
  this->symbol = nullptr;
}

IRItem::IRItem(IRItemType type, const string &sVal, int iVal) {
  this->type = type;
  this->name = sVal;
  this->iVal = iVal;
  this->symbol = nullptr;
}

IRItem::~IRItem() {}
