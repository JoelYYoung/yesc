#include <iostream>

#include "parseNode.h"

using namespace std;
string parseNode::toString(){
  string str;
  switch (parseType)
  {
  case ASSIGN_STMT:
    str += "ASSIGN_STMT";
    break;
  case BINARY_EXP:
    str += "BINARY_EXP";
    break;
  case BLANK_STMT:
    str += "BLANK_STMT";
    break;
  case BLOCK:
    str += "BLOCK";
    break;
  case BREAK_STMT:
    str += "BREAK_STMT";
    break;
  case CONST_DEF:
    str += "CONST_DEF";
    break;
  case CONST_INIT_VAL:
    str += "CONST_INIT_VAL";
    break;
  case CONTINUE_STMT:
    str += "CONTINUE_STMT";
    break;
  case EXP_STMT:
    str += "EXP_STMT";
    break;
  case FLOAT_LITERAL:
    str += "FLOAT_LITERAL";
    break;
  case FUNC_CALL:
    str += "FUNC_CALL";
    break;
  case FUNC_DEF:
    str += "FUNC_DEF";
    break;
  case FUNC_PARAM:
    str += "FUNC_PARAM";
    break;
  case GLOBAL_VAR_DEF:
    str += "GLOBAL_VAR_DEF";
    break;
  case IF_STMT:
    str += "IF_STMT";
    break;
  case INIT_VAL:
    str += "INIT_VAL";
    break;
  case INT_LITERAL:
    str += "INT_LITERAL";
    break;
  case LOCAL_VAR_DEF:
    str += "LOCAL_VAR_DEF";
    break;
  case L_VAL:
    str += "L_VAL:";
    str += symbol->name;
    break;
  case MEMSET_ZERO:
    str += "MEMSET_ZERO";
    break;
  case RETURN_STMT:
    str += "RETURN_STMT";
    break;
  case ROOT:
    str += "ROOT";
    break;
  case FORMAT:
    str += "FORMAT";
    break;
  case UNARY_EXP:
    str += "UNARY_EXP";
    break;
  case WHILE_STMT:
    str += "WHILE_STMT";
    break;
  default:
    break;
  }
  str += "\n";
  if(nodes.size()!=0)
  {
    for (int i = 0; i < nodes.size(); i++)
    {
      str += nodes[i]->toString();
    }
  }
  return str;
}

parseNode::parseNode(parseNodeType parseType, bool isFloat, OPType opType,
         const vector<parseNode *> &nodes) {
  this->parseType = parseType;
  this->isFloat = isFloat;
  this->opType = opType;
  this->nodes = nodes;
  this->symbol = nullptr;
}

parseNode::parseNode(parseNodeType parseType, bool isFloat, Symbol *symbol,
         const vector<parseNode *> &nodes) {
  this->parseType = parseType;
  this->isFloat = isFloat;
  this->symbol = symbol;
  this->nodes = nodes;
}

parseNode::parseNode(parseNodeType parseType, bool isFloat, const vector<parseNode *> &nodes) {
  this->parseType = parseType;
  this->isFloat = isFloat;
  this->nodes = nodes;
  this->symbol = nullptr;
}

parseNode::parseNode(float fVal) {
  this->parseType = FLOAT_LITERAL;
  this->isFloat = true;
  this->fVal = fVal;
  this->symbol = nullptr;
}

parseNode::parseNode(int iVal) {
  this->parseType = INT_LITERAL;
  this->isFloat = false;
  this->iVal = iVal;
  this->symbol = nullptr;
}

parseNode::parseNode(string str) {
  this->parseType = FORMAT;
  this->isFloat = false;
  this->format = str;
  this->symbol = nullptr;
}

parseNode::~parseNode() {
  for (parseNode *node : nodes)
    delete node;
}
