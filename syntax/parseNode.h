#include <string>
#include <unordered_map>
#include <vector>

#include "symbolTable.h"

using namespace std;

class Symbol;

class parseNode {
public:
  enum parseNodeType {
    ASSIGN_STMT,
    BINARY_EXP,
    BLANK_STMT,
    BLOCK,
    BREAK_STMT,
    CONST_DEF,
    CONST_INIT_VAL,
    CONTINUE_STMT,
    EXP_STMT,
    FLOAT_LITERAL,
    FUNC_CALL,
    FUNC_DEF,
    FUNC_PARAM,
    GLOBAL_VAR_DEF,
    IF_STMT,
    INIT_VAL,
    INT_LITERAL,
    LOCAL_VAR_DEF,
    L_VAL,
    MEMSET_ZERO,
    RETURN_STMT,
    ROOT,
    FORMAT,
    UNARY_EXP,
    WHILE_STMT
  };

  enum OPType {
    ADD,
    DIV,
    EQ,
    F2I,
    GE,
    GT,
    I2F,
    LE,
    LT,
    AND,
    NOT,
    OR,
    MOD,
    MUL,
    NE,
    NEG,
    POS,
    SUB
  };

  parseNodeType parseType;
  bool isFloat;
  OPType opType;
  Symbol *symbol;
  int iVal;
  float fVal;
  string format;
  vector<parseNode *> nodes;
  string toString();
  parseNode(parseNodeType, bool, OPType, const vector<parseNode *> &);
  parseNode(parseNodeType, bool, Symbol *, const vector<parseNode *> &);
  parseNode(parseNodeType, bool, const vector<parseNode *> &);
  parseNode(float);
  parseNode(int);
  parseNode(string);
  bool nodeEq(parseNode *);
  ~parseNode();
};
