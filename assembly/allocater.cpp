//
// Created by joelyang on 2022/7/28.
//

#include "allocater.h"
#include <algorithm>
#include <unordered_set>
#include <sstream>
#include <string.h>

// active list sort function
bool cmp(pair<int, int> a, pair<int, int> b){
    if(b.second > a.second){
        return true;
    }else{
        return false;
    }
}

void Allocater::allocateVar(int irId, vector<int> varIdList, vector<string> &extraInstruction) {
    unordered_set<int> varIdSet(varIdList.begin(), varIdList.end());
    stringstream buffer;

    // free used register in activeList
    sort(activeList.begin(), activeList.end(), cmp);
    auto activeListItor = activeList.begin();
    while(activeListItor != activeList.end() && (*activeListItor).second < irId){
        int tmpVarId = (*activeListItor).first;
        int tmpVarRegId = tmpVarLocationMap[tmpVarId].second;
        freeRegister.insert(tmpVarRegId);
        tmpVarLocationMap.erase(tmpVarId);
        activeListItor = activeList.erase(activeListItor);
    }

    for (int varId : varIdList){

        auto op1Location = tmpVarLocationMap.find(varId);
        if( op1Location == tmpVarLocationMap.end()){  // not allocated, allocate reg
            // allocate or spill and allocate
            if(freeRegister.size() != 0){  // allocate
                int allocateRegId = *freeRegister.begin();
                tmpVarLocationMap[varId] = pair<int, int>(0, allocateRegId);
                activeList.push_back(pair<int, int>(varId, tmpVarLiveInterval[varId].second));
                freeRegister.erase(freeRegister.begin());
                usedRegister.insert(allocateRegId);
            }else{  // spill and allocate
                auto spillItor = activeList.end() - 1;
                while(true){
                    if(varIdSet.find((*spillItor).first) != varIdSet.end()){
                        spillItor -= 1;
                        continue;
                    }else{
                        break;
                    }
                }
                int spillVarId = (*spillItor).first;
                int spillVarRegId = tmpVarLocationMap[spillVarId].second;
                if(freeStack.size() == 0){  // increase stack size
                    tmpVarStackOffset += 1;
                    tmpVarLocationMap[spillVarId] = pair<int, int> (1, tmpVarStackOffset);
                    tmpVarLocationMap[varId] = pair<int, int> (0, spillVarRegId);
                    (*spillItor).first = varId;
                    (*spillItor).second = tmpVarLiveInterval[varId].second;
//                    string stackIncIntruction("SUB sp, sp, #4");
//                    extraInstruction.push_back(stackIncIntruction);

                    int spillStackOffset = (tmpVarStackOffset+funcLocalVarSize)*4;
                    if(spillStackOffset < 255){
                        buffer << "STR r" << spillVarRegId << ", [ip, #-" << spillStackOffset << "]";
                        string spillIntruction(buffer.str());
                        extraInstruction.push_back(spillIntruction);
                        buffer.clear();
                        buffer.str("");
                    }else{
                        spillStackOffset = -spillStackOffset;
                        buffer << "MOVW r11, #:lower16:"
                               << *((unsigned *) (&(spillStackOffset)));
                        extraInstruction.push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                        buffer << "MOVT r11, #:upper16:"
                               << *((unsigned *) (&(spillStackOffset)));
                        extraInstruction.push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");

                        buffer << "STR r" << spillVarRegId << ", [ip, r11]";
                        extraInstruction.push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }


                }else{
                    int spillStackOffset = *freeStack.begin();
                    freeStack.erase(freeStack.begin());
                    tmpVarLocationMap[spillVarId] = pair<int, int> (1, spillStackOffset);
                    tmpVarLocationMap[varId] = pair<int, int> (0, spillVarRegId);
                    (*spillItor).first = varId;
                    (*spillItor).second = tmpVarLiveInterval[varId].second;

                    int spillStackOffset_1 = (spillStackOffset+funcLocalVarSize)*4;
                    if(spillStackOffset_1 < 255){
                        buffer << "STR r" << spillVarRegId << ", [ip, #-" << spillStackOffset_1 << "]";
                        string spillIntruction(buffer.str());
                        extraInstruction.push_back(spillIntruction);
                        buffer.clear();
                        buffer.str("");
                    }else{
                        spillStackOffset_1 = -spillStackOffset_1;
                        buffer << "MOVW r11, #:lower16:"
                               << *((unsigned *) (&(spillStackOffset_1)));
                        extraInstruction.push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                        buffer << "MOVT r11, #:upper16:"
                               << *((unsigned *) (&(spillStackOffset_1)));
                        extraInstruction.push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");

                        buffer << "STR r" << spillVarRegId << ", [ip, r11]";
                        extraInstruction.push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }
                }
            }

        }else{  // allocated
            if(op1Location->second.first == 0){  // in register
                continue;
            }else {  // in stack
                // allocate or spill and allocate
                int freeStackId = tmpVarLocationMap[varId].second;
                if(freeRegister.size() != 0){  // allocate
                    int allocateRegId = *freeRegister.begin();
                    tmpVarLocationMap[varId] = pair<int, int>(0, allocateRegId);
                    activeList.push_back(pair<int, int>(varId, tmpVarLiveInterval[varId].second));
                    freeRegister.erase(freeRegister.begin());
                    usedRegister.insert(allocateRegId);
                    freeStack.insert(freeStackId);

                    int spillStackOffset_1 = (freeStackId + funcLocalVarSize) * 4;
                    if(spillStackOffset_1 < 255){
                        buffer << "LDR r" << allocateRegId << ", [ip, #-" << spillStackOffset_1 << "]";
                        string loadInstruction(buffer.str());
                        extraInstruction.push_back(loadInstruction);
                        buffer.clear();
                        buffer.str("");
                    }else{
                        spillStackOffset_1 = -spillStackOffset_1;
                        buffer << "MOVW r11, #:lower16:"
                               << *((unsigned *) (&(spillStackOffset_1)));
                        extraInstruction.push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                        buffer << "MOVT r11, #:upper16:"
                               << *((unsigned *) (&(spillStackOffset_1)));
                        extraInstruction.push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");

                        buffer << "LDR r" << allocateRegId << ", [ip, r11]";
                        extraInstruction.push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }
                }else{  // spill and allocate
                    auto spillItor = activeList.end() - 1;
                    while(true){
                        if(varIdSet.find((*spillItor).first) != varIdSet.end()){
                            spillItor -= 1;
                            continue;
                        }else{
                            break;
                        }
                    }
                    int spillVarId = (*spillItor).first;
                    int spillVarRegId = tmpVarLocationMap[spillVarId].second;
                    if(freeStack.size() == 0){  // increase stack size
                        tmpVarStackOffset += 1;
                        tmpVarLocationMap[spillVarId] = pair<int, int> (1, tmpVarStackOffset);
                        tmpVarLocationMap[varId] = pair<int, int> (0, spillVarRegId);
                        (*spillItor).first = varId;
                        (*spillItor).second = tmpVarLiveInterval[varId].second;
//                        string stackIncIntruction("SUB sp, sp, #4");
//                        extraInstruction.push_back(stackIncIntruction);

                        int spillStackOffset_1 = (tmpVarStackOffset+funcLocalVarSize)*4;
                        if(spillStackOffset_1 < 255){
                            buffer << "STR r" << spillVarRegId << ", [ip, #-" << spillStackOffset_1 << "]";
                            string spillIntruction(buffer.str());
                            extraInstruction.push_back(spillIntruction);
                            buffer.clear();
                            buffer.str("");
                        }else{
                            spillStackOffset_1 = -spillStackOffset_1;
                            buffer << "MOVW r11, #:lower16:"
                                   << *((unsigned *) (&(spillStackOffset_1)));
                            extraInstruction.push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                            buffer << "MOVT r11, #:upper16:"
                                   << *((unsigned *) (&(spillStackOffset_1)));
                            extraInstruction.push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");

                            buffer << "STR r" << spillVarRegId << ", [ip, r11]";
                            extraInstruction.push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }


                    }else{
                        int spillStackOffset = *freeStack.begin();
                        freeStack.erase(freeStack.begin());
                        tmpVarLocationMap[spillVarId] = pair<int, int> (1, spillStackOffset);
                        tmpVarLocationMap[varId] = pair<int, int> (0, spillVarRegId);
                        (*spillItor).first = varId;
                        (*spillItor).second = tmpVarLiveInterval[varId].second;

                        int spillStackOffset_1 = (spillStackOffset+funcLocalVarSize)*4;
                        if(spillStackOffset_1 < 255){
                            buffer << "STR r" << spillVarRegId << ", [ip, #-" << spillStackOffset_1 << "]";
                            string spillIntruction(buffer.str());
                            extraInstruction.push_back(spillIntruction);
                            buffer.clear();
                            buffer.str("");
                        }else{
                            spillStackOffset_1 = -spillStackOffset_1;
                            buffer << "MOVW r11, #:lower16:"
                                   << *((unsigned *) (&(spillStackOffset_1)));
                            extraInstruction.push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                            buffer << "MOVT r11, #:upper16:"
                                   << *((unsigned *) (&(spillStackOffset_1)));
                            extraInstruction.push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");

                            buffer << "STR r" << spillVarRegId << ", [ip, r11]";
                            extraInstruction.push_back(buffer.str());
                            buffer.clear();
                            buffer.str("");
                        }


                    }
                    freeStack.insert(freeStackId);

                    int spillStackOffset_1 = (freeStackId + funcLocalVarSize) * 4;
                    if(spillStackOffset_1 < 255){
                        buffer << "LDR r" << spillVarRegId << ", [ip, #-" << spillStackOffset_1 << "]";
                        string loadInstruction(buffer.str());
                        extraInstruction.push_back(loadInstruction);
                        buffer.clear();
                        buffer.str("");
                    }else{
                        spillStackOffset_1 = -spillStackOffset_1;
                        buffer << "MOVW r11, #:lower16:"
                               << *((unsigned *) (&(spillStackOffset_1)));
                        extraInstruction.push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                        buffer << "MOVT r11, #:upper16:"
                               << *((unsigned *) (&(spillStackOffset_1)));
                        extraInstruction.push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");

                        buffer << "LDR r" << spillVarRegId << ", [ip, r11]";
                        extraInstruction.push_back(buffer.str());
                        buffer.clear();
                        buffer.str("");
                    }
                }
            }
        }
    }
}

int Allocater::getVarRegId(int varId){
    auto varLocation = tmpVarLocationMap.find(varId);
    if( varLocation != tmpVarLocationMap.end() && varLocation->second.first == 0){  // in reg
        return varLocation->second.second;
    }else{  // not int reg, not allocated or in stack
        return -1;
    }
}

set<int> Allocater::getUsedRegister() {
    return usedRegister;
}

void Allocater::prepareForCall(int paramNum, vector<string> &extraInstruction) {
    if(freeStack.size() < paramNum){
        stringstream buffer;
        int incNum = paramNum - freeStack.size();
        for(int i = 0; i < incNum; i++){
            tmpVarStackOffset += 1;
            freeStack.insert(tmpVarStackOffset);
        }
//        buffer << "SUB sp, sp, #" << incNum * 4;
//        string stackIncInstruction(buffer.str());
//        extraInstruction.push_back(stackIncInstruction);
        buffer.clear();
        buffer.str("");
    }
    return;
}

int Allocater::getTmpVarStackOffset() {
    return tmpVarStackOffset;
}