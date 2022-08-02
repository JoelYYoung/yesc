#include <string>
#include <unordered_map>

#include "symbolTable.h"

using namespace std;

unordered_map<Symbol::SymbolType, string> symbolTypeStr = {
    {Symbol::CONST, "CONST"},
    {Symbol::FUNC, "FUNC"},
    {Symbol::GLOBAL_VAR, "GLOBAL_VAR"},
    {Symbol::LOCAL_VAR, "LOCAL_VAR"},
    {Symbol::PARAM, "PARAM"},
    {Symbol::PARAMLIST, "PARAMLIST"}};

unordered_map<Symbol::DataType, string> dataTypeStr = {
    {Symbol::FLOAT, "FLOAT"}, {Symbol::INT, "INT"}, {Symbol::VOID, "VOID"}, {Symbol::STRING, "STRING"}, {Symbol::LIST, "LIST"}};

Symbol::Symbol(SymbolType symbolType, DataType dataType, const string &name) {
  this->symbolType = symbolType;
  this->dataType = dataType;
  this->name = name;
}

Symbol::Symbol(SymbolType symbolType, DataType dataType, const string &name,
               float fVal) {
  this->symbolType = symbolType;
  this->dataType = dataType;
  this->name = name;
  if (dataType == INT)
    this->iVal = fVal;
  else
    this->fVal = fVal;
}

Symbol::Symbol(SymbolType symbolType, DataType dataType, const string &name, int iVal) {
  this->symbolType = symbolType;
  this->dataType = dataType;
  this->name = name;
  if (dataType == INT)
    this->iVal = iVal;
  else
    this->fVal = iVal;
}

Symbol::Symbol(SymbolType symbolType, DataType dataType, const string &name, const vector<Symbol *> &params) {
  if(symbolType == SymbolType::PARAMLIST)
  {
    this->symbolType = symbolType;
    this->dataType = dataType;
    this->name = name;
    this->paramList = params;
  }
  else{
    this->symbolType = symbolType;
    this->dataType = dataType;
    this->name = name;
    this->params = params;
  }
}

Symbol::Symbol(SymbolType symbolType, DataType dataType, const string &name, const vector<int> &dimensions) {
  this->symbolType = symbolType;
  this->dataType = dataType;
  this->name = name;
  this->dimensions = dimensions;
}

Symbol::Symbol(SymbolType symbolType, DataType dataType, const string &name, const vector<int> &dimensions, set<int> &constInt)
{
  this->symbolType = symbolType;
  this->dataType = dataType;
  this->dimensions = dimensions;
  this->constInt = constInt;
}

Symbol::Symbol(SymbolType symbolType, DataType dataType, const string &name, const vector<int> &dimensions, set<float> &constFloat)
{
  this->symbolType = symbolType;
  this->dataType = dataType;
  this->dimensions = dimensions;
  this->constFloat = constFloat;
}

Symbol::Symbol(SymbolType symbolType, DataType dataType, const string &name,
               const vector<int> &dimensions,
               const unordered_map<int, float> &fMap) {
  this->symbolType = symbolType;
  this->dataType = dataType;
  this->name = name;
  this->dimensions = dimensions;
  this->fMap = fMap;
}

Symbol::Symbol(SymbolType symbolType, DataType dataType, const string &name,
               const vector<int> &dimensions,
               const unordered_map<int, int> &iMap) {
  this->symbolType = symbolType;
  this->dataType = dataType;
  this->name = name;
  this->dimensions = dimensions;
  this->iMap = iMap;
}

Symbol::~Symbol() {}

string Symbol::toString() {
  bool first = true;
  string s = "(" + symbolTypeStr[symbolType] + ", " + name + ", ";
  switch (symbolType) {
  case CONST:
  case GLOBAL_VAR:
  case LOCAL_VAR:
    s += dataTypeStr[dataType];
    for (int dimension : dimensions)
      s += "[" + to_string(dimension) + "]";
    break;
  case FUNC:
    s += dataTypeStr[dataType] + "(";
    for (Symbol *param : params) {
      if (!first)
        s += ", ";
      s += dataTypeStr[param->dataType];
      for (int dimension : param->dimensions)
        s += "[" + (dimension == -1 ? "" : to_string(dimension)) + "]";
      first = false;
    }
    s += ")";
    break;
  case PARAM:
    s += dataTypeStr[dataType];
    for (int dimension : dimensions)
      s += "[" + (dimension == -1 ? "" : to_string(dimension)) + "]";
    break;
  default:
    break;
  }
  s += ")";
  return s;
}
