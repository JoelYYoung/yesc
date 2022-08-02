#include <iostream>
#include <string>
#include <cstdio>
#include "lexer.h"
#include <vector>
#include <map>
using namespace std;

void printToken(tokenType symbol, string name);

int main(int argc, char* argv[]){
	vector<tokenInfo> tokenInfoList = lexicalAnalyze(argv[1]);
//    vector<tokenInfo> tokenInfoList = lexicalAnalyze();
	auto vit = tokenInfoList.begin();
	while(vit != tokenInfoList.end()){
		printToken(vit->symbol, vit->name);
		vit ++;
	}	
	return 0;
}

void printToken(tokenType symbol, string name){
	map<tokenType, string> type2String;
	type2String[IDENT] = string("ID");
	type2String[INTCONST] = string("INT_LIT");
    type2String[FLOATCONST] = string("FLOAT_LIT");
	type2String[STRCONST] = string("STRING");
	type2String[CONST] = string("CONST");
	type2String[INT] = string("INT");
	type2String[VOID] = string("VOID");
	type2String[FLOAT] = string("FLOAT");
	type2String[IF_TK] = string("IF");
	type2String[ELSE_TK] = string("ELSE");
	type2String[WHILE_TK] = string("WHILE");
	type2String[BREAK_TK] = string("BREAK");
	type2String[CONTINUE_TK] = string("CONTINUE");
	type2String[RETURN_TK] = string("RETURN");
	type2String[COMMA] = string("COMMA");
	type2String[SEMICOLON] = string("SEMICOLON");
	type2String[LB] = string("LB");
	type2String[RB] = string("RB");
	type2String[LC] = string("LC");
	type2String[RC] = string("RC");
	type2String[LP] = string("LP");
	type2String[RP] = string("RP");
	type2String[ASSIGN] = string("ASSIGN");
	type2String[PLUS] = string("ADD");
	type2String[MINUS] = string("MINUS");
	type2String[MUL] = string("MUL");
	type2String[DIV] = string("DIV");
	type2String[MOD] = string("MOD");
	type2String[LT] = string("LT");
	type2String[GT] = string("GT");
	type2String[LE] = string("LE");
	type2String[GE] = string("GE");
	type2String[EQ] = string("EQ");
	type2String[NE] = string("NE");
	type2String[NOT] = string("NOT");
	type2String[AND] = string("AND");
	type2String[OR] = string("OR");
	
	cout << name << " : " << type2String[symbol] << endl;
}
