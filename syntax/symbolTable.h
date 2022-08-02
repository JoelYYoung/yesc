#include <string>
#include <unordered_map>
#include <set>
#include <utility>
#include <vector>

using namespace std;

class Symbol {
public:
  enum SymbolType { CONST, FUNC, GLOBAL_VAR, LOCAL_VAR, PARAM, PARAMLIST};
  enum DataType { FLOAT, INT, VOID, STRING, LIST};
  SymbolType symbolType;
  DataType dataType;
  string name;
  string format;
  vector<Symbol *> params; //func
  vector<int> dimensions; //array
  vector<Symbol*> paramList; //paramList
  set<int> constInt;
  set<float> constFloat;
  float fVal;
  int iVal;
  unordered_map<int, float> fMap;
  unordered_map<int, int> iMap;

  Symbol(SymbolType, DataType, const string &);
  Symbol(SymbolType, DataType, const string &, float);
  Symbol(SymbolType, DataType, const string &, int);
  Symbol(SymbolType, DataType, const string &, const vector<Symbol *> &);
  Symbol(SymbolType, DataType, const string &, const vector<int> &);
  Symbol(SymbolType, DataType, const string &, const vector<int> &, set<int> &);
  Symbol(SymbolType, DataType, const string &, const vector<int> &, set<float> &);
  Symbol(SymbolType, DataType, const string &, const vector<int> &, const unordered_map<int, float> &);
  Symbol(SymbolType, DataType, const string &, const vector<int> &, const unordered_map<int, int> &);
  ~Symbol();
  string toString();
};
