#include <algorithm>
#include <iostream>
#include <queue>
#include <utility>

#include "BaseBlock.h"

using namespace std;

vector<BaseBlock *> BaseBlock::getBlockIn()
{
    return this->BlockIn;
}

vector<BaseBlock *> BaseBlock::getBlockOut()
{
    return this->BlockOut;
}

string BaseBlock::toString(){
    string bbstring;
    bbstring = ("Block"+to_string(BlockId)+":\n");
    for(IR* ir: this->IRlist){
        bbstring += ir->toString()+"\n";
    }
    
    bbstring += "FlowIn:\n";
    for(BaseBlock* flow : this->BlockIn){
        if(flow->BlockId == -1)
            break;
        bbstring += "Block" + to_string(flow->BlockId) + " ";
    }
    bbstring += "\n";
    bbstring += "FlowOut:\n";
    for(BaseBlock* flow : this->BlockOut){
        if(flow->BlockId == -1)
            break;
        bbstring += "Block"+to_string(flow->BlockId)+" ";
    }
    bbstring += "\n";
    bbstring += "Dom:\n";
    for(BaseBlock* flow : this->domS){
        bbstring += "Block"+to_string(flow->BlockId)+" ";
    }
    return bbstring;
}