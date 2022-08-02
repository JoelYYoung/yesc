#ifndef __IR_BUILD__
#define __IR_BUILD__

#include <utility>
#include <vector>
#include "IR.h"

using namespace std;

class IRBuild {
private:
    bool isProcessed;
    parseNode *root;
    vector<Symbol *> symbols;
    vector<Symbol *> constList;
    vector<Symbol *> globalList;
    unordered_map<Symbol *, vector<Symbol *>> localList;
    vector<pair<Symbol *, vector<IR *>>> funcList;
    int varId;
    void parseRoot(parseNode *);
    vector<IR *> parseTree(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseAlgoExp(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseAssignExp(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseBinaryExp(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseBlock(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseCmpExp(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseExpStmt(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseFuncCall(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseFuncDef(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseIfStmt(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseAndExp(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseOrExp(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseLVal(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseReturnStmt(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseUnaryExp(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseWhileStmt(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseTreeWithParam(parseNode *, Symbol *, Attribute * att);
    vector<IR *> parseFormat(parseNode *, Symbol *, Attribute *att);

public:
    vector<Symbol *> getConsts();
    vector<Symbol *> getGlobalVars();
    unordered_map<Symbol *, vector<Symbol *>> getLocalVars();
    vector<pair<Symbol *, vector<IR *>>> getFuncs();
    void printIRs();

    IRBuild(parseNode *, vector<Symbol *> &);
    ~IRBuild();
};

#endif
