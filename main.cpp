#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <vector>
#include "Ir/IRBuild.h"
#include "assembly/assembler.h"
using namespace std;

std::string inputFilename("<stdin>");
std::ostream* output = &std::cout;
int optimizeLevel = 0;

void parseArgv(int argc, char **argv){
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
}

int main(int argc, char **argv)
{
    //vector<tokenInfo> tokenlist = lexicalAnalyze("/home/joelyang/Documents/bisheng/MyHust_V3/test.sy");

    parseArgv(argc, argv);
    vector<tokenInfo> tokenlist = lexicalAnalyze(inputFilename);

    Analyse *analyse = new Analyse(tokenlist);

    vector<Symbol *> symbolList = analyse->getSymbolTable();
    parseNode *pn = analyse->getparseNode();
    IRBuild Ir(pn, symbolList);
    if(optimizeLevel == 2)
    {
        Ir.commonExpression();
        Ir.deadExpDelete();
        //Ir.loadDelete();
    }
    Ir.loadDelete();
    //Ir.printIRs(true);
    Assembler assembler = Assembler(Ir);
    assembler.generateAsm();
    assembler.outputAsm(*output);
    return 0;
}
