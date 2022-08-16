#ifndef __BLOCK_BUILD__
#define __BLOCK_BUILD__
#include <utility>
#include <vector>
#include "../Ir/IR.h"
#include "BaseBlock.h"
//static int blockNode;
using namespace std;
class BlockBuild {
public:
    vector<pair<BaseBlock *, BaseBlock *>> backEdge;
    vector<set<BaseBlock *>> loop;
    vector<BaseBlock *> generateFunctionBlock(vector<IR *> IRList);
    void checkBlockType();
    BlockBuild(){}
    ~BlockBuild(){}
};

#endif