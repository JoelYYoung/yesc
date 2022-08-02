//
// Created by joelyang on 2022/7/29.
//

#ifndef HUST_PARAMETER_H
#define HUST_PARAMETER_H
#include <unordered_map>
#include "../Ir/IRBuild.h"
using namespace std;

class Parameter {
private:
    int paramStackSize;
    int paramRegSize;
    Symbol *func;
    unordered_map<Symbol *, pair<int, int>> paramLocationMap;  // 0 means reg, 1 means stack

public:
    Parameter& operator=(const Parameter& p);
    Parameter(): func(NULL), paramStackSize(0), paramRegSize(0){}
    Parameter(Symbol *func): func(func), paramStackSize(0), paramRegSize(0){  };
    void generateParamLocation();
    int getRegOrStack(Symbol *param);
    int getLocationId(Symbol *param);
    int getParamRegSize();
    int getParamStackSize();
};


#endif //HUST_PARAMETER_H
