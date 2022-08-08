#ifndef __BLOCK_BUILD__
#define __BLOCK_BUILD__
#include <utility>
#include <vector>
#include "../Ir/IR.h"
#include "BaseBlock.h"
static int blockNode;
using namespace std;
class BlockBuild {
public:
    vector<BaseBlock *> generateFunctionBlock(vector<IR *> IRList);
    void checkBlcokType();
    BlockBuild(){}
    ~BlockBuild(){}
};

#endif