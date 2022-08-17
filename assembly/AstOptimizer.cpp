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
    for(auto node : rootNode->nodes){
        if(node->parseType == parseNode::FUNC_DEF){
            optimizeAstRec(node);
        }
    }
}

void AstOptimizer::optimizeAstRec(parseNode *astNode) {
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
        optimizeAstRec((*childNodeListItor));
        childNodeListItor ++;
    }
}