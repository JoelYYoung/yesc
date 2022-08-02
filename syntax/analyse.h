#ifndef __ANALYSE_H__
#define __ANALYSE_H__

#include <stack>
#include <utility>
#include <vector>

#include "../lexer/lexer.h"
#include "Util.h"
using namespace std;

class Analyse {
private:
  parseNode *rootNode;
  int head;
  vector<tokenInfo> tokenInfoList;
  vector<Symbol *> symbols;
  vector<unordered_map<string, Symbol *>> symbolStack;

  void allocInitVal(vector<int>, unordered_map<int, parseNode *> &, int, parseNode *);
  void deleteInitVal(parseNode *);
  void initSymbols();
  Symbol *lastSymbol(string &);
  parseNode *parseAddExp();
  parseNode *parseAssignStmt();
  parseNode *parseBlock(Symbol *);
  vector<parseNode *> parseConstDef();
  template <typename T>
  void parseConstInitVal(vector<int>, unordered_map<int, T> &, int, parseNode *);
  parseNode *parseEqExp();
  parseNode *parseExpStmt();
  parseNode *parseFuncCall();
  parseNode *parseFuncDef();
  Symbol *parseFuncParam();
  vector<parseNode *> parseGlobalVarDef();
  parseNode *parseIfStmt(Symbol *);
  parseNode *parseInitVal();
  parseNode *parseLAndExp();
  parseNode *parseLOrExp();
  parseNode *parseLVal();
  vector<parseNode *> parseLocalVarDef();
  parseNode *parseMulExp();
  parseNode *parseRelExp();
  parseNode *parseReturnStmt(Symbol *);
  void parseRoot();
  parseNode *parseStmt(Symbol *);
  parseNode *parseUnaryExp();
  parseNode *parseWhileStmt(Symbol *);
  parseNode *parseFormat();

public:
  Analyse(vector<tokenInfo > &tokenList);
  ~Analyse();
  Util *util;
  void getNextToken(int num);
  parseNode *getparseNode();
  vector<unordered_map<string, Symbol *>> &getSymbolStack();
  vector<Symbol *> &getSymbolTable();
};

#endif