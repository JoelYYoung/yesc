//
// Created by joelyang on 2022/7/24.
//

#ifndef HUST_ASSEMBLER_H
#define HUST_ASSEMBLER_H

#include "../Ir/IRBuild.h"
#include "Parameter.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

class Assembler {
private :
    IRBuild& irBuild;
    vector<string> asmVector;       // store asm code by line
    map<Symbol *, string> globalVarLabel;   //store label assigned to global var
    map<Symbol *, Parameter> funcParamMap;  // store info of func parameter
    map<string, Parameter> sysFuncParamMap;
    void generateGlobalVarAsm();    // global var asm
    void generateFunctionAsm();     // generate asm of functions
    void generateFuncParamInfo();   // generate func param info
    void singleFunctionAsm(pair<Symbol *, vector<IR *>> & func);       // generate asm of single function

public:
    Assembler(IRBuild& irBuild) : irBuild(irBuild){};
    void outputAsm(ostream &out);   // output to out
    void generateAsm();             // generate Asm
};


#endif //HUST_ASSEMBLER_H