#ifndef BASEBLOCK_H
#define BASEBLOCK_H
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <list>
#include "../Ir/IR.h"
using namespace std;
class Function;

class BaseBlock {
public:
  enum BlockType {
    Basic,
    If,
    While,
  };
  int BlockId;
  bool isBasicBlock() const { return block_type == Basic; }
  bool isIfBlock() const { return block_type == If; }
  bool isWhileBlock() const { return block_type == While; }

  void setBlockType(BlockType type) { block_type = type; }
  void setBaseFather(BaseBlock *father) {
    this->father = father;
  }
  void clearFather() { this->father = nullptr; }

  void addDom(BaseBlock *bb) { domS.insert(bb); }
  std::set<BaseBlock *> &getDoms() { return domS; }
  void setDoms(std::set<BaseBlock *> &doms) {
    domS.clear();
    domS.insert(doms.begin(), doms.end());
  }

  void setIRlist(vector<IR *> IRlist) { this->IRlist = IRlist; }
  void setBlockId(int BlockId){this->BlockId = BlockId;}
  void insertFlowIn(BaseBlock* bb){this->BlockIn.push_back(bb);}
  void insertFlowOut(BaseBlock* bb){this->BlockOut.push_back(bb);}

  BaseBlock *getBaseFather() const { return this->father; }
  IR* getLastIR(){
    if(IRlist.size()>0)
      return IRlist.back();
    else return nullptr;
    }
  
  BaseBlock(){}
  BaseBlock(int BlockId){this->BlockId = BlockId;}
  BaseBlock(int BlockId,vector<IR *> IRlist){this->BlockId = BlockId;this->IRlist = IRlist;}
  ~BaseBlock();
  string toString();

protected:
  vector<IR *> IRlist;
  vector<BaseBlock *> BlockIn;
  vector<BaseBlock*> BlockOut;
  set<BaseBlock*> domS; //Blcok的支配结点集
  BlockType block_type;
  BaseBlock *father;
};



#endif // BASEBLOCK_H