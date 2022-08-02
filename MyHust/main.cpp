#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <vector>
#include "Ir/IRBuild.h"
#include "assembly/assembler.h"
using namespace std;

int main(int argc, char **argv)
{
    //vector<tokenInfo> tokenlist = lexicalAnalyze("/home/zyy/Whitee/test/function_test2020/83_enc_dec.sy");
    vector<tokenInfo> tokenlist = lexicalAnalyze("/home/joelyang/Documents/bisheng/MyHust_V3/test.sy");
    for(tokenInfo tk : tokenlist)
    {
        //cout<<tk.getName()<<':'<<tk.getSym()<<endl;
    }
    Analyse *analyse = new Analyse(tokenlist);
    vector<Symbol *> symbolList = analyse->getSymbolTable(); 
    vector<unordered_map<string, Symbol *>> symbolStack = analyse->getSymbolStack();
    //cout<<symbolList.size()<<endl;
    //cout<<symbolStack[0].size()<<endl;
//    for(Symbol * symbol : symbolList)
//    {
//        cout << symbol->toString() << " : " << symbol->iVal << endl;
//    }
    parseNode *pn = analyse->getparseNode();
    //cout << pn->toString();
    IRBuild Ir(pn, symbolList);
    //Ir->printIRs();


    Ir.printIRs();
    Symbol * myfunc = Ir.getFuncs()[0].first;
    vector<Symbol *> globalList = Ir.getGlobalVars();
    vector<Symbol *> localList = Ir.getLocalVars()[myfunc];
    //cout << "localsize" << ().size() << endl;
    //cout << globalList.size() << endl;
    Assembler assembler = Assembler(Ir);
    assembler.generateAsm();
    assembler.outputAsm(cout);
    return 0;
}