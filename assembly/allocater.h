//
// Created by joelyang on 2022/7/28.
//

#ifndef HUST_ALLOCATER_H
#define HUST_ALLOCATER_H
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
using namespace std;

class Allocater {
private:
    unordered_map<int, pair<int, int>> tmpVarLiveInterval;
    unordered_map<int, pair<int, int>> tmpVarLocationMap;  // (0 means reg, 1 means stack)
    vector<pair<int, int>> activeList;
    set<int> freeRegister;
    set<int> freeStack;
    set<int> usedRegister;  // record used registers for later environment saving
    int funcLocalVarSize;
    int tmpVarStackOffset;

public:
    // constructor and destructor(default)
    Allocater(unordered_map<int, pair<int, int>> tmpVarLiveInterval, int funcLocalVarSize)
              : tmpVarLiveInterval(tmpVarLiveInterval), tmpVarStackOffset(0) , funcLocalVarSize(funcLocalVarSize),
              freeRegister(set({4, 5, 6})){};

    // allocate for variable
    void allocateVar(int irId, vector<int> varIdList, vector<string> &extraInstruction);
    // get position of variable
    int getVarRegId(int varId);
    set<int> getUsedRegister();
};


#endif //HUST_ALLOCATER_H
