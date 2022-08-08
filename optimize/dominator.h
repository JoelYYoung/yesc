#ifndef DOMINATOR_H
#define DOMINATOR_H

#include "BaseBlock.h"
#include <list>
#include <map>
#include <set>

class Dominator {
public:
  Dominator() {}
  void run();
  void createDoms();
  void createReversePostOrder();
  void createIDom();
  void createDominanceFrontier();
  void createDomTreeSucc();

  // for debug
  void printIDom();
  void printDominanceFrontier();

private:
  void postOrderVisit(BaseBlock *bb, std::set<BaseBlock *> &visited);
  BaseBlock *intersect(BaseBlock *b1, BaseBlock *b2);

  std::list<BaseBlock *> reversePostOrder_;
  std::map<BaseBlock *, int> postOrderID_; // the root has highest ID
};

#endif // SYSYC_DOMINATORS_H
