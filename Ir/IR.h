#ifndef __IR_H__
#define __IR_H__
#include "iostream"
#include "IRItem.h"
#include "Attribute.h"

using namespace std;

class IRItem;

class IR {
public:
  enum IRType {
    ADD,
    ARR,
    BEQ,
    BGE,
    BGT,
    BLE,
    BLT,
    BNE,  
    CALL,
    DIV,
    EQ, //==
    F2I,
    GE, //>=
    GOTO,
    GT, //>
    I2F,
    NOT,
    LE, //<=
    LDR,
    LT,  //<
    MEMSET_ZERO,
    MOD,
    MOV,
    MUL,
    NAME,
    NE, //!=
    NEG, // -
    RETURN,
    STR,
    SUB
  };

  int irId;
  IRType type;
  vector<IRItem *> items;

  string toString();

  void deleteIr(int num);
  vector<IRItem *> getDefVar();
  vector<IRItem *> getUseVar();
  IR(IRType);
  IR(IRType, const vector<IRItem *> &);
  ~IR();
};

#endif
