//
// Created by joelyang on 2022/8/15.
//

#include "AstOptimizer.h"

bool AstOptimizer::haveFuncCallRec(parseNode *astNode){
    bool flag = false;
    vector<parseNode *> childNodeList = astNode->nodes;
    if(astNode->parseType == parseNode::FUNC_CALL) return true;
    for(int i = 0; i < childNodeList.size(); i++){
        if(haveFuncCallRec(childNodeList[i])) flag = true;
    }
    return flag;
}

bool AstOptimizer::generateFuncCallDelInfoRec(parseNode *astNode, bool fatherIsExp, Symbol *funcSymbol) {
    bool haveGlobalAssign = false;
    bool isExp = false;
    if(astNode->parseType == parseNode::ASSIGN_STMT){
        if(astNode->nodes[0]->symbol->symbolType == Symbol::GLOBAL_VAR){
            haveGlobalAssign = true;
        }
    }else if(astNode->parseType == parseNode::EXP_STMT){
        isExp = true;
    }else if(astNode->parseType == parseNode::FUNC_CALL){
        // add into funcCalledGraph
        if(fatherIsExp){
            funcCalledGraph[astNode->symbol].push_back(pair<Symbol *, int>(funcSymbol, 1));
        }else{
            funcCalledGraph[astNode->symbol].push_back(pair<Symbol *, int>(funcSymbol, 0));
        }
        callOtherFuncSet.insert(funcSymbol);
    }

    vector<parseNode *> childNodeList = astNode->nodes;
    for(int i = 0; i < childNodeList.size(); i++){
        bool tmp = generateFuncCallDelInfoRec(childNodeList[i], isExp, funcSymbol);
        if(tmp) haveGlobalAssign = true;
    }
    return haveGlobalAssign;
}

void AstOptimizer::generateFuncCallDelInfo() {
    vector<parseNode *> childNodeList = rootNode->nodes;
    Symbol *mainFuncSymbol = NULL;
    for(int i = 0; i < childNodeList.size(); i++){
        if(childNodeList[i]->parseType == parseNode::FUNC_DEF){
            // find out main function first
            if(childNodeList[i]->symbol->name == "main"){
                mainFuncSymbol = childNodeList[i]->symbol;
            }

            // if params have array?
            for(Symbol *funcParam : childNodeList[i]->symbol->params){
                if(funcParam->dimensions.size() != 0){
                    funcOutInfluenceMap[childNodeList[i]->symbol] = true;
                }
            }
//            if(callOtherFuncSet.find(childNodeList[i]->symbol) != callOtherFuncSet.end()){
//                funcOutInfluenceMap[childNodeList[i]->symbol] = true;
//            }
            if(funcOutInfluenceMap.find(childNodeList[i]->symbol) == funcOutInfluenceMap.end()){
                funcOutInfluenceMap[childNodeList[i]->symbol] = false;
            }

            // recursively detect funcCall Graph and if have global assignment
            bool haveGlobalAssign = generateFuncCallDelInfoRec(childNodeList[i], false, childNodeList[i]->symbol);
            funcOutInfluenceMap[childNodeList[i]->symbol] = funcOutInfluenceMap[childNodeList[i]->symbol] || haveGlobalAssign;
        }
    }

    for(int i = 0; i < childNodeList.size(); i++){
        if(childNodeList[i]->parseType == parseNode::FUNC_DEF) {
            unordered_set<Symbol *> callerSet({childNodeList[i]->symbol});
            int callerSize = -1;
            while(callerSet.size() != callerSize){
                for(Symbol *funcSymbol : callerSet){
                    for(pair<Symbol *, int> caller : funcCalledGraph[funcSymbol]){
                        if(caller.second == 0){
                            callerSet.insert(caller.first);
                        }
                    }
                }
                callerSize = callerSet.size();
            }

            if(callerSet.find(mainFuncSymbol) == callerSet.end()
               && funcOutInfluenceMap[childNodeList[i]->symbol] == false
               && callOtherFuncSet.find(childNodeList[i]->symbol) == callOtherFuncSet.end()){
                uselessFuncSet.insert(childNodeList[i]->symbol);
            }

        }
    }
}

void AstOptimizer::genDepGraphRec(parseNode *astNode, Symbol * fatherDefSymbol){
    switch(astNode->parseType){
//        case parseNode::ROOT :{
//
//            break;
//        }
        case parseNode::FUNC_DEF:{
            vector<parseNode *> childNodeList = astNode->nodes;
            for(int i = 0; i < childNodeList.size(); i++){
//                // critical variables
//                if(childNodeList[i]->parseType == parseNode::FUNC_PARAM && childNodeList[i]->symbol->dimensions.size() != 0){
//                    criticalVariableSet.insert(childNodeList[i]->symbol);
//                }
                genDepGraphRec(childNodeList[i], NULL);
            }

            break;
        }

        case parseNode::LOCAL_VAR_DEF:{
            vector<parseNode *> childNodeList = astNode->nodes;
            for(int i = 0; i < childNodeList.size(); i++){
                genDepGraphRec(childNodeList[i], NULL);
            }
            break;
        }

        case parseNode::ASSIGN_STMT:{
            vector<parseNode *> childNodeList = astNode->nodes;
            parseNode *lValNode = childNodeList[0];
            genDepGraphRec(lValNode, lValNode->symbol);
            for(int i = 1; i < childNodeList.size(); i++){
                genDepGraphRec(childNodeList[i], lValNode->symbol);
            }

            //in which layer should we generate the edges of the graph?
            break;
        }
        case parseNode::BINARY_EXP:{
            vector<parseNode *> childNodeList = astNode->nodes;
            for(int i = 0; i < childNodeList.size(); i++){
                genDepGraphRec(childNodeList[i], fatherDefSymbol);
            }
            break;
        }
        case parseNode::EXP_STMT:{
            vector<parseNode *> childNodeList = astNode->nodes;
            for(int i = 0; i < childNodeList.size(); i++){
                genDepGraphRec(childNodeList[i], fatherDefSymbol);
            }
            break;
        }
        case parseNode::FUNC_CALL:{
            vector<parseNode *> childNodeList = astNode->nodes;
            for(int i = 0; i < childNodeList.size(); i++){
                genCriticalVarRec(childNodeList[i]);
                genDepGraphRec(childNodeList[i], fatherDefSymbol);
            }
            break;
        }

//        case parseNode::INIT_VAL:{
//            dependencyGraph[fatherDefSymbol].push_back(astNode->symbol);
//            break;
//        }

        case parseNode::L_VAL:{
            vector<parseNode *> childNodeList = astNode->nodes;
            for(int i = 0; i < childNodeList.size(); i++){
                genDepGraphRec(childNodeList[i], fatherDefSymbol);
            }
            if(fatherDefSymbol != NULL){
                dependencyGraph[fatherDefSymbol].insert(astNode->symbol);
            }
            break;
        }
        case parseNode::BLOCK:{
            vector<parseNode *> childNodeList = astNode->nodes;
            for(int i = 0; i < childNodeList.size(); i++){
                genDepGraphRec(childNodeList[i], fatherDefSymbol);
            }
            break;
        }

        case parseNode::IF_STMT:{
            vector<parseNode *> childNodeList = astNode->nodes;
            for(int i = 0; i < childNodeList.size(); i++){
                genDepGraphRec(childNodeList[i], fatherDefSymbol);
            }

            //critical variables
            genCriticalVarRec(childNodeList[0]);

            break;
        }

        case parseNode::UNARY_EXP:{
            vector<parseNode *> childNodeList = astNode->nodes;
            for(int i = 0; i < childNodeList.size(); i++){
                genDepGraphRec(childNodeList[i], fatherDefSymbol);
            }
            break;
        }

        case parseNode::WHILE_STMT:{
            vector<parseNode *> childNodeList = astNode->nodes;
            for(int i = 0; i < childNodeList.size(); i++){
                genDepGraphRec(childNodeList[i], fatherDefSymbol);
            }
            //critical variables
            genCriticalVarRec(childNodeList[0]);
            break;
        }

        case parseNode::RETURN_STMT:{
            vector<parseNode *> childNodeList = astNode->nodes;
            if(childNodeList.size() != 0){
                genCriticalVarRec(childNodeList[0]);
            }
            break;
        }

        default: {
            break;
        }
    }

}

void AstOptimizer::generateDependencyGraph() {
    vector<parseNode *> childNodeList = rootNode->nodes;
    for(int i = 0; i < childNodeList.size(); i++){
        parseNode *node = childNodeList[i];
        if(node->parseType == parseNode::FUNC_DEF){
            genDepGraphRec(node, NULL);
        }
    }
}

void AstOptimizer::genCriticalVarRec(parseNode *astNode){
    if(astNode->parseType == parseNode::L_VAL){
        criticalVariableSet.insert(astNode->symbol);
    }
    vector<parseNode *> childNodeList = astNode->nodes;
    for(int i = 0; i < childNodeList.size(); i++){
        genCriticalVarRec(childNodeList[i]);
    }
}

void AstOptimizer::generateCriticalVariableSet() {
    // add into global variables
    vector<Symbol *> symbolTable = analyse->getSymbolTable();
    for(int i = 0; i < symbolTable.size(); i ++){
        if(symbolTable[i]->symbolType == Symbol::GLOBAL_VAR){
            criticalVariableSet.insert(symbolTable[i]);
        }else if(symbolTable[i]->symbolType == Symbol::PARAM && symbolTable[i]->dimensions.size() != 0){
            criticalVariableSet.insert(symbolTable[i]);
        }
    }

    // recursively add into more critical variables
    int size = criticalVariableSet.size();
    while(true){
        auto criticalVarItor = criticalVariableSet.begin();
        for(;criticalVarItor != criticalVariableSet.end();criticalVarItor++){
            auto depVarVecItor = dependencyGraph.find(*criticalVarItor);
            if(depVarVecItor != dependencyGraph.end()){
                auto depVarItor = (depVarVecItor->second).begin();
                for(;depVarItor != (depVarVecItor->second).end(); depVarItor++){
                    criticalVariableSet.insert(*depVarItor);
                }
            }
        }
        if(criticalVariableSet.size() == size){
            break;
        }
        size = criticalVariableSet.size();
    }
}

void AstOptimizer::optimizeAst() {
    generateDependencyGraph();
    generateCriticalVariableSet();
    for(auto node : rootNode->nodes){
        if(node->parseType == parseNode::FUNC_DEF){
            optimizeAstRec_1(node);
        }
    }
    generateFuncCallDelInfo();
    for(auto node : rootNode->nodes){
        if(node->parseType == parseNode::FUNC_DEF){
            optimizeAstRec_2(node, node->symbol->dataType);
        }
    }
}

void AstOptimizer::optimizeAstRec_2(parseNode *astNode, int returnType){
    vector<parseNode *>::iterator childNodeListItor = astNode->nodes.begin();
    while(childNodeListItor != astNode->nodes.end()){
        if((*childNodeListItor)->parseType == parseNode::EXP_STMT
            &&(*childNodeListItor)->nodes[0]->parseType == parseNode::FUNC_CALL){
            if(uselessFuncSet.find((*childNodeListItor)->nodes[0]->symbol) != uselessFuncSet.end()){
                delete (*childNodeListItor);
                astNode->nodes.erase(childNodeListItor);
                continue;
            }
        }else if((*childNodeListItor)->parseType == parseNode::RETURN_STMT
                 &&(*childNodeListItor)->nodes.size() != 0
                 &&(*childNodeListItor)->nodes[0]->parseType == parseNode::FUNC_CALL){
            if(uselessFuncSet.find((*childNodeListItor)->nodes[0]->symbol) != uselessFuncSet.end()){
                delete (*childNodeListItor)->nodes[0];
                if(returnType == Symbol::INT){
                    (*childNodeListItor)->nodes[0] = new parseNode(1);
                }else{
                    (*childNodeListItor)->nodes[0] = new parseNode((float)1.0);
                }

                childNodeListItor ++;
                continue;
            }
        }
        optimizeAstRec_2((*childNodeListItor), returnType);
        childNodeListItor ++;
    }
}

void AstOptimizer::optimizeAstRec_1(parseNode *astNode) {
    // function by function, delete def of none-critical variables
    vector<parseNode *>::iterator childNodeListItor = astNode->nodes.begin();
    while(childNodeListItor != astNode->nodes.end()){
        if((*childNodeListItor)->parseType == parseNode::ASSIGN_STMT){
            //rm it recursively
            if(haveFuncCallRec(*childNodeListItor)){
                childNodeListItor ++;
                continue;
            } // no func call, could change global var!!
            auto findRes = criticalVariableSet.find((*childNodeListItor)->nodes[0]->symbol);
            if(findRes == criticalVariableSet.end()){
                delete (*childNodeListItor);
                astNode->nodes.erase(childNodeListItor);
                continue;
            }else{
                childNodeListItor ++;
                continue;
            }
//        }else if((*childNodeListItor)->parseType == parseNode::LOCAL_VAR_DEF){
//            if(haveFuncCallRec(*childNodeListItor)){
//                childNodeListItor ++;
//                continue;
//            }  // no func call, could change global var!!
//            auto findRes = criticalVariableSet.find((*childNodeListItor)->symbol);
//            if(findRes == criticalVariableSet.end()){
//                delete (*childNodeListItor);
//                astNode->nodes.erase(childNodeListItor);
//                continue;
//            }else{
//                childNodeListItor ++;
//                continue;
//            }
        }else if((*childNodeListItor)->parseType == parseNode::MEMSET_ZERO){
            auto findRes = criticalVariableSet.find((*childNodeListItor)->symbol);
            if(findRes == criticalVariableSet.end()){
                delete (*childNodeListItor);
                astNode->nodes.erase(childNodeListItor);
                continue;
            }else{
                childNodeListItor ++;
                continue;
            }
        }
        optimizeAstRec_1((*childNodeListItor));
        childNodeListItor ++;
    }
}