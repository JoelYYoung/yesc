//
// Created by joelyang on 2022/7/24.
//

#include "assembler.h"
#include <vector>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <climits>
#include <algorithm>
#include "allocater.h"
using namespace std;

void Assembler::generateAsm() {
    this->generateFuncParamInfo();
    this->generateFunctionAsm();
    asmVector.push_back(string(""));
    this->generateGlobalVarAsm();
}

void Assembler::outputAsm(ostream &out) {
    for(string asmStr : this->asmVector){
        out << asmStr <<endl;
    }
}

void Assembler::generateGlobalVarAsm() {  // put lines of global var
    vector<Symbol *> globalVarList = this->irBuild.getGlobalVars();

    int globalVarId = 0;
    ostringstream buffer;

//    for(auto globalVar : globalVarList){  // put lines of declaration
//        buffer << ".LCPI" << globalVarId << ":";
//        string varLabel = buffer.str();
//        buffer.clear();
//        buffer.str("");
//        buffer << "        " << ".long" << "    " << globalVar->name;
//        string varDeclare = buffer.str();
//        buffer.clear();
//        buffer.str("");
//        this->asmVector.push_back(varLabel);
//        this->asmVector.push_back(varDeclare);
//    }

    // data section
    this->asmVector.push_back(string(".data"));

    for(auto globalVar : globalVarList){  //put lines of initialization
        buffer << ".global " << globalVar->name;
        this->asmVector.push_back(buffer.str());
        buffer.clear();
        buffer.str("");

        buffer << globalVar->name << ":";
        string varInitLine_1 = buffer.str();
        this->asmVector.push_back(varInitLine_1);
        buffer.clear();
        buffer.str("");

        if(globalVar->dataType == Symbol::FLOAT){
            if(globalVar->dimensions.size() != 0){
                int globalVarSize = 1;
                for(int dimSize : globalVar->dimensions){
                    globalVarSize *= dimSize;
                }
                if(globalVar->fMap.size() != 0){
                    // need to initialize
                    for(int i = 0; i < globalVarSize; i++){
                        unsigned int *f2i = (unsigned int *)&(globalVar->fMap[i]);  // type conversion
                        buffer << "    " << ".long" << "    " << *f2i;
                        string varInitLine_2 = buffer.str();
                        buffer.clear();
                        buffer.str("");
                        this->asmVector.push_back(varInitLine_2);
                    }
                }else{
                    // no need to initialize
                    unsigned int *f2i = (unsigned int *)&(globalVar->fVal);  // type conversion
                    buffer << "    " << ".zero" << "    " << globalVarSize * 4;
                    string varInitLine_2 = buffer.str();
                    buffer.clear();
                    buffer.str("");

                    this->asmVector.push_back(varInitLine_2);
                }
            }else{
                unsigned int *f2i = (unsigned int *)&(globalVar->fVal);  // type conversion
                buffer << "    " << ".long" << "    " << *f2i;
                string varInitLine_2 = buffer.str();
                buffer.clear();
                buffer.str("");
                this->asmVector.push_back(varInitLine_2);
            }

        }else{
            if(globalVar->dimensions.size() != 0){
                int globalVarSize = 1;
                for(int dimSize : globalVar->dimensions){
                    globalVarSize *= dimSize;
                }
                if(globalVar->iMap.size() != 0){
                    // need to initialize
                    for(int i = 0; i < globalVarSize; i++){
                        unsigned int *si2i = (unsigned int *)&(globalVar->iMap[i]);
                        buffer << "    " << ".long" << "    " << *si2i;
                        string varInitLine_2 = buffer.str();
                        buffer.clear();
                        buffer.str("");
                        this->asmVector.push_back(varInitLine_2);
                    }
                }else{
                    // no need to initialize
                    buffer << "    " << ".zero" << "    " << globalVarSize * 4;
                    string varInitLine_2 = buffer.str();
                    buffer.clear();
                    buffer.str("");
                    this->asmVector.push_back(varInitLine_2);
                }
            }else{
                unsigned int *si2i = (unsigned int *)&(globalVar->iVal);  //type conversion
                buffer << "    " << ".long" << "    " << *si2i;
                string varInitLine_2 = buffer.str();
                buffer.clear();
                buffer.str("");
                this->asmVector.push_back(varInitLine_2);
            }
        }
    }
}

void Assembler::generateFuncParamInfo() {
    vector<pair<Symbol *, vector<IR *>>> funcList = this->irBuild.getFuncs();
    for(pair<Symbol *, vector<IR *>> func : funcList){
        Parameter funcParam (func.first);
        funcParam.generateParamLocation();
        funcParamMap[func.first] = funcParam;
    }
}

void Assembler::singleFunctionAsm(pair<Symbol *, vector<IR *>> & func) {
    Symbol *funcSymbol = func.first;
    vector<IR *> &funcIR = func.second;
    asmVector.push_back("");
    asmVector.push_back(".global " + funcSymbol->name);
    asmVector.push_back(funcSymbol->name + ":");
    int firstAsmIndex = asmVector.size();

    ostringstream buffer;

    unordered_map<Symbol *, int> symbolStackOffsetMap;
    int localDataSize = 0;  // word size of total local data;

    // get local variables
    vector<Symbol *> funcLocalVarList = this->irBuild.getLocalVars()[funcSymbol];
    for(Symbol *funcVar : funcLocalVarList){
        int tmpOffset = 0;  // word offset from bottom of func stack(NOT top)
        if(funcVar->dimensions.size() == 0){  // is single variable
            symbolStackOffsetMap[funcVar] = ++tmpOffset;
            localDataSize ++;
        }else{                                // is array variable
            int variableSize = 1;  // word size of array
            for(int dimSize : funcVar->dimensions){
                variableSize *= dimSize;
            }
            tmpOffset += variableSize;
            symbolStackOffsetMap[funcVar] = tmpOffset;
            localDataSize += variableSize;
        }
    }

    //cout << "get global Var done" << endl;

    // alloc stack space
    if(localDataSize != 0){
        buffer << "SUB " << "sp, sp, #" << (localDataSize * 4);  // byte size
        string localDataStackPrepare(buffer.str());  // INSTRUCTION
        asmVector.push_back(localDataStackPrepare);
        buffer.clear();
        buffer.str("");
    }

    // register allocation
    unordered_map<int, pair<int, int>> tmpVarLiveInterval;

        // step 1: live interval cal
    int irId = 0;
    for(IR* funcIr : funcIR){
        vector<IRItem *> defVarList = funcIr->getDefVar();
        vector<IRItem *> useVarList = funcIr->getUseVar();

        if(defVarList.size() != 0){
            int defVarId = defVarList[0]->iVal;
            if(tmpVarLiveInterval.find(defVarId) == tmpVarLiveInterval.end()){
                tmpVarLiveInterval[defVarId] = pair<int, int> (irId, 0);
            }
        }

        for(IRItem *useVar : useVarList){
            int useVarId = useVar->iVal;
            if(tmpVarLiveInterval.find(useVarId) == tmpVarLiveInterval.end()){
                tmpVarLiveInterval[useVarId].second = irId;
                //cout << "use before def error" << endl;
            }else{
                tmpVarLiveInterval[useVarId].second = irId;
            }
        }

        irId++;
    }
    //cout << "live Interval Cal done" << endl;
        // step 2: register allocation
    irId = 0;
    Allocater allocater(tmpVarLiveInterval, localDataSize);
    unordered_map<int, vector<string>> irAsmVectorMap;  // from ir to asm vector
    Parameter funcParam = funcParamMap[funcSymbol];
    vector<tuple<int, int, int, Symbol *>> backFillList;  // list to be back filled, first 1 need "]", second irId, third offset
    vector<int> labelInsertList;
    int backFillReturn;
    bool useLR = false;
    //cout << "Start for" << endl;
    for(IR *funcIr : funcIR){
        //cout << "to irId " << irId << endl;
        irAsmVectorMap[irId] = vector<string>();
        switch(funcIr->type){
            case IR::ADD:{  // ADD op1, op2, op3 (op1, op2 is I/FVAR)

                // allocate for op1
                int op1Id = funcIr->items[0]->iVal;
                IRItem *op2 = funcIr->items[1];
                IRItem *op3 = funcIr->items[2];  // IVAR/FVAR/INT/FLOAT
                vector<int> tmpVarList({op1Id, });

                if(op2->type != IRItem::PC){
                    tmpVarList.push_back(op2->iVal);
                }

                if(op3->type == IRItem::IVAR || op3->type == IRItem::FVAR){
                    tmpVarList.push_back(op3->iVal);
                }


                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                if(op3->type == IRItem::IVAR){
                    if(op2->type == IRItem::PC){
                        buffer << "ADD r" << allocater.getVarRegId(op1Id) << ", pc, r"
                               << allocater.getVarRegId(op3->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else{
                        buffer << "ADD r" << allocater.getVarRegId(op1Id) << ", r"<< allocater.getVarRegId(op2->iVal)
                               <<", r" << allocater.getVarRegId(op3->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }

                }else if(op3->type == IRItem::FVAR){
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "MOV r1, r" << allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("BL __aeabi_fadd");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                }else if(op3->type == IRItem::INT){
                    if(op2->type == IRItem::PC){
                        buffer << "ADD r" << allocater.getVarRegId(op1Id) << ", pc"
                               << ", #" << op3->iVal;
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else{
                        buffer << "ADD r" << allocater.getVarRegId(op1Id) << ", r" << allocater.getVarRegId(op2->iVal)
                               << ", #" << op3->iVal;
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }
                }else if(op3->type == IRItem::FLOAT){
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(*((unsigned *)(&(op3->fVal))) < 65535){
                        buffer << "MOV r1, #" << *((unsigned *)(&(op3->fVal)));
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else{
                        buffer << "MOVW r1, , #:lower16:" << *((unsigned *)(&(op3->fVal)));
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                        buffer << "MOVT r1, , #:upper16:" << *((unsigned *)(&(op3->fVal)));
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }


                    irAsmVectorMap[irId].push_back("BL __aeabi_fadd");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                }
                break;
            }
            case IR::MOV:{
                int op1Id = funcIr->items[0]->iVal;
                IRItem *op2 = funcIr->items[1];  // IVAR/FVAR/INT/FLOAT
                vector<int> tmpVarList({op1Id, });
                if(op2->type == IRItem::IVAR || op2->type == IRItem::FVAR){
                    tmpVarList.push_back(op2->iVal);
                }
                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                if(op2->type == IRItem::IVAR || op2->type == IRItem::FVAR){
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << \
                        ", r" << allocater.getVarRegId(op2->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else{
                    if(op2->type == IRItem::INT){
                        if(*((unsigned *) (&(op2->iVal))) > 65535){
                            buffer << "MOVW r" << allocater.getVarRegId(op1Id)  << ", #:lower16:" << *((unsigned *) (&(op2->iVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                            buffer << "MOVT r" << allocater.getVarRegId(op1Id)  << ", #:upper16:" << *((unsigned *) (&(op2->iVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }else{
                            buffer << "MOV r" << allocater.getVarRegId(op1Id)  << ", #" << *((unsigned *) (&(op2->iVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }
                    }else if(op2->type == IRItem::FLOAT){
                        if(*((unsigned *) (&(op2->fVal))) > 65535) {
                            buffer << "MOVW r" << allocater.getVarRegId(op1Id) << ", #:lower16:"
                                   << *((unsigned *) (&(op2->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                            buffer << "MOVT r" << allocater.getVarRegId(op1Id) << ", #:upper16:"
                                   << *((unsigned *) (&(op2->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }else{
                            buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", #"
                                   << *((unsigned *) (&(op2->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }
                    }
                }
                break;
            }
            case IR::STR:{
                int op1Id = funcIr->items[0]->iVal;
                IRItem *op2 = funcIr->items[1];  // IVAR/FVAR/INT/FLOAT
                vector<int> tmpVarList({op1Id, });
                if(op2->type == IRItem::IVAR || op2->type == IRItem::FVAR){
                    tmpVarList.push_back(op2->iVal);
                }
                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                if(op2->type == IRItem::IVAR || op2->type == IRItem::FVAR) {
                    buffer << "STR r" << allocater.getVarRegId(op1Id) << \
                            ", [r" << allocater.getVarRegId(op2->iVal) << "]";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else{
                    if(op2->symbol->symbolType == Symbol::PARAM){
                        if(funcParam.getRegOrStack(op2->symbol) == 0){  // parameter on reg
                            buffer << "MOV r" << funcParam.getLocationId(op2->symbol)
                                   << ", r" << allocater.getVarRegId(op1Id);
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }else{  // parameter on stack
                            buffer << "STR r" << allocater.getVarRegId(op1Id) << \
                            ", [sp, #";
                            irAsmVectorMap[irId].push_back(buffer.str());
                            backFillList.push_back(tuple<int, int, int, Symbol *>(1, irId, irAsmVectorMap[irId].size()-1, op2->symbol));
                            buffer.clear();
                            buffer.str("");
                        }

                    }else if(op2->symbol->symbolType == Symbol::LOCAL_VAR){
                        buffer << "STR r" << allocater.getVarRegId(op1Id) << \
                            ", [ip, #-" << symbolStackOffsetMap[op2->symbol]*4 << "]";
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }
                }

                break;
            }
            case IR::LDR:{
                int op1Id = funcIr->items[0]->iVal;
                IRItem *op2 = funcIr->items[1];  // IVAR/FVAR/INT/FLOAT
                vector<int> tmpVarList({op1Id, });
                if(op2->type == IRItem::IVAR || op2->type == IRItem::FVAR){
                    tmpVarList.push_back(op2->iVal);
                }
                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                if(op2->type == IRItem::IVAR || op2->type == IRItem::FVAR) {
                    buffer << "LDR r" << allocater.getVarRegId(op1Id) << \
                            ", [r" << allocater.getVarRegId(op2->iVal) << "]";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else{
                    if(op2->symbol->symbolType == Symbol::PARAM){
                        if(funcParam.getRegOrStack(op2->symbol) == 0){  // parameter on reg
                            buffer << "MOV r" << allocater.getVarRegId(op1Id) << \
                            ", r" << funcParam.getLocationId(op2->symbol);
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }else{  // parameter on stack
                            buffer << "LDR r" << allocater.getVarRegId(op1Id) << \
                            ", [ip, #";
                            irAsmVectorMap[irId].push_back(buffer.str());
                            backFillList.push_back(tuple<int, int, int, Symbol *>(1, irId, irAsmVectorMap[irId].size()-1, op2->symbol));
                            buffer.clear();
                            buffer.str("");
                        }

                    }else if(op2->symbol->symbolType == Symbol::LOCAL_VAR){
                        buffer << "LDR r" << allocater.getVarRegId(op1Id) << \
                            ", [ip, #-" << symbolStackOffsetMap[op2->symbol]*4 << "]";
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else if(op2->symbol->symbolType == Symbol::GLOBAL_VAR){
                        buffer << "LDR r" << allocater.getVarRegId(op1Id) << \
                            ", =" << op2->symbol->name;
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");

//                        //if()
//                        buffer << "LDR r" << allocater.getVarRegId(op1Id) << ", [r"
//                               << allocater.getVarRegId(op1Id) << "]";
//                        irAsmVectorMap[irId].push_back(buffer.str());
//                        buffer.clear();
//                        buffer.str("");
                    }
                }
                break;
            }
            case IR::NAME:{
                int op1Id = funcIr->items[0]->iVal;
                IRItem *op2 = funcIr->items[1];  // IVAR/FVAR/INT/FLOAT
                vector<int> tmpVarList({op1Id, });
                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                if(op2->symbol->symbolType == Symbol::LOCAL_VAR){
                    buffer << "SUB r" << allocater.getVarRegId(op1Id) << \
                            ", ip, #" << symbolStackOffsetMap[op2->symbol] * 4;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op2->symbol->symbolType == Symbol::PARAM){
                    buffer << "ADD r" << allocater.getVarRegId(op1Id) << \
                            ", ip, #";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    backFillList.push_back(tuple<int, int, int, Symbol *>(0, irId, irAsmVectorMap[irId].size()-1, op2->symbol));
                    buffer.clear();
                    buffer.str("");
                }
                break;
            }
            case IR::RETURN:{
                int op1Id = funcIr->items[0]->iVal;
                vector<int> tmpVarList({op1Id, });
                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                buffer << "MOV r0, r" << allocater.getVarRegId(op1Id);
                irAsmVectorMap[irId].push_back(buffer.str());
                backFillReturn = irAsmVectorMap[irId].size();
                buffer.clear();
                buffer.str("");
                irAsmVectorMap[irId].push_back(string("BX lr"));
                break;
            }
            case IR::CALL:{
                useLR = true;
                Parameter callFuncParam = funcParamMap[funcIr->items[0]->symbol];
                // preserve register r0-r3, lr, ip
                int paramRegSize = callFuncParam.getParamRegSize();
                int paramStackSize = callFuncParam.getParamStackSize();
                if(paramRegSize == 0){
                    irAsmVectorMap[irId].push_back("PUSH {ip}");
                }else if(paramRegSize == 1){
                    irAsmVectorMap[irId].push_back("PUSH {r0, ip}");
                }else{
                    buffer << "PUSH {r0-r" << paramRegSize - 1 << ", ip}";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }
                // parameter into stack
                if(paramRegSize >= 1 && paramStackSize == 0){
                    for(int i = 0; i < paramRegSize; i++){
                        vector<int> tmpVarList({funcIr->items[i+1]->iVal, });
                        vector<string> irAsmList;
                        allocater.allocateVar(irId, tmpVarList, irAsmList);
                        irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                        buffer << "MOV r" << i << ", r" << allocater.getVarRegId(funcIr->items[i+1]->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }
                }else if( paramStackSize != 0){
                    for(int i = 0; i < 4; i++){
                        vector<int> tmpVarList({funcIr->items[i+1]->iVal, });
                        vector<string> irAsmList;
                        allocater.allocateVar(irId, tmpVarList, irAsmList);
                        irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                        buffer << "MOV r" << i << ", r" << allocater.getVarRegId(funcIr->items[i+1]->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }

                    auto paramItor = funcIr->items.end() - 1;
                    for(int i = 0; i < paramStackSize; i++){
                        vector<int> tmpVarList({(*paramItor)->iVal, });
                        vector<string> irAsmList;
                        allocater.allocateVar(irId, tmpVarList, irAsmList);
                        irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                        buffer << "PUSH {r" << allocater.getVarRegId((*paramItor)->iVal) << "}";
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                        paramItor --;
                    }
                };

                // BL funcname
                irAsmVectorMap[irId].push_back("BL " + funcIr->items[0]->symbol->name);

                // reserve registers pop r0-r3, lr, ip
                if(paramStackSize != 0){
                    buffer << "ADD sp, sp, #" << paramStackSize * 4;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }

                if(paramRegSize == 0){
                    irAsmVectorMap[irId].push_back("POP {ip}");
                }else if(paramRegSize == 1){
                    irAsmVectorMap[irId].push_back("POP {r0, ip}");
                }else{
                    buffer << "POP {r0-r" << paramRegSize - 1 << ", ip}";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }
                break;
            }
            case IR::ARR:{
                // allocate for op1
                int op1Id = funcIr->items[0]->iVal;
                int op2Id = funcIr->items[1]->iVal;
                vector<int> tmpVarList({op1Id, op2Id});
                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                buffer << "ADD r" << allocater.getVarRegId(op1Id) << ", r" << allocater.getVarRegId(op1Id)
                       << ", r" << allocater.getVarRegId(op2Id);
                irAsmVectorMap[irId].push_back(buffer.str());
                buffer.clear();
                buffer.str("");
                break;
            }
            case IR::MUL:{
                // judge if is float!!
                int op1Id = funcIr->items[0]->iVal;
                int op2Id = funcIr->items[0]->iVal;
                IRItem *op3 = funcIr->items[2];
                vector<int> tmpVarList({op1Id, op2Id});

                if(op3->type == IRItem::IVAR || op3->type == IRItem::FVAR){
                    tmpVarList.push_back(op3->iVal);
                }

                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());

                if(op3->type == IRItem::IVAR){
                    buffer << "MUL r" << allocater.getVarRegId(op1Id) << ", r"<< allocater.getVarRegId(op2Id)
                           <<", r" << allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::FVAR){
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "MOV r1, r" << allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("BL __aeabi_fmul");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                }else if(op3->type == IRItem::INT){
                    buffer << "MUL r" << allocater.getVarRegId(op1Id) << ", r" << allocater.getVarRegId(op2Id)
                           << ", #" << op3->iVal;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::FLOAT){
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(*((unsigned *)(&(op3->fVal))) < 65535){
                        buffer << "MOV r1, #" << *((unsigned *)(&(op3->fVal)));
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else{
                        buffer << "MOVW r1, , #:lower16:" << *((unsigned *)(&(op3->fVal)));
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                        buffer << "MOVT r1, , #:upper16:" << *((unsigned *)(&(op3->fVal)));
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }


                    irAsmVectorMap[irId].push_back("BL __aeabi_fmul");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                }

                break;
            }
            case IR::DIV:{
                // judge if is float!!
                int op1Id = funcIr->items[0]->iVal;
                int op2Id = funcIr->items[0]->iVal;
                IRItem *op3 = funcIr->items[2];
                vector<int> tmpVarList({op1Id, op2Id});

                if(op3->type == IRItem::IVAR || op3->type == IRItem::FVAR){
                    tmpVarList.push_back(op3->iVal);
                }

                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());

                if(op3->type == IRItem::IVAR){
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "MOV r1, r" << allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("BL __aeabi_idiv");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                }else if(op3->type == IRItem::FVAR){
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "MOV r1, r" << allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("BL __aeabi_fdiv");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                }else if(op3->type == IRItem::INT){
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "MOV r1, #" << op3->iVal;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("BL __aeabi_idiv");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                }else if(op3->type == IRItem::FLOAT){
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(*((unsigned *)(&(op3->fVal))) < 65535){
                        buffer << "MOV r1, #" << *((unsigned *)(&(op3->fVal)));
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else{
                        buffer << "MOVW r1, , #:lower16:" << *((unsigned *)(&(op3->fVal)));
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                        buffer << "MOVT r1, , #:upper16:" << *((unsigned *)(&(op3->fVal)));
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }

                    irAsmVectorMap[irId].push_back("BL __aeabi_fdiv");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                }

                break;
            }
            case IR::SUB:{
                // judge if is float!!
                int op1Id = funcIr->items[0]->iVal;
                int op2Id = funcIr->items[0]->iVal;
                IRItem *op3 = funcIr->items[2];
                vector<int> tmpVarList({op1Id, op2Id});

                if(op3->type == IRItem::IVAR || op3->type == IRItem::FVAR){
                    tmpVarList.push_back(op3->iVal);
                }

                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());

                if(op3->type == IRItem::IVAR){
                    buffer << "SUB r" << allocater.getVarRegId(op1Id) << ", r"<< allocater.getVarRegId(op2Id)
                           <<", r" << allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::FVAR){
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "MOV r1, r" << allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("BL __aeabi_fsub");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                }else if(op3->type == IRItem::INT){
                    buffer << "SUB r" << allocater.getVarRegId(op1Id) << ", r" << allocater.getVarRegId(op2Id)
                           << ", #" << op3->iVal;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::FLOAT){
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(*((unsigned *)(&(op3->fVal))) < 65535){
                        buffer << "MOV r1, #" << *((unsigned *)(&(op3->fVal)));
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else{
                        buffer << "MOVW r1, , #:lower16:" << *((unsigned *)(&(op3->fVal)));
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                        buffer << "MOVT r1, , #:upper16:" << *((unsigned *)(&(op3->fVal)));
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }


                    irAsmVectorMap[irId].push_back("BL __aeabi_fsub");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                }

                break;
            }
            case IR::I2F:{
                int op1Id = funcIr->items[0]->iVal;
                int op2Id = funcIr->items[1]->iVal;
                vector<int> tmpVarList({op1Id, op2Id});
                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());

                irAsmVectorMap[irId].push_back("PUSH {r0, ip}");
                buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                irAsmVectorMap[irId].push_back(buffer.str());
                buffer.clear();
                buffer.str("");
                irAsmVectorMap[irId].push_back("BL __aeabi_i2f");
                buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                irAsmVectorMap[irId].push_back(buffer.str());
                buffer.clear();
                buffer.str("");
                irAsmVectorMap[irId].push_back("POP {r0, ip}");
                break;
            }
            case IR::F2I:{
                int op1Id = funcIr->items[0]->iVal;
                int op2Id = funcIr->items[1]->iVal;
                vector<int> tmpVarList({op1Id, op2Id});
                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());

                irAsmVectorMap[irId].push_back("PUSH {r0, ip}");
                buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                irAsmVectorMap[irId].push_back(buffer.str());
                buffer.clear();
                buffer.str("");
                irAsmVectorMap[irId].push_back("BL __aeabi_f2i");
                buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                irAsmVectorMap[irId].push_back(buffer.str());
                buffer.clear();
                buffer.str("");
                irAsmVectorMap[irId].push_back("POP {r0, ip}");
                break;
            }
            case IR::GOTO:{
                int gotoIrId = funcIr->items[0]->iVal;
                labelInsertList.push_back(gotoIrId);
                buffer << "GOTO " << funcSymbol->name << "_label" << gotoIrId;
                irAsmVectorMap[irId].push_back(buffer.str());
                buffer.clear();
                buffer.str("");
                break;
            }
            case IR::BEQ:{
                int gotoIrId = funcIr->items[0]->iVal;
                labelInsertList.push_back(gotoIrId);
                int op2Id = funcIr->items[1]->iVal;
                IRItem* op3 = funcIr->items[2];
                vector<int> tmpVarList({op2Id, });
                if(op3->type == IRItem::IVAR){
                    tmpVarList.push_back(op3->iVal);
                }
                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());

                if(op3->type == IRItem::IVAR){
                    buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", r" <<allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "BEQ " << funcSymbol->name << "_label" << gotoIrId;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::INT){
                    buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", #" <<allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "BEQ " << funcSymbol->name << "_label" << gotoIrId;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }
                break;
            }
            case IR::BGE:{
                int gotoIrId = funcIr->items[0]->iVal;
                labelInsertList.push_back(gotoIrId);
                int op2Id = funcIr->items[1]->iVal;
                IRItem* op3 = funcIr->items[2];
                vector<int> tmpVarList({op2Id, });
                if(op3->type == IRItem::IVAR){
                    tmpVarList.push_back(op3->iVal);
                }
                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());

                if(op3->type == IRItem::IVAR){
                    buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", r" <<allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "BGE " << funcSymbol->name << "_label" << gotoIrId;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::INT){
                    buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", #" <<allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "BGE " << funcSymbol->name << "_label" << gotoIrId;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }
                break;
            }
            case IR::BGT:{
                int gotoIrId = funcIr->items[0]->iVal;
                labelInsertList.push_back(gotoIrId);
                int op2Id = funcIr->items[1]->iVal;
                IRItem* op3 = funcIr->items[2];
                vector<int> tmpVarList({op2Id, });
                if(op3->type == IRItem::IVAR){
                    tmpVarList.push_back(op3->iVal);
                }
                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());

                if(op3->type == IRItem::IVAR){
                    buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", r" <<allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "BGT " << funcSymbol->name << "_label" << gotoIrId;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::INT){
                    buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", #" <<allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "BGT " << funcSymbol->name << "_label" << gotoIrId;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }
                break;
            }
            case IR::BLE:{
                int gotoIrId = funcIr->items[0]->iVal;
                labelInsertList.push_back(gotoIrId);
                int op2Id = funcIr->items[1]->iVal;
                IRItem* op3 = funcIr->items[2];
                vector<int> tmpVarList({op2Id, });
                if(op3->type == IRItem::IVAR){
                    tmpVarList.push_back(op3->iVal);
                }
                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());

                if(op3->type == IRItem::IVAR){
                    buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", r" <<allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "BLE " << funcSymbol->name << "_label" << gotoIrId;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::INT){
                    buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", #" <<allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "BLE " << funcSymbol->name << "_label" << gotoIrId;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }
                break;
            }
            case IR::BLT:{
                int gotoIrId = funcIr->items[0]->iVal;
                labelInsertList.push_back(gotoIrId);
                int op2Id = funcIr->items[1]->iVal;
                IRItem* op3 = funcIr->items[2];
                vector<int> tmpVarList({op2Id, });
                if(op3->type == IRItem::IVAR){
                    tmpVarList.push_back(op3->iVal);
                }
                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());

                if(op3->type == IRItem::IVAR){
                    buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", r" <<allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "BLT " << funcSymbol->name << "_label" << gotoIrId;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::INT){
                    buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", #" <<allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "BLT " << funcSymbol->name << "_label" << gotoIrId;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }
                break;
            }
            case IR::BNE:{
                int gotoIrId = funcIr->items[0]->iVal;
                labelInsertList.push_back(gotoIrId);
                int op2Id = funcIr->items[1]->iVal;
                IRItem* op3 = funcIr->items[2];
                vector<int> tmpVarList({op2Id, });
                if(op3->type == IRItem::IVAR){
                    tmpVarList.push_back(op3->iVal);
                }
                vector<string> irAsmList;
                allocater.allocateVar(irId, tmpVarList, irAsmList);
                irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());

                if(op3->type == IRItem::IVAR){
                    buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", r" <<allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "BNE " << funcSymbol->name << "_label" << gotoIrId;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::INT){
                    buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", #" <<allocater.getVarRegId(op3->iVal);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "BNE " << funcSymbol->name << "_label" << gotoIrId;
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }
                break;
            }
            case IR::GE:{
                int op1Id = funcIr->items[0]->iVal;
                int op2Id = funcIr->items[1]->iVal;
                IRItem *op3 = funcIr->items[2];
                if(op3->type == IRItem::FVAR || op3->type == IRItem::FLOAT){
                    vector<int> tmpVarList({op1Id, op2Id, });
                    if(op3->type == IRItem::FVAR){
                        tmpVarList.push_back(op3->iVal);
                    }
                    vector<string> irAsmList;
                    allocater.allocateVar(irId, tmpVarList, irAsmList);
                    irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(op3->type == IRItem::FVAR){
                        buffer << "MOV r1, r" << allocater.getVarRegId(op3->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else if(op3->type == IRItem::FLOAT){
                        if(*((unsigned *) (&(op3->fVal))) > 65535) {
                            buffer << "MOVW r1, #:lower16:" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                            buffer << "MOVT r1, #:upper16:" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }else{
                            buffer << "MOV r1, #" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }
                    }

                    irAsmVectorMap[irId].push_back("BL __aeabi_fcmpge");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                    buffer << "CMP r" << allocater.getVarRegId(op1Id) << ", #0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "MOVNE r" << allocater.getVarRegId(op1Id) << ", #1";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::IVAR || op3->type == IRItem::INT){
                    vector<int> tmpVarList({op1Id, op2Id, });
                    if(op3->type == IRItem::IVAR){
                        tmpVarList.push_back(op3->iVal);
                    }
                    vector<string> irAsmList;
                    allocater.allocateVar(irId, tmpVarList, irAsmList);
                    irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", #0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(op3->type == IRItem::IVAR){
                        buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", r" << allocater.getVarRegId(op3->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else if(op3->type == IRItem::INT){
                        buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", #" << op3->iVal;
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }
                    buffer << "MOVGE r" << allocater.getVarRegId(op1Id) << ", #1";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }
            }
            case IR::GT:{
                int op1Id = funcIr->items[0]->iVal;
                int op2Id = funcIr->items[1]->iVal;
                IRItem *op3 = funcIr->items[2];
                if(op3->type == IRItem::FVAR || op3->type == IRItem::FLOAT){
                    vector<int> tmpVarList({op1Id, op2Id, });
                    if(op3->type == IRItem::FVAR){
                        tmpVarList.push_back(op3->iVal);
                    }
                    vector<string> irAsmList;
                    allocater.allocateVar(irId, tmpVarList, irAsmList);
                    irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(op3->type == IRItem::FVAR){
                        buffer << "MOV r1, r" << allocater.getVarRegId(op3->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else if(op3->type == IRItem::FLOAT){
                        if(*((unsigned *) (&(op3->fVal))) > 65535) {
                            buffer << "MOVW r1, #:lower16:" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                            buffer << "MOVT r1, #:upper16:" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }else{
                            buffer << "MOV r1, #" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }
                    }

                    irAsmVectorMap[irId].push_back("BL __aeabi_fcmpgt");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                    buffer << "CMP r" << allocater.getVarRegId(op1Id) << ", #0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "MOVNE r" << allocater.getVarRegId(op1Id) << ", #1";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::IVAR || op3->type == IRItem::INT){
                    vector<int> tmpVarList({op1Id, op2Id, });
                    if(op3->type == IRItem::IVAR){
                        tmpVarList.push_back(op3->iVal);
                    }
                    vector<string> irAsmList;
                    allocater.allocateVar(irId, tmpVarList, irAsmList);
                    irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", #0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(op3->type == IRItem::IVAR){
                        buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", r" << allocater.getVarRegId(op3->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else if(op3->type == IRItem::INT){
                        buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", #" << op3->iVal;
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }
                    buffer << "MOVGT r" << allocater.getVarRegId(op1Id) << ", #1";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }
            }
            case IR::LE:{
                int op1Id = funcIr->items[0]->iVal;
                int op2Id = funcIr->items[1]->iVal;
                IRItem *op3 = funcIr->items[2];
                if(op3->type == IRItem::FVAR || op3->type == IRItem::FLOAT){
                    vector<int> tmpVarList({op1Id, op2Id, });
                    if(op3->type == IRItem::FVAR){
                        tmpVarList.push_back(op3->iVal);
                    }
                    vector<string> irAsmList;
                    allocater.allocateVar(irId, tmpVarList, irAsmList);
                    irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(op3->type == IRItem::FVAR){
                        buffer << "MOV r1, r" << allocater.getVarRegId(op3->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else if(op3->type == IRItem::FLOAT){
                        if(*((unsigned *) (&(op3->fVal))) > 65535) {
                            buffer << "MOVW r1, #:lower16:" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                            buffer << "MOVT r1, #:upper16:" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }else{
                            buffer << "MOV r1, #" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }
                    }

                    irAsmVectorMap[irId].push_back("BL __aeabi_fcmple");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                    buffer << "CMP r" << allocater.getVarRegId(op1Id) << ", #0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "MOVNE r" << allocater.getVarRegId(op1Id) << ", #1";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::IVAR || op3->type == IRItem::INT){
                    vector<int> tmpVarList({op1Id, op2Id, });
                    if(op3->type == IRItem::IVAR){
                        tmpVarList.push_back(op3->iVal);
                    }
                    vector<string> irAsmList;
                    allocater.allocateVar(irId, tmpVarList, irAsmList);
                    irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", #0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(op3->type == IRItem::IVAR){
                        buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", r" << allocater.getVarRegId(op3->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else if(op3->type == IRItem::INT){
                        buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", #" << op3->iVal;
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }
                    buffer << "MOVLE r" << allocater.getVarRegId(op1Id) << ", #1";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }
            }
            case IR::LT:{
                int op1Id = funcIr->items[0]->iVal;
                int op2Id = funcIr->items[1]->iVal;
                IRItem *op3 = funcIr->items[2];
                if(op3->type == IRItem::FVAR || op3->type == IRItem::FLOAT){
                    vector<int> tmpVarList({op1Id, op2Id, });
                    if(op3->type == IRItem::FVAR){
                        tmpVarList.push_back(op3->iVal);
                    }
                    vector<string> irAsmList;
                    allocater.allocateVar(irId, tmpVarList, irAsmList);
                    irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(op3->type == IRItem::FVAR){
                        buffer << "MOV r1, r" << allocater.getVarRegId(op3->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else if(op3->type == IRItem::FLOAT){
                        if(*((unsigned *) (&(op3->fVal))) > 65535) {
                            buffer << "MOVW r1, #:lower16:" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                            buffer << "MOVT r1, #:upper16:" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }else{
                            buffer << "MOV r1, #" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }
                    }

                    irAsmVectorMap[irId].push_back("BL __aeabi_fcmplt");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                    buffer << "CMP r" << allocater.getVarRegId(op1Id) << ", #0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "MOVNE r" << allocater.getVarRegId(op1Id) << ", #1";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::IVAR || op3->type == IRItem::INT){
                    vector<int> tmpVarList({op1Id, op2Id, });
                    if(op3->type == IRItem::IVAR){
                        tmpVarList.push_back(op3->iVal);
                    }
                    vector<string> irAsmList;
                    allocater.allocateVar(irId, tmpVarList, irAsmList);
                    irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", #0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(op3->type == IRItem::IVAR){
                        buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", r" << allocater.getVarRegId(op3->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else if(op3->type == IRItem::INT){
                        buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", #" << op3->iVal;
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }
                    buffer << "MOVLT r" << allocater.getVarRegId(op1Id) << ", #1";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }
            }
            case IR::EQ:{
                int op1Id = funcIr->items[0]->iVal;
                int op2Id = funcIr->items[1]->iVal;
                IRItem *op3 = funcIr->items[2];
                if(op3->type == IRItem::FVAR || op3->type == IRItem::FLOAT){
                    vector<int> tmpVarList({op1Id, op2Id, });
                    if(op3->type == IRItem::FVAR){
                        tmpVarList.push_back(op3->iVal);
                    }
                    vector<string> irAsmList;
                    allocater.allocateVar(irId, tmpVarList, irAsmList);
                    irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(op3->type == IRItem::FVAR){
                        buffer << "MOV r1, r" << allocater.getVarRegId(op3->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else if(op3->type == IRItem::FLOAT){
                        if(*((unsigned *) (&(op3->fVal))) > 65535) {
                            buffer << "MOVW r1, #:lower16:" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                            buffer << "MOVT r1, #:upper16:" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }else{
                            buffer << "MOV r1, #" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }
                    }

                    irAsmVectorMap[irId].push_back("BL __aeabi_fcmpeq");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                    buffer << "CMP r" << allocater.getVarRegId(op1Id) << ", #0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "MOVNE r" << allocater.getVarRegId(op1Id) << ", #1";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::IVAR || op3->type == IRItem::INT){
                    vector<int> tmpVarList({op1Id, op2Id, });
                    if(op3->type == IRItem::IVAR){
                        tmpVarList.push_back(op3->iVal);
                    }
                    vector<string> irAsmList;
                    allocater.allocateVar(irId, tmpVarList, irAsmList);
                    irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", #0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(op3->type == IRItem::IVAR){
                        buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", r" << allocater.getVarRegId(op3->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else if(op3->type == IRItem::INT){
                        buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", #" << op3->iVal;
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }
                    buffer << "MOVEQ r" << allocater.getVarRegId(op1Id) << ", #1";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }
            }
            case IR::NE:{
                int op1Id = funcIr->items[0]->iVal;
                int op2Id = funcIr->items[1]->iVal;
                IRItem *op3 = funcIr->items[2];
                if(op3->type == IRItem::FVAR || op3->type == IRItem::FLOAT){
                    vector<int> tmpVarList({op1Id, op2Id, });
                    if(op3->type == IRItem::FVAR){
                        tmpVarList.push_back(op3->iVal);
                    }
                    vector<string> irAsmList;
                    allocater.allocateVar(irId, tmpVarList, irAsmList);
                    irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                    irAsmVectorMap[irId].push_back("PUSH {r0, r1, ip}");
                    buffer << "MOV r0, r" << allocater.getVarRegId(op2Id);
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(op3->type == IRItem::FVAR){
                        buffer << "MOV r1, r" << allocater.getVarRegId(op3->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else if(op3->type == IRItem::FLOAT){
                        if(*((unsigned *) (&(op3->fVal))) > 65535) {
                            buffer << "MOVW r1, #:lower16:" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                            buffer << "MOVT r1, #:upper16:" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }else{
                            buffer << "MOV r1, #" << *((unsigned *) (&(op3->fVal)));
                            irAsmVectorMap[irId].push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }
                    }

                    irAsmVectorMap[irId].push_back("BL __aeabi_fcmpne");
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", r0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    irAsmVectorMap[irId].push_back("POP {r0, r1, ip}");
                    buffer << "CMP r" << allocater.getVarRegId(op1Id) << ", #0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    buffer << "MOVNE r" << allocater.getVarRegId(op1Id) << ", #1";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }else if(op3->type == IRItem::IVAR || op3->type == IRItem::INT){
                    vector<int> tmpVarList({op1Id, op2Id, });
                    if(op3->type == IRItem::IVAR){
                        tmpVarList.push_back(op3->iVal);
                    }
                    vector<string> irAsmList;
                    allocater.allocateVar(irId, tmpVarList, irAsmList);
                    irAsmVectorMap[irId].insert(irAsmVectorMap[irId].end(), irAsmList.begin(), irAsmList.end());
                    buffer << "MOV r" << allocater.getVarRegId(op1Id) << ", #0";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                    if(op3->type == IRItem::IVAR){
                        buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", r" << allocater.getVarRegId(op3->iVal);
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }else if(op3->type == IRItem::INT){
                        buffer << "CMP r" << allocater.getVarRegId(op2Id) << ", #" << op3->iVal;
                        irAsmVectorMap[irId].push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }
                    buffer << "MOVNE r" << allocater.getVarRegId(op1Id) << ", #1";
                    irAsmVectorMap[irId].push_back(buffer.str());
                    buffer.clear();
                    buffer.str("");
                }
            }
            case IR::NOT:{
                break;
            }
            case IR::NEG:{
                break;
            }
            case IR::MEMSET_ZERO:{
                break;
            }
            default :{
                irAsmVectorMap[irId].push_back("Not written...");
            }
        }
        irId += 1;
    }
    // cout << "instruction insertion done" << endl;
    // backFill func parameters after knowing stack
    set<int> usedRegister = allocater.getUsedRegister();
    for(tuple<int, int, int, Symbol *> backFillInfo : backFillList){
        string initialAsm(irAsmVectorMap[get<1>(backFillInfo)][get<2>(backFillInfo)]);
        buffer << (usedRegister.size() +
                   funcParam.getLocationId(get<3>(backFillInfo)))*4;
        if(get<0>(backFillInfo) == 1) {  // need "]"
            buffer << "]";
        }
        string updatedAsm = initialAsm + buffer.str();
        buffer.clear();
        buffer.str("");
        irAsmVectorMap[get<1>(backFillInfo)][get<2>(backFillInfo)] = updatedAsm;
    }

    // insert labels for text section
    for(int labelInsertIr : labelInsertList){
        buffer << funcSymbol->name << "_label" << labelInsertIr << ":";
        irAsmVectorMap[labelInsertIr].insert(irAsmVectorMap[labelInsertIr].begin(), buffer.str());
        buffer.clear();
        buffer.str("");
    }

    // insert single ir asm vector to asmVector
    for(int i = 0; i < funcIR.size(); i++){
        asmVector.insert(asmVector.end(), irAsmVectorMap[i].begin(), irAsmVectorMap[i].end());
    }

    // insert environment saving and recovering code
    int offsetFlag = 0;
    if(useLR || usedRegister.size() != 0){
        offsetFlag = 1;
        buffer <<  "PUSH {";
        for(int saveRegId : usedRegister){
            buffer << "r" << saveRegId << ",";
        }
        if(useLR){
            buffer << "lr,";
        }
        string savingAsm = buffer.str();
        buffer.clear();
        buffer.str("");
        *(savingAsm.end()-1) = '}';
        asmVector.insert(asmVector.begin() + firstAsmIndex, savingAsm);
    }

    asmVector.insert(asmVector.begin() + firstAsmIndex + offsetFlag, "MOV ip, sp");

    if(useLR || usedRegister.size() != 0){
        buffer << "MOV sp, ip";
        asmVector.insert(asmVector.end() - 1, buffer.str());
        buffer.clear();
        buffer.str("");

        buffer <<  "POP {";
        for(int saveRegId : usedRegister){
            buffer << "r" << saveRegId << ",";
        }
        if(useLR){
            buffer << "lr,";
        }
        string recoveringAsm = buffer.str();
        buffer.clear();
        buffer.str("");
        *(recoveringAsm.end()-1) = '}';
        asmVector.insert(asmVector.end() - 1, recoveringAsm);
    }
}

void Assembler::generateFunctionAsm() {
    vector<pair<Symbol *, vector<IR *>>> funcList = this->irBuild.getFuncs();

    this->asmVector.push_back(string(".text"));

    for(pair<Symbol *, vector<IR *>> func : funcList){
        singleFunctionAsm(func);
    }
}