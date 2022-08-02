#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <vector>
#include "Ir/IRBuild.h"
using namespace std;

int main(int argc, char **argv)
{
    //vector<tokenInfo> tokenlist = lexicalAnalyze("/home/zyy/Whitee/test/function_test2021/117_nested_loops.sy");
    vector<tokenInfo> tokenlist = lexicalAnalyze("./test.sy");
    for(tokenInfo tk : tokenlist)
    {
        cout<<tk.getName()<<':'<<tk.getSym()<<endl;
    }
    Analyse *analyse = new Analyse(tokenlist);
    parseNode *pn = analyse->getparseNode();
    vector<Symbol *> symbolList = analyse->getSymbolTable(); 
    vector<unordered_map<string, Symbol *>> symbolStack = analyse->getSymbolStack();
    cout<<symbolList.size()<<endl;
    cout<<symbolStack[0].size()<<endl;
    for(Symbol * symbol : symbolList)
    {
        cout << symbol->toString() << " : " << symbol->iVal << endl;
    }
    cout << pn->toString();
    IRBuild *Ir = new IRBuild(pn, symbolList);
    Ir->printIRs();
    unordered_map<Symbol*,vector<Symbol*>>  mp = Ir->getLocalVars();
    vector<pair<Symbol *, vector<IR *>>> funclist = Ir->getFuncs();
    return 0;
}