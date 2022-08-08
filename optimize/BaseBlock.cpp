#include <algorithm>
#include <iostream>
#include <queue>
#include <utility>

#include "BaseBlock.h"

using namespace std;

string BaseBlock::toString(){
    string bbstring;
    bbstring = ("Block"+to_string(BlockId)+":\n");
    for(IR* ir: this->IRlist){
        bbstring += ir->toString()+"\n";
    }
    /*
    bbstring += "FlowIn:\n";
    for(BaseBlock* flow : this->BlockIn){
        bbstring += "Block"+to_string(flow->BlockId)+" ";
    }
    bbstring += "\n";
    bbstring += "FlowOut:\n";
    for(BaseBlock* flow : this->BlockOut){
        bbstring += "Block"+to_string(flow->BlockId)+" ";
    }
    bbstring += "\n";*/
    return bbstring;
}