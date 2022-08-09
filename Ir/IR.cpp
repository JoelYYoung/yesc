#include <string>
#include <unordered_map>

#include "IR.h"

using namespace std;

int irIdCount = 0;

unordered_map<IR::IRType, string> irTypeStr = {
    {IR::ADD, "ADD"},
    {IR::ARR, "ARR"},
    {IR::BEQ, "BEQ"},
    {IR::BNE, "BNE"},
    {IR::CALL, "CALL"},
    {IR::DIV, "DIV"},
    {IR::EQ, "EQ"},
    {IR::F2I, "F2I"},
    {IR::GE, "GE"},
    {IR::GOTO, "GOTO"},
    {IR::GT, "GT"},
    {IR::I2F, "I2F"},
    {IR::NOT, "NOT"},
    {IR::LE, "LE"},
    {IR::LDR, "LDR"},
    {IR::LT, "LT"},
    {IR::MEMSET_ZERO, "MEMSET_ZERO"},
    {IR::MOD, "MOD"},
    {IR::MOV, "MOV"},
    {IR::MUL, "MUL"},
    {IR::NAME, "NAME"},
    {IR::NE, "NE"},
    {IR::NEG, "NEG"},
    {IR::RETURN, "RETURN"},
    {IR::STR, "STR"},
    {IR::SUB, "SUB"}};

IR::IR(IRType type) {
  this->irId = irIdCount++;
  this->type = type;
}

IR::IR(IRType type, const vector<IRItem *> &items) {
  this->irId = irIdCount++;
  this->type = type;
  this->items = items;
}

IR::~IR() {
  for (IRItem *item : items)
    delete item;
}

void IR::deleteIr(int num){
  irIdCount -= num;
}

string IR::toString() {
  string s = "(" + to_string(irId) + ", " + irTypeStr[type];
  for (IRItem *item : items)
  {
    s += ", (" + ItemStr[item->type];
    if ((item->type != IRItem::RETURN) && (item->type != IRItem::PC))
      s += ", ";
    switch (item->type) {
    case IRItem::FLOAT:
      s += to_string(item->fVal);
      break;
    case IRItem::FVAR:
      s += to_string(item->iVal);
      break;
    case IRItem::INT:
      s += to_string(item->iVal);
      break;
    case IRItem::IVAR:
      s += to_string(item->iVal);
      break;
    case IRItem::IR_ID:
      s += to_string(item->iVal);
      break;
    case IRItem::PLT:
      s += item->name;
      break;
    case IRItem::SYMBOL:
      s += item->symbol->name;
      break;
    case IRItem::SVAR:
      s += to_string(item->iVal);
      break;
    case IRItem::STRING:
      s += item->name;
      break;
    default:
      break;
    }
    s += ")";
  }
  s += ")";
  return s;
}

vector<IRItem *> IR::getDefVar() {
    vector<IRItem *> defVarVec;
    if(this->type == IR::STR || this->type == IR::RETURN){
        return defVarVec;
    }
    if(this->items[0]->type == IRItem::FVAR || this->items[0]->type == IRItem::IVAR){
        defVarVec.push_back(this->items[0]);
        return defVarVec;
    }else{
        return defVarVec;
    }
}

vector<IRItem *> IR::getUseVar() {
    vector<IRItem *> useVarVec;
    if(this->type == IR::RETURN && this->items.size() == 0){
        return useVarVec;
    }
    auto tmpIt = this->items.begin() + 1;
    if(this->type == IR::STR || this->type == IR::RETURN) tmpIt = this->items.begin();
    while(tmpIt != this->items.end()){
        if((*tmpIt)->type == IRItem::FVAR || (*tmpIt)->type == IRItem::IVAR){
            useVarVec.push_back(*tmpIt);
        }
        tmpIt ++;
    }
    return useVarVec;
}
