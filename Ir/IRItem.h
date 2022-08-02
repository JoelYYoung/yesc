#ifndef __IR_ITEM_H__
#define __IR_ITEM_H__

#include <string>
#include <unordered_map>
#include <vector>
#include "IR.h"
#include "../syntax/analyse.h"

using namespace std;
class IR;

class IRItem
{
public:
  enum IRItemType {
    INT,
    IVAR,
    FLOAT,
    FVAR,
    IR_ID,
    PLT,
    RETURN,
    SYMBOL,
    VAL,
    PC,
    STRING,
    SVAR
  };

  IRItemType type;
  IR *ir;
  Symbol *symbol;
  union {
    int iVal;
    float fVal;
  };
  string name;

  IRItem(IRItemType);
  IRItem(IRItemType, IR *);
  IRItem(IRItemType, Symbol *);
  IRItem(IRItemType, float);
  IRItem(IRItemType, int);
  IRItem(IRItemType, const string &);
  IRItem(IRItemType, const string &, int);
  ~IRItem();
};

extern unordered_map<IRItem::IRItemType, string> ItemStr;

#endif
