#include "dominator.h"


void Dominator::createIDom() {
  // // init
  // for (auto bb : f->getBasicBlocks())
  //   bb->setIDom(nullptr);
  // auto root = f->getEntryBlock();
  // root->setIDom(root);

  // // iterate
  // bool changed = true;
  // while (changed) {
  //   changed = false;
  //   for (auto bb : this->reversePostOrder_) {
  //     if (bb == root) {
  //       continue;
  //     }

  //     // find one pred which has idom
  //     BasicBlock *pred = nullptr;
  //     for (auto p : bb->getPreBasicBlocks()) {
  //       if (p->getIDom()) {
  //         pred = p;
  //         break;
  //       }
  //     }
  //     exit_ifnot(_assertPredFalse_createIDom_Dominators, pred);

  //     BasicBlock *new_idom = pred;
  //     for (auto p : bb->getPreBasicBlocks()) {
  //       if (p == pred)
  //         continue;
  //       if (p->getIDom()) {
  //         new_idom = intersect(p, new_idom);
  //       }
  //     }
  //     if (bb->getIDom() != new_idom) {
  //       bb->setIDom(new_idom);
  //       changed = true;
  //     }
  //   }
  // }
}