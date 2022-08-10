#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>

#include "analyse.h"

using namespace std;

Analyse::Analyse(vector<tokenInfo> &tokenList) {
  this->rootNode = nullptr;
  this->head = 0;
  this->tokenInfoList = tokenList;
}

Analyse::~Analyse() {
  for (Symbol *&symbol : symbols)
    delete symbol;
  if (rootNode)
    delete rootNode;
}

void Analyse::initSymbols() {
  symbolStack.resize(1);
  Symbol *func;
  Symbol *param1, *param2;
  unordered_map<string, Symbol *> mp;
  func = new Symbol(Symbol::FUNC, Symbol::INT, "getint", vector<Symbol *>());
  symbols.push_back(func);
  symbolStack.back()["getint"] = func;
  func = new Symbol(Symbol::FUNC, Symbol::INT, "getch", vector<Symbol *>());
  symbols.push_back(func);
  symbolStack.back()["getch"] = func;
  param1 = new Symbol(Symbol::PARAM, Symbol::INT, "a", vector<int>{-1});
  func = new Symbol(Symbol::FUNC, Symbol::INT, "getarray", vector<Symbol *>{param1});
  symbols.push_back(func);
  symbols.push_back(param1);
  symbolStack.back()["getarray"] = func;
  func = new Symbol(Symbol::FUNC, Symbol::FLOAT, "getfloat", vector<Symbol *>());
  symbols.push_back(func);
  symbolStack.back()["getfloat"] = func;
  param1 = new Symbol(Symbol::PARAM, Symbol::FLOAT, "a", vector<int>{-1});
  func = new Symbol(Symbol::FUNC, Symbol::INT, "getfarray", vector<Symbol *>{param1});
  symbols.push_back(func);
  symbols.push_back(param1);
  symbolStack.back()["getfarray"] = func;
  param1 = new Symbol(Symbol::PARAM, Symbol::INT, "a", vector<int>());
  func = new Symbol(Symbol::FUNC, Symbol::VOID, "putint", vector<Symbol *>{param1});
  symbols.push_back(func);
  symbols.push_back(param1);
  symbolStack.back()["putint"] = func;
  param1 = new Symbol(Symbol::PARAM, Symbol::INT, "a", vector<int>());
  func = new Symbol(Symbol::FUNC, Symbol::VOID, "putch", vector<Symbol *>{param1});
  symbols.push_back(func);
  symbols.push_back(param1);
  symbolStack.back()["putch"] = func;
  param1 = new Symbol(Symbol::PARAM, Symbol::INT, "n", vector<int>());
  param2 = new Symbol(Symbol::PARAM, Symbol::INT, "a", vector<int>{-1});
  func = new Symbol(Symbol::FUNC, Symbol::VOID, "putarray", vector<Symbol *>{param1, param2});
  symbols.push_back(func);
  symbols.push_back(param1);
  symbols.push_back(param2);
  symbolStack.back()["putarray"] = func;
  param1 = new Symbol(Symbol::PARAM, Symbol::FLOAT, "a", vector<int>());
  func = new Symbol(Symbol::FUNC, Symbol::VOID, "putfloat", vector<Symbol *>{param1});
  symbols.push_back(func);
  symbols.push_back(param1);
  symbolStack.back()["putfloat"] = func;
  param1 = new Symbol(Symbol::PARAM, Symbol::INT, "n", vector<int>());
  param2 = new Symbol(Symbol::PARAM, Symbol::FLOAT, "a", vector<int>{-1});
  func = new Symbol(Symbol::FUNC, Symbol::VOID, "putfarray",  vector<Symbol *>{param1, param2});
  symbols.push_back(func);
  symbols.push_back(param1);
  symbols.push_back(param2);
  symbolStack.back()["putfarray"] = func;
  param1 = new Symbol(Symbol::PARAM, Symbol::STRING, "format");
  param2 = new Symbol(Symbol::PARAMLIST, Symbol::LIST, "paramList", vector<Symbol *>());
  func = new Symbol(Symbol::FUNC, Symbol::VOID, "putf",  vector<Symbol *>{param1, param2});
  symbols.push_back(func);
  symbols.push_back(param1);
  symbols.push_back(param2);
  symbolStack.back()["putf"] = func;
  func = new Symbol(Symbol::FUNC, Symbol::VOID, "_sysy_starttime", vector<Symbol *>());
  symbols.push_back(func);
  symbolStack.back()["starttime"] = func;
  func = new Symbol(Symbol::FUNC, Symbol::VOID, "_sysy_stoptime", vector<Symbol *>());
  symbols.push_back(func);
  symbolStack.back()["stoptime"] = func;
}

Symbol *Analyse::lastSymbol(string &name) {
  for (int i = symbolStack.size() - 1; i >= 0; i--)
    if (symbolStack[i].find(name) != symbolStack[i].end())
      return symbolStack[i][name];
  cerr << "no such function: " << name << endl;
  exit(-1);
  return nullptr;
}

parseNode *Analyse::parseAssignStmt() {
  parseNode *lVal = parseLVal();
  getNextToken(1);
  parseNode *rVal = parseAddExp();
  getNextToken(1);
  if (!lVal->isFloat && rVal->isFloat)
    rVal = new parseNode(parseNode::UNARY_EXP, false, parseNode::F2I, {rVal});
  if (lVal->isFloat && !rVal->isFloat)
  {
    rVal = new parseNode(parseNode::UNARY_EXP, true, parseNode::I2F, {rVal});
  }
  return new parseNode(parseNode::ASSIGN_STMT, false, {lVal, rVal});
}

parseNode *Analyse::parseBlock(Symbol *func) {
  vector<parseNode *> items;
  getNextToken(1);
  while (tokenInfoList[head].getSym() != tokenType::RC) {
    switch (tokenInfoList[head].getSym()) {
    case tokenType::BREAK_TK:
    case tokenType::CONTINUE_TK:
    case tokenType::IDENT:
    case tokenType::IF_TK:
    case tokenType::LC:
    case tokenType::RETURN_TK:
    case tokenType::SEMICOLON:
    case tokenType::WHILE_TK:
      items.push_back(parseStmt(func));
      break;
    case tokenType::CONST: {
      vector<parseNode *> consts = parseConstDef();
      items.insert(items.end(), consts.begin(), consts.end());
      break;
    }
    case tokenType::INT:
    case tokenType::FLOAT: {
      vector<parseNode *> vars = parseLocalVarDef();
      items.insert(items.end(), vars.begin(), vars.end());
      break;
    }
    default:
      break;
    }
  }
  getNextToken(1);
  return new parseNode(parseNode::BLOCK, false, items);
}

vector<parseNode *> Analyse::parseConstDef() {
  getNextToken(1);
  Symbol::DataType type = tokenInfoList[head].getSym() == tokenType::INT ? Symbol::INT : Symbol::FLOAT;
  getNextToken(1);
  vector<parseNode *> consts;
  while (tokenInfoList[head].getSym() != tokenType::SEMICOLON) {
    string name = tokenInfoList[head].getName();
    getNextToken(1);
    bool isArray = false;
    vector<int> dimensions;
    if(tokenInfoList[head].getSym() == tokenType::LB)
      isArray = true;
    vector<int> arr = arrayAnalyse();
    dimensions.insert(dimensions.end(), arr.begin(), arr.end());
    getNextToken(1);
    parseNode *val = parseInitVal();
    Symbol *symbol = nullptr;
    if (isArray) {
      if (type == Symbol::INT) {
        unordered_map<int, int> iMap;
        parseConstInitVal(dimensions, iMap, 0, val);
        symbol = new Symbol(Symbol::CONST, type, name, dimensions, iMap);
      } else {
        unordered_map<int, float> fMap;
        parseConstInitVal(dimensions, fMap, 0, val);
        symbol = new Symbol(Symbol::CONST, type, name, dimensions, fMap);
      }
    } else
    {
      Symbol* symInt = new Symbol(Symbol::CONST, type, name, val->iVal);
      Symbol* symFloat = new Symbol(Symbol::CONST, type, name, val->fVal);
      if(val->parseType == parseNode::INT_LITERAL)
      {
        symbol = symInt;
      }
      else{
        symbol = symFloat;
      }
    }
    symbols.push_back(symbol);
    symbolStack.back()[name] = symbol;
    delete val;
    consts.push_back(new parseNode(parseNode::CONST_DEF, false, symbol, {}));
    if (tokenInfoList[head].getSym() == tokenType::SEMICOLON)
      break;
    getNextToken(1);
  }
  getNextToken(1);
  return consts;
}

template <typename T> void Analyse::parseConstInitVal(vector<int> array, unordered_map<int, T> &arraymap, int base, parseNode *src) {
  vector<int> index(array.size());
  vector<parseNode *> initList(src->nodes.rbegin(), src->nodes.rend());
  while (!initList.empty()) {
    if (initList.back()->parseType != parseNode::INIT_VAL) {
      int offset = 0;
      for (int i = 0; i < array.size(); i++)
        offset = offset * array[i] + index[i];
      if (initList.back()->parseType == parseNode::INT_LITERAL) {
          arraymap[base + offset] = initList.back()->iVal;
      } else {
          arraymap[base + offset] = initList.back()->fVal;
      }
      index.back()++;
    } else {
      int d = array.size() - 1;
      while (d > 0 && !index[d])
      {
        d--;
      }
      vector<int> dimension;
      for (int i = d + 1; i < array.size(); i++)
        dimension.push_back(array[i]);
      int offset = 0;
      for (int i = 0; i < array.size(); i++)
      {
        int add = 0;
        if(i<=d)
        {
          add = index[i];
        }
        offset = offset * array[i] + add;
      }
      parseConstInitVal(dimension, arraymap, base + offset, initList.back());
      index[d]++;
    }
    for (int i = array.size() - 1; i >= 0; i--) {
      if(index[i] == array[i])
      {
          index[i] = 0;
          if (i == 0)
            return;
          index[i - 1]++;
      }
    }
    initList.pop_back();
  }
}

void Analyse::allocInitVal(vector<int> array,  unordered_map<int, parseNode *> &exps, int base, parseNode *src) {
  vector<int> index(array.size());
  vector<parseNode *> initList(src->nodes.rbegin(), src->nodes.rend());
  while (!initList.empty()) {
    if (initList.back()->parseType != parseNode::INIT_VAL) {
      int offset = 0;
      for (int i = 0; i < array.size(); i++)
        offset = offset * array[i] + index[i];
      if (initList.back())
        exps[base + offset] = initList.back();
      index.back()++;
    } else {
      int d = array.size() - 1;
      while (d > 0 && !index[d])
        d--;
      vector<int> dimension;
      for (int i = d + 1; i < array.size(); i++)
        dimension.push_back(array[i]);
      int offset = 0;
      for (int i = 0; i < array.size(); i++)
      {
        int add = 0;
        if(i<=d)
        {
          add = index[i];
        }
        offset = offset * array[i] + add;
      }
      allocInitVal(dimension, exps, base + offset, initList.back());
      index[d]++;
    }
    for (int i = array.size() - 1; i >= 0; i--) {
      if(index[i] == array[i])
      {
          index[i] = 0;
          if (i == 0)
            return;
          index[i - 1]++;
      }
    }
    initList.pop_back();
  }
}

void Analyse::deleteInitVal(parseNode *root) {
  for (int i = 0; i < root->nodes.size(); i++)
    if (root->nodes[i]->parseType == parseNode::INIT_VAL)
      deleteInitVal(root->nodes[i]);
  root->nodes.clear();
  delete root;
}

parseNode *Analyse::parseAddExp() {
  vector<parseNode *> items;
  vector<parseNode::OPType> opList;
  items.push_back(parseMulExp());
  while (tokenInfoList[head].getSym() == tokenType::MINUS || tokenInfoList[head].getSym() == tokenType::PLUS) {
    opList.push_back(tokenInfoList[head].getSym() == tokenType::MINUS ? parseNode::SUB : parseNode::ADD);
    getNextToken(1);
    items.push_back(parseMulExp());
  }
  while (!opList.empty()) {
    if (util->judgeItem(items))
      break;
    Constant cons = util->deleteConst(items);
    delete items[0];
    delete items[1];
    items.erase(items.begin() + 1);
    int isInt1 = cons.isInt1;
    int isInt2 = cons.isInt2;
    int intVal1 = cons.intVal1;
    int intVal2 = cons.intVal2;
    float floatVal1 = cons.floatVal1;
    float floatVal2 = cons.floatVal2;
    items[0] = opList[0] == parseNode::ADD ? (isInt1 && isInt2 ? new parseNode(intVal1 + intVal2) : new parseNode((isInt1 ? intVal1 : floatVal1) + (isInt2 ? intVal2 : floatVal2)))
                                           : (isInt1 && isInt2 ? new parseNode(intVal1 - intVal2) : new parseNode((isInt1 ? intVal1 : floatVal1) - (isInt2 ? intVal2 : floatVal2)));
    opList.erase(opList.begin());
  }
  for (int i = 0; i < opList.size(); i++)
  {
    if (items[i + 1]->parseType == parseNode::UNARY_EXP && items[i + 1]->opType == parseNode::NEG) {
      parseNode *val = items[i + 1]->nodes[0];
      items[i + 1]->nodes[0] = nullptr;
      delete items[i + 1];
      items[i + 1] = val;
      if(opList[i] == parseNode::ADD)
      {
        opList[i] = parseNode::SUB;
      }
      else{
        opList[i] = parseNode::ADD;
      }
    }
  }
  parseNode *root = items[0];
  for (int i = 0; i < opList.size(); i++) {
      parseNode *left = root;
      parseNode *right = items[i + 1];
      if (!left->isFloat && right->isFloat)
      {
        left = new parseNode(parseNode::UNARY_EXP, true, parseNode::I2F, {left});
      }
      if (left->isFloat && !right->isFloat)
      {
        right = new parseNode(parseNode::UNARY_EXP, true, parseNode::I2F, {right});
      }
      root = new parseNode(parseNode::BINARY_EXP, left->isFloat, opList[i], {left, right});
  }
  return root;
}

parseNode *Analyse::parseMulExp() {
  vector<parseNode *> items;
  vector<parseNode::OPType> opList;
  items.push_back(parseUnaryExp());
  while (tokenInfoList[head].getSym() == tokenType::MUL || tokenInfoList[head].getSym() == tokenType::DIV || tokenInfoList[head].getSym() == tokenType::MOD) {
    switch (tokenInfoList[head].getSym()) {
    case tokenType::MUL:
      opList.push_back(parseNode::MUL);
      break;
    case tokenType::DIV:
      opList.push_back(parseNode::DIV);
      break;
    case tokenType::MOD:
      opList.push_back(parseNode::MOD);
      break;
    default:
      break;
    }
    getNextToken(1);
    items.push_back(parseUnaryExp());
  }
  while (!opList.empty()) {
    if (util->judgeItem(items))
      break;
    Constant cons = util->deleteConst(items);
    delete items[0];
    delete items[1];
    items.erase(items.begin() + 1);
    int isInt1 = cons.isInt1;
    int isInt2 = cons.isInt2;
    int intVal1 = cons.intVal1;
    int intVal2 = cons.intVal2;
    float floatVal1 = cons.floatVal1;
    float floatVal2 = cons.floatVal2;
    switch (opList[0]) {
    case parseNode::MUL:
      items[0] = isInt1 && isInt2 ? new parseNode(intVal1 * intVal2) : new parseNode((isInt1 ? intVal1 : floatVal1) * (isInt2 ? intVal2 : floatVal2));
      break;
    case parseNode::DIV:
      items[0] = isInt1 && isInt2 ? new parseNode(intVal1 / intVal2) : new parseNode((isInt1 ? intVal1 : floatVal1) / (isInt2 ? intVal2 : floatVal2));
      break;
    case parseNode::MOD:
      items[0] = new parseNode(intVal1 % intVal2);
      break;
    default:
      break;
    }
    opList.erase(opList.begin());
  }
  if(items.size()==2 && opList[0] == parseNode::MUL)
  {
    if (items[0]->parseType == parseNode::INT_LITERAL && items[1]->parseType == parseNode::L_VAL)
    {
      if(items[0]->iVal == 2)
      {
        delete items[0];
        items[0] = items[1];
        opList[0] = parseNode::ADD;
      }
    }
    else if(items[1]->parseType == parseNode::INT_LITERAL && items[0]->parseType == parseNode::L_VAL)
    {
      if(items[1]->iVal == 2)
      {
        delete items[1];
        items[1] = items[0];
        opList[0] = parseNode::ADD;
      }
    }
  }
  parseNode *root = items[0];
  for (int i = 0; i < opList.size(); i++) {
    parseNode *left = root;
    parseNode *right = items[i + 1];
    if (!left->isFloat && right->isFloat)
    {
      left = new parseNode(parseNode::UNARY_EXP, true, parseNode::I2F, {left});
    }
    if (left->isFloat && !right->isFloat)
      right = new parseNode(parseNode::UNARY_EXP, true, parseNode::I2F, {right});
    root = new parseNode(parseNode::BINARY_EXP, left->isFloat, opList[i], {left, right});
  }
  return root;
}

parseNode *Analyse::parseRelExp() {
  vector<parseNode *> items;
  vector<parseNode::OPType> opList;
  items.push_back(parseAddExp());
  while (tokenInfoList[head].getSym() == tokenType::GE || tokenInfoList[head].getSym() == tokenType::GT ||
         tokenInfoList[head].getSym() == tokenType::LE || tokenInfoList[head].getSym() == tokenType::LT) {
    switch (tokenInfoList[head].getSym()) {
    case tokenType::GE:
      opList.push_back(parseNode::GE);
      break;
    case tokenType::GT:
      opList.push_back(parseNode::GT);
      break;
    case tokenType::LE:
      opList.push_back(parseNode::LE);
      break;
    case tokenType::LT:
      opList.push_back(parseNode::LT);
      break;
    default:
      break;
    }
    getNextToken(1);
    items.push_back(parseAddExp());
  }
  while (!opList.empty()) {
    if (util->judgeItem(items))
      break;
    Constant cons = util->deleteConst(items);
    delete items[0];
    delete items[1];
    items.erase(items.begin() + 1);
    switch (opList[0]) {
    case parseNode::LE:
      items[0] = new parseNode((cons.isInt1 ? cons.intVal1 : cons.floatVal1) <= (cons.isInt2 ? cons.intVal2 : cons.floatVal2));
      break;
    case parseNode::LT:
      items[0] = new parseNode((cons.isInt1 ? cons.intVal1 : cons.floatVal1) < (cons.isInt2 ? cons.intVal2 : cons.floatVal2));
      break;
    case parseNode::GE:
      items[0] = new parseNode((cons.isInt1 ? cons.intVal1 : cons.floatVal1) >= (cons.isInt2 ? cons.intVal2 : cons.floatVal2));
      break;
    case parseNode::GT:
      items[0] = new parseNode((cons.isInt1 ? cons.intVal1 : cons.floatVal1) > (cons.isInt2 ? cons.intVal2 : cons.floatVal2));
      break;
    default:
      break;
    }
    opList.erase(opList.begin());
  }
  parseNode *root = items[0];
  for (int i = 0; i < opList.size(); i++) {
    parseNode *left = root;
    parseNode *right = items[i + 1];
    if (!left->isFloat && right->isFloat)
      left = new parseNode(parseNode::UNARY_EXP, true, parseNode::I2F, {left});
    if (left->isFloat && !right->isFloat)
      right = new parseNode(parseNode::UNARY_EXP, true, parseNode::I2F, {right});
    root = new parseNode(parseNode::BINARY_EXP, false, opList[i], {left, right});
  }
  return root;
}

parseNode *Analyse::parseEqExp() {
  vector<parseNode *> items;
  vector<parseNode::OPType> opList;
  items.push_back(parseRelExp());
  while (tokenInfoList[head].getSym() == tokenType::EQ || tokenInfoList[head].getSym() == tokenType::NE) {
    opList.push_back(tokenInfoList[head].getSym() == tokenType::EQ ? parseNode::EQ : parseNode::NE);
    getNextToken(1);
    items.push_back(parseRelExp());
  }
  while (!opList.empty()) {
    if (util->judgeItem(items))
      break;
    Constant cons = util->deleteConst(items);
    delete items[0];
    delete items[1];
    items.erase(items.begin() + 1);
    if(opList[0] == parseNode::EQ)
    {
      if(cons.isInt1&&cons.isInt2)
      {
        items[0] = new parseNode(cons.intVal1 == cons.intVal2);
      }
      else{
        items[0] = new parseNode((cons.isInt1 ? cons.intVal1 : cons.floatVal1) == (cons.isInt2 ? cons.intVal2 : cons.floatVal2));
      }
    }
    else if(opList[0] == parseNode::NE)
    {
      if(cons.isInt1&&cons.isInt2)
      {
        items[0] = new parseNode(cons.intVal1 != cons.intVal2);
      }
      else{
        items[0] = new parseNode((cons.isInt1 ? cons.intVal1 : cons.floatVal1) != (cons.isInt2 ? cons.intVal2 : cons.floatVal2));
      }
    }
    opList.erase(opList.begin());
  }
  parseNode *root = items[0];
  for (int i = 0; i < opList.size(); i++) {
    parseNode *left = root;
    parseNode *right = items[i + 1];
    if (!left->isFloat && right->isFloat)
      left = new parseNode(parseNode::UNARY_EXP, true, parseNode::I2F, {left});
    if (left->isFloat && !right->isFloat)
      right = new parseNode(parseNode::UNARY_EXP, true, parseNode::I2F, {right});
    root = new parseNode(parseNode::BINARY_EXP, false, opList[i], {left, right});
  }
  return root;
}

parseNode *Analyse::parseExpStmt() {
  parseNode *exp = parseAddExp();
  getNextToken(1);
  return new parseNode(parseNode::EXP_STMT, false, {exp});
}

parseNode *Analyse::parseFuncCall() {
  Symbol *symbol = lastSymbol(tokenInfoList[head].name);
  getNextToken(2);
  vector<parseNode *> params;
  if(symbol->name == "putf")
  {
    parseNode *param1 = parseFormat();
    params.push_back(param1);
    getNextToken(2);
    while(tokenInfoList[head].getSym() != tokenType::RP) {
      parseNode *param = parseAddExp();
      params.push_back(param);
      if (tokenInfoList[head].getSym() == tokenType::RP)
        break;
      getNextToken(1);
    }
    getNextToken(1);
  }
  else{
    while (tokenInfoList[head].getSym() != tokenType::RP)
    {
      parseNode *param = parseAddExp();
      param = util->typeTrans(symbol->params[params.size()]->dataType, param);
      params.push_back(param);
      if (tokenInfoList[head].getSym() == tokenType::RP)
        break;
      getNextToken(1);
    }
    getNextToken(1);
  }
  return new parseNode(parseNode::FUNC_CALL, symbol->dataType == Symbol::FLOAT, symbol, {params});
}

parseNode *Analyse::parseFormat() {
  return new parseNode(tokenInfoList[head].getName());
}

Symbol *Analyse::parseFuncParam() {
  Symbol::DataType type =
      tokenInfoList[head].getSym() == tokenType::INT ? Symbol::INT : Symbol::FLOAT;
  getNextToken(1);
  string name = tokenInfoList[head].getName();
  getNextToken(1);
  if (tokenInfoList[head].getSym() != tokenType::LB)
    return new Symbol(Symbol::PARAM, type, name);
  vector<int> dimensions;
  dimensions.push_back(-1);
  getNextToken(2);
  vector<int> arr = arrayAnalyse();
  dimensions.insert(dimensions.end(), arr.begin(), arr.end());
  return new Symbol(Symbol::PARAM, type, name, dimensions);
}

parseNode *Analyse::parseFuncDef() {
  Symbol::DataType type = Symbol::INT;
  switch (tokenInfoList[head].getSym()) {
  case tokenType::FLOAT:
    type = Symbol::FLOAT;
    break;
  case tokenType::INT:
    type = Symbol::INT;
    break;
  case tokenType::VOID:
    type = Symbol::VOID;
    break;
  default:
    break;
  }
  getNextToken(1);
  string name = tokenInfoList[head].getName();
  getNextToken(2);
  vector<Symbol *> params;
  while (tokenInfoList[head].getSym() != tokenType::RP) {
    params.push_back(parseFuncParam());
    if (tokenInfoList[head].getSym() == tokenType::RP)
      break;
    getNextToken(1);
  }
  Symbol *symbol = new Symbol(Symbol::FUNC, type, name, params);
  symbols.push_back(symbol);
  symbolStack.back()[name] = symbol;
  getNextToken(1);
  symbolStack.push_back({});
  symbols.insert(symbols.end(), params.begin(), params.end());
  for (Symbol *param : params)
    symbolStack.back()[param->name] = param;
  parseNode *body = parseBlock(symbol);
  symbolStack.pop_back();
  return new parseNode(parseNode::FUNC_DEF, false, symbol, {body});
}

vector<parseNode *> Analyse::parseGlobalVarDef() {
  Symbol::DataType type = tokenInfoList[head].getSym() == tokenType::INT ? Symbol::INT : Symbol::FLOAT;
  getNextToken(1);
  vector<parseNode *> consts;
  while (tokenInfoList[head].getSym() != tokenType::SEMICOLON) {
    string name = tokenInfoList[head].getName();
    getNextToken(1);
    vector<int> dimensions;
    vector<int> arr = arrayAnalyse();
    dimensions.insert(dimensions.end(), arr.begin(), arr.end());
    Symbol *symbol = nullptr;
    if (tokenInfoList[head].getSym() == tokenType::ASSIGN) {
      getNextToken(1);
      parseNode *val = parseInitVal();
      if (dimensions.empty())
        symbol = val->parseType == parseNode::INT_LITERAL
                     ? new Symbol(Symbol::GLOBAL_VAR, type, name, val->iVal)
                     : new Symbol(Symbol::GLOBAL_VAR, type, name, val->fVal);
      else if (type == Symbol::INT) {
        unordered_map<int, int> iMap;
        parseConstInitVal(dimensions, iMap, 0, val);
        symbol = new Symbol(Symbol::GLOBAL_VAR, type, name, dimensions, iMap);
      } else {
        unordered_map<int, float> fMap;
        parseConstInitVal(dimensions, fMap, 0, val);
        symbol = new Symbol(Symbol::GLOBAL_VAR, type, name, dimensions, fMap);
      }
      delete val;
    } else {
      if (dimensions.empty())
        symbol = new Symbol(Symbol::GLOBAL_VAR, type, name, 0);
      else if (type == Symbol::INT)
        symbol = new Symbol(Symbol::GLOBAL_VAR, type, name, dimensions);
      else
        symbol = new Symbol(Symbol::GLOBAL_VAR, type, name, dimensions);
    }
    symbols.push_back(symbol);
    symbolStack.back()[name] = symbol;
    consts.push_back(new parseNode(parseNode::GLOBAL_VAR_DEF, false, symbol, {}));
    if (tokenInfoList[head].getSym() == tokenType::SEMICOLON)
      break;
    getNextToken(1);
  }
  getNextToken(1);
  return consts;
}

parseNode *Analyse::parseIfStmt(Symbol *func) {
  getNextToken(2);
  parseNode *cond = parseLOrExp();
  getNextToken(1);
  bool flag = tokenInfoList[head].getSym() != tokenType::LC;
  if (flag)
    symbolStack.push_back({});
  parseNode *stmt1 = parseStmt(func);
  if (flag)
    symbolStack.pop_back();
  parseNode *stmt2 = nullptr;
  if (tokenInfoList[head].getSym() == tokenType::ELSE_TK) {
    flag = tokenInfoList[head].getSym() != tokenType::LC;
    getNextToken(1);
    if (flag)
      symbolStack.push_back({});
    stmt2 = parseStmt(func);
    if (flag)
      symbolStack.pop_back();
    return new parseNode(parseNode::IF_STMT, false, {cond, stmt1, stmt2});
  }
  return new parseNode(parseNode::IF_STMT, false, {cond, stmt1});
}

parseNode *Analyse::parseInitVal() {
  if (tokenInfoList[head].getSym() != tokenType::LC)
  {
    return parseAddExp();
  }
  getNextToken(1);
  vector<parseNode *> items;
  while (tokenInfoList[head].getSym() != tokenType::RC) {
    items.push_back(parseInitVal());
    if (tokenInfoList[head].getSym() == tokenType::RC)
      break;
    getNextToken(1);
  }
  getNextToken(1);
  return new parseNode(parseNode::INIT_VAL, false, items);
}

parseNode *Analyse::parseLAndExp() {
  vector<parseNode *> items;
  items.push_back(parseEqExp());
  while (tokenInfoList[head].getSym() == tokenType::AND) {
    getNextToken(1);
    items.push_back(parseEqExp());
  }
  while (items.size() > 1) {
    if (util->judgeItem(items))
      break;
    Constant cons = util->deleteConst(items);
    delete items[0];
    delete items[1];
    items.erase(items.begin() + 1);
    items[0] = new parseNode((cons.isInt1 ? cons.intVal1 : cons.floatVal1) && (cons.isInt2 ? cons.intVal2 : cons.floatVal2));
  }
  parseNode *root = items[0];
  for (int i = 1; i < items.size(); i++)
    root = new parseNode(parseNode::BINARY_EXP, false, parseNode::AND, {root, items[i]});
  return root;
}

parseNode *Analyse::parseLOrExp() {
  vector<parseNode *> items;
  items.push_back(parseLAndExp());
  while (tokenInfoList[head].getSym() == tokenType::OR) {
    getNextToken(1);
    items.push_back(parseLAndExp());
  }
  while (items.size() > 1) {
    if (util->judgeItem(items))
      break;
    Constant cons = util->deleteConst(items);
    delete items[0];
    delete items[1];
    items.erase(items.begin() + 1);
    items[0] = new parseNode((cons.isInt1 ? cons.intVal1 : cons.floatVal1) || (cons.isInt2 ? cons.intVal2 : cons.floatVal2));
  }
  parseNode *root = items[0];
  for (int i = 1; i < items.size(); i++)
    root = new parseNode(parseNode::BINARY_EXP, false, parseNode::OR, {root, items[i]});
  return root;
}

parseNode *Analyse::parseLVal() {
  vector<int> array;
  int num = 0;
  int offset = 0;
  string name = tokenInfoList[head].getName();
  Symbol *symbol = lastSymbol(name);
  getNextToken(1);
  vector<parseNode *> nodeList;
  int arrarSize = 0;
  while (tokenInfoList[head].getSym() == tokenType::LB) {
    getNextToken(1);
    nodeList.push_back(parseAddExp());
    if (nodeList.back()->parseType == parseNode::INT_LITERAL)
    {
      offset *= symbol->dimensions[arrarSize];
      offset += nodeList.back()->iVal;
      num++;
    }
    arrarSize++;
    getNextToken(1);
  }
  if (symbol->symbolType == Symbol::CONST && num == symbol->dimensions.size()) {
    //cout << offset << endl;
    for (parseNode *node : nodeList)
      delete node;
    if (symbol->dimensions.empty())
      return symbol->dataType == Symbol::INT ? new parseNode(symbol->iVal) : new parseNode(symbol->fVal);
    if (symbol->dataType == Symbol::INT)
      return new parseNode(symbol->iMap.find(offset) == symbol->iMap.end() ? 0 : symbol->iMap[offset]);
    else
      return new parseNode(symbol->fMap.find(offset) == symbol->fMap.end() ? 0 : symbol->fMap[offset]);
  }
  return new parseNode(parseNode::L_VAL, symbol->dataType == Symbol::FLOAT, symbol, nodeList);
}

parseNode *Analyse::parseReturnStmt(Symbol *func) {
  getNextToken(1);
  if (tokenInfoList[head].getSym() == tokenType::SEMICOLON) {
    getNextToken(1);
    return new parseNode(parseNode::RETURN_STMT, false, {});
  }
  parseNode *val = parseAddExp();
  getNextToken(1);
  val = util->typeTrans(func->dataType, val);
  return new parseNode(parseNode::RETURN_STMT, false, {val});
}

void Analyse::parseRoot() {
  initSymbols();
  vector<parseNode *> items;
  while (head < tokenInfoList.size()) {
    switch (tokenInfoList[head].getSym()) {
    case tokenType::CONST: {
      vector<parseNode *> consts = parseConstDef();
      items.insert(items.end(), consts.begin(), consts.end());
      break;
    }
    case tokenType::FLOAT:
    case tokenType::INT:
      switch (tokenInfoList[head + 2].getSym()) {
      case tokenType::ASSIGN:
      case tokenType::COMMA:
      case tokenType::LB:
      case tokenType::SEMICOLON: {
        vector<parseNode *> vars = parseGlobalVarDef();
        items.insert(items.end(), vars.begin(), vars.end());
        break;
      }
      case tokenType::LP:
        items.push_back(parseFuncDef());
        break;
      default:
        break;
      }
      break;
    case tokenType::VOID:
      items.push_back(parseFuncDef());
      break;
    default:
      break;
    }
  }
  rootNode = new parseNode(parseNode::ROOT, false, items);
}

parseNode *Analyse::parseStmt(Symbol *func) {
  switch (tokenInfoList[head].getSym()) {
  case tokenType::BREAK_TK:
    getNextToken(2);
    return new parseNode(parseNode::BREAK_STMT, false, {});
  case tokenType::CONTINUE_TK:
    getNextToken(2);
    return new parseNode(parseNode::CONTINUE_STMT, false, {});
  case tokenType::IDENT:
    switch (tokenInfoList[head + 1].getSym()) {
    case tokenType::ASSIGN:
    case tokenType::LB:
      return parseAssignStmt();
    case tokenType::DIV:
    case tokenType::LP:
    case tokenType::MINUS:
    case tokenType::MOD:
    case tokenType::MUL:
    case tokenType::PLUS:
      return parseExpStmt();
    default:
      break;
    }
  case tokenType::IF_TK:
    return parseIfStmt(func);
  case tokenType::LC: {
    symbolStack.push_back({});
    parseNode *root = parseBlock(func);
    symbolStack.pop_back();
    return root;
  }
  case tokenType::LP:
  case tokenType::PLUS:
  case tokenType::MINUS:
  case tokenType::NOT:
    return parseExpStmt();
  case tokenType::RETURN_TK:
    return parseReturnStmt(func);
  case tokenType::SEMICOLON:
    getNextToken(1);
    return new parseNode(parseNode::BLANK_STMT, false, {});
  case tokenType::WHILE_TK:
    return parseWhileStmt(func);
  default:
    break;
  }
  return nullptr;
}

parseNode *Analyse::parseUnaryExp() {
  switch (tokenInfoList[head].getSym()) {
  case tokenType::INTCONST:{
    int iVal = tokenInfoList[head].getValue();
    getNextToken(1);
    return new parseNode(iVal);
  } 
  case tokenType::FLOATCONST:
  {
    float fVal = tokenInfoList[head].fvalue;
    getNextToken(1);
    return new parseNode(fVal);
  }
  case tokenType::IDENT:
    if (tokenInfoList[head + 1].getSym() == tokenType::LP)
      return parseFuncCall();
    return parseLVal();
  case tokenType::LP: {
    getNextToken(1);
    parseNode *root = parseAddExp();
    getNextToken(1);
    return root;
  }
  case tokenType::NOT: {
    getNextToken(1);
    parseNode *val = parseUnaryExp();
    switch (val->parseType) {
    case parseNode::FLOAT_LITERAL: {
      float fVal = val->fVal;
      delete val;
      return new parseNode(!fVal);
    }
    case parseNode::INT_LITERAL: {
      int iVal = val->iVal;
      delete val;
      return new parseNode(!iVal);
    }
    case parseNode::UNARY_EXP:
      switch (val->opType) {
      case parseNode::NOT: {
        parseNode *ret = val->nodes[0];
        val->nodes.clear();
        delete val;
        return ret;
      }
      case parseNode::NEG:
        return val;
      default:
        break;
      }
      break;
    default:
      break;
    }
    return new parseNode(parseNode::UNARY_EXP, false, parseNode::NOT, {val});
  }
  case tokenType::MINUS: {
    getNextToken(1);
    parseNode *val = parseUnaryExp();
    switch (val->parseType) {
    case parseNode::FLOAT_LITERAL:
      val->fVal = -val->fVal;
      return val;
    case parseNode::INT_LITERAL:
      val->iVal = -val->iVal;
      return val;
    case parseNode::UNARY_EXP:
      if (val->opType == parseNode::NEG) {
        parseNode *ret = val->nodes[0];
        val->nodes.clear();
        delete val;
        return ret;
      }
      break;
    default:
      break;
    }
    return new parseNode(parseNode::UNARY_EXP, val->isFloat, parseNode::NEG, {val});
  }
  case tokenType::PLUS: {
    getNextToken(1);
    return parseUnaryExp();
  }
  default:
    break;
  }
  return nullptr;
}

vector<parseNode *> Analyse::parseLocalVarDef() {
  vector<parseNode *> items;
  Symbol::DataType type = tokenInfoList[head].getSym() == tokenType::INT ? Symbol::INT : Symbol::FLOAT;
  getNextToken(1);
  while (tokenInfoList[head].getSym() != tokenType::SEMICOLON) {
    string name = tokenInfoList[head].getName();
    getNextToken(1);
    vector<int> dimensions;
    vector<int> arr = arrayAnalyse();
    dimensions.insert(dimensions.end(), arr.begin(), arr.end());
    Symbol *symbol = new Symbol(Symbol::LOCAL_VAR, type, name, dimensions);
    items.push_back(new parseNode(parseNode::LOCAL_VAR_DEF, false, symbol, {}));
    if (tokenInfoList[head].getSym() == tokenType::ASSIGN) {
      getNextToken(1);
      parseNode *val = parseInitVal();
      if (dimensions.empty()) {
        val = util->typeTrans(type, val);
        items.push_back(new parseNode(parseNode::ASSIGN_STMT, false, {new parseNode(parseNode::L_VAL, type == Symbol::FLOAT, symbol, {}), val}));
      } else {
        items.push_back(new parseNode(parseNode::MEMSET_ZERO, false, symbol, {}));
        unordered_map<int, parseNode *> exps;
        allocInitVal(dimensions, exps, 0, val);
        vector<pair<int, parseNode *>> arrayList(exps.begin(), exps.end());
        sort(arrayList.begin(), arrayList.end());
        for (pair<int, parseNode *> exp : arrayList) {
          int size = exp.first;
          vector<parseNode *> nodes;
          for (int j = dimensions.size() - 1; j >= 0;j--)
          {
            nodes.push_back(new parseNode(size % dimensions[j]));
            size /= dimensions[j];
          }
          reverse(nodes.begin(), nodes.end());
          parseNode *expVal = exp.second;
          expVal = util->typeTrans(type, expVal);
          items.push_back(new parseNode(parseNode::ASSIGN_STMT, false, {new parseNode(parseNode::L_VAL, type == Symbol::FLOAT, symbol, nodes), expVal}));
        }
        deleteInitVal(val);
      }
    }
    symbols.push_back(symbol);
    symbolStack.back()[name] = symbol;
    if (tokenInfoList[head].getSym() == tokenType::SEMICOLON)
      break;
    getNextToken(1);
  }
  getNextToken(1);
  return items;
}

parseNode *Analyse::parseWhileStmt(Symbol *func) {
  getNextToken(2);
  parseNode *cond = parseLOrExp();
  getNextToken(1);
  bool flag = tokenInfoList[head].getSym() != tokenType::LC;
  parseNode *stmt;
  if (flag)
  {
    symbolStack.push_back({});
    stmt = parseStmt(func);
    symbolStack.pop_back();
  }
  else  stmt = parseStmt(func);
  return new parseNode(parseNode::WHILE_STMT, false, {cond, stmt});
}

vector<int> Analyse::arrayAnalyse() {
  vector<int> dimension;
  while (tokenInfoList[head].getSym() == tokenType::LB) {
    getNextToken(1);
    parseNode *node = parseAddExp();
    dimension.push_back(node->iVal);
    delete node;
    getNextToken(1);
  }
  return dimension;
}

void Analyse::getNextToken(int num){
  head += num;
  return;
}

parseNode *Analyse::getparseNode() {
  parseRoot();
  return rootNode;
};

vector<Symbol *> &Analyse::getSymbolTable() {
  return symbols;
}

vector<unordered_map<string, Symbol *>> &Analyse::getSymbolStack() {
  return symbolStack;
}