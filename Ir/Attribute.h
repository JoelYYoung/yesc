#ifndef __IR_ATT__
#define __IR_ATT__

#include <utility>
#include <vector>
#include "IR.h"

using namespace std;
class Attribute
{
public:
    int continueLabel;
    int breakLabel;

    Attribute(int con,int bk);
    ~Attribute();
};

#endif