#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>
#include "Util.h"
using namespace ::std;

Util::~Util(){

}

Constant Util::deleteConst(vector<parseNode *> items) {
    bool isInt1 = false;
    bool isInt2 = false;
    int intVal1 = 0;
    float floatVal1 = 0;
    int intVal2 = 0;
    float floatVal2 = 0;
    (isInt1 = items[0]->parseType == parseNode::INT_LITERAL)
        ? (intVal1 = items[0]->iVal)
        : (floatVal1 = items[0]->fVal);
    (isInt2 = items[1]->parseType == parseNode::INT_LITERAL)
        ? (intVal2 = items[1]->iVal)
        : (floatVal2 = items[1]->fVal);
    //delete items[0];
    //delete items[1];
    items.erase(items.begin() + 1);
    Constant cons;
    cons.isInt1 = isInt1;
    cons.isInt2 = isInt2;
    cons.intVal1 = intVal1;
    cons.intVal2 = intVal2;
    cons.floatVal1 = floatVal1;
    cons.floatVal2 = floatVal2;
    return cons;
}

bool Util::judgeItem(vector<parseNode*> items)
{
    return (items[0]->parseType != parseNode::FLOAT_LITERAL && items[0]->parseType != parseNode::INT_LITERAL) || (items[1]->parseType != parseNode::FLOAT_LITERAL && items[1]->parseType != parseNode::INT_LITERAL);
}

parseNode* Util::typeTrans(Symbol::DataType type, parseNode* pn)
{
    if (type == Symbol::INT && pn->isFloat)
        pn = new parseNode(parseNode::UNARY_EXP, false, parseNode::F2I, {pn});
    if (type == Symbol::FLOAT && !pn->isFloat)
        pn = new parseNode(parseNode::UNARY_EXP, true, parseNode::I2F, {pn});
    return pn;
}