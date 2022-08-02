//
// Created by joelyang on 2022/7/29.
//

#include "Parameter.h"

Parameter& Parameter::operator=(const Parameter &p) {
    paramStackSize = p.paramStackSize;
    paramRegSize = p.paramRegSize;
    func = p.func;
    paramLocationMap = p.paramLocationMap;
    return *this;
}

void Parameter::generateParamLocation() {
    vector<Symbol *> funcParamList = func->params;
    if(funcParamList.size() == 0){
        return;
    }else if(funcParamList.size() <= 4){   // 4 parameter registers
        int regId = 0;
        for(Symbol *param : funcParamList){
            paramLocationMap[param] = pair<int, int> (0, regId);
            regId ++;
        }
        paramRegSize = regId;
    }else{
        paramRegSize = 4;
        for(int regId = 0; regId < 4; regId++){
            paramLocationMap[funcParamList[regId]] = pair<int, int> (0, regId);
        }
        auto stackParam = funcParamList.begin() + 4;
        while(stackParam != funcParamList.end()){
            paramLocationMap[*stackParam] = pair<int, int> (1, paramStackSize++);
            stackParam ++;
        }
    }
}

int Parameter::getRegOrStack(Symbol *param) {
    return paramLocationMap[param].first;
}

int Parameter::getLocationId(Symbol *param) {
    return paramLocationMap[param].second;
}

int Parameter::getParamRegSize() {
    return paramRegSize;
}

int Parameter::getParamStackSize() {
    return paramStackSize;
}


