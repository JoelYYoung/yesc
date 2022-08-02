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
    std::string inputFilename("<stdin>");
    std::ostream* output = &std::cout;
    int optimizeLevel = 0;

    int s = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (std::string("-o") == argv[i])
                s = 1;
            else if (std::string("-O0") == argv[i])
                optimizeLevel = 0;
            else if (std::string("-O1") == argv[i])
                optimizeLevel = 1;
            else if (std::string("-O2") == argv[i])
                optimizeLevel = 2;
        } else {
            if (s == 1) {
                if (std::string("-") == argv[i])
                    output = &cout;
                else
                    output = new std::ofstream(argv[i], std::ofstream::out);
            } else if (s == 0) {
                if (std::string("-") == argv[i]) {
                    inputFilename = "<stdin>";
                } else {
                    inputFilename = argv[i];
                }
            }
            s = 0;
        }
    }

    vector<tokenInfo> tokenlist = lexicalAnalyze(inputFilename);

    Analyse *analyse = new Analyse(tokenlist);
    vector<Symbol *> symbolList = analyse->getSymbolTable();
//    vector<unordered_map<string, Symbol *>> symbolStack = analyse->getSymbolStack();
//    cout<<symbolList.size()<<endl;
//    cout<<symbolStack[0].size()<<endl;
//    for(Symbol * symbol : symbolList)
//    {
//        cout << symbol->toString() << " : " << symbol->iVal << endl;
//    }
    parseNode *pn = analyse->getparseNode();
//    cout << pn->toString();
    IRBuild Ir(pn, symbolList);
//    Ir->printIRs();


    Ir.printIRs();
//    Symbol * myfunc = Ir.getFuncs()[0].first;
//    vector<Symbol *> globalList = Ir.getGlobalVars();
//    vector<Symbol *> localList = Ir.getLocalVars()[myfunc];
    //cout << "localsize" << ().size() << endl;
    //cout << globalList.size() << endl;
    Assembler assembler = Assembler(Ir);
    assembler.generateAsm();
    assembler.outputAsm(*output);
    return 0;
}