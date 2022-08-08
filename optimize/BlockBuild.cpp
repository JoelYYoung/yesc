#include <algorithm>
#include <iostream>
#include <queue>
#include <utility>
#include <map>

#include "BlockBuild.h"

using namespace std;

vector<BaseBlock *> BlockBuild::generateFunctionBlock(vector<IR *> IRList){
    vector<BaseBlock *> blocklist;
    vector<int> blockbegin;
    vector<int> blockNum;
    map<int, int> blockmp;
    IR * irtemp;
    blockbegin.push_back(IRList[0]->irId);
    for(IR* ir : IRList){
        if (ir->type == IR::BEQ || ir->type == IR::BNE)
        {
            blockbegin.push_back(ir->irId+1);
            blockbegin.push_back(ir->items.at(0)->iVal);
        }
        else if(ir->type==IR::GOTO){
            blockbegin.push_back(ir->irId+1);
            blockbegin.push_back(ir->items.at(0)->iVal);
        }
    }
    sort(blockbegin.begin(),blockbegin.end());
    blockbegin.erase(unique(blockbegin.begin(),blockbegin.end()),blockbegin.end());
    blockbegin.push_back(IRList.back()->irId+1);
    int firstBlockNode = blockNode;
    for (int i = 0; i < blockbegin.size() - 1; i++)
    {
        vector<IR*> bbir;
        BaseBlock* bb = new BaseBlock(blockNode++);
        blockmp[blockbegin[i]] = i;
        for (int j = blockbegin[i] - IRList[0]->irId; j < blockbegin[i + 1] - IRList[0]->irId; j++)
        {
            bbir.push_back(IRList[j]);
            blockNum.push_back(blockNode-1);
        }
        bb->setIRlist(bbir);
        blocklist.push_back(bb);
    }
    
    for(BaseBlock* bb :blocklist){
        irtemp = bb->getLastIR();
        if(irtemp == nullptr) break;
        if (irtemp->type == IR::BEQ || irtemp->type == IR::BNE)
        {
            int jumpId = irtemp->items.at(0)->iVal;
            int nextId = irtemp->irId + 1;
            bb->insertFlowIn(blocklist[blockmp[nextId]]);
            bb->insertFlowIn(blocklist[blockmp[jumpId]]);
        }
        else if(irtemp->type==IR::GOTO){
            int jumpId = irtemp->items.at(0)->iVal;
            int nextId = irtemp->irId + 1;
            bb->insertFlowIn(blocklist[blockmp[nextId]]);
            bb->insertFlowIn(blocklist[blockmp[jumpId]]);
        }
        else{
            int nextId = irtemp->irId + 1;
            bb->insertFlowIn(blocklist[blockmp[nextId]]);
        }
    }
    return blocklist;
}