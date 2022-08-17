//
// Created by joelyang on 2022/8/15.
//

#ifndef HUST_ASTOPTIMIZER_H
#define HUST_ASTOPTIMIZER_H
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "../syntax/analyse.h"
//#include "../syntax/symbolTable.h"

class AstOptimizer {
private:
    unordered_map<Symbol *, unordered_set<Symbol *>> dependencyGraph;
    unordered_map<parseNode *, pair<vector<Symbol *>, vector<Symbol *>>> defUseMap;
    unordered_set<Symbol *> criticalVariableSet;
    parseNode *rootNode;
    Analyse *analyse;

    //recursively generate dependencyGraph
    void genDepGraphRec(parseNode *astNode, Symbol * fatherDefSymbol);
    void genCriticalVarRec(parseNode *astNode);
    void optimizeAstRec(parseNode *astNode);
    bool haveFuncCallRec(parseNode *astNode);
    //recursively generate def and use Map
    //void genDefUseMapRec();

public:
    AstOptimizer(parseNode *rootNode, Analyse *analyse): rootNode(rootNode), analyse(analyse){};
    void generateDependencyGraph();
    void generateCriticalVariableSet();  //4 types of critical variables global variable, array param, return, control flow related
    void optimizeAst();
    //void generateDefUseMap();
};


#endif //HUST_ASTOPTIMIZER_H
