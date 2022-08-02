#ifndef __UTIL_H__
#define __UTIL_H__
#include "parseNode.h"
struct Constant{
    bool isInt1;
    bool isInt2;
    int intVal1;
    float floatVal1;
    int intVal2;
    float floatVal2;
};
class Util
{
public:
    Constant deleteConst(vector<parseNode *>);
    bool judgeItem(vector<parseNode *>);
    parseNode *typeTrans(Symbol::DataType, parseNode *pn);
    Util();
    ~Util();
};

#endif