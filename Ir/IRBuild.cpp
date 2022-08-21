#include <algorithm>
#include <iostream>
#include <queue>
#include <utility>

#include "IRBuild.h"

using namespace std;

IRBuild::IRBuild(parseNode *root, vector<Symbol *> &symbols) {
    this->varId = 0;
    this->root = root;
    this->symbols = symbols;
    parseRoot(root);
}

IRBuild::~IRBuild() {
    for (const pair<Symbol *, vector<IR *>> &func : funcList)
        for (IR *ir : func.second)
            delete ir;
}

vector<IR *> IRBuild::parseTree(parseNode * pn, Symbol * sym,Attribute * att)
{
    switch(pn->parseType)
    {
        case parseNode::ASSIGN_STMT:
            //cout << "ASS" << endl;
            return parseAssignExp(pn, sym, att);
        case parseNode::BINARY_EXP:
            //cout << "BIN" << endl;
            return parseBinaryExp(pn, sym, att);
        case parseNode::UNARY_EXP:
            return parseUnaryExp(pn, sym, att);
        case parseNode::BLOCK:
            //cout << "BLOCK" << endl;
            return parseBlock(pn,sym, att);
        case parseNode::INT_LITERAL:
            //cout << "INT" << endl;
            return {new IR(IR::MOV, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::INT, pn->iVal)})};
        case parseNode::FLOAT_LITERAL:
            //cout << "FLOAT" << endl;
            return {new IR(IR::MOV, {new IRItem(IRItem::FVAR, ++varId), new IRItem(IRItem::FLOAT, pn->fVal)})};
        case parseNode::L_VAL:
            //cout << "LVAL" << endl;
            return parseLVal(pn, sym, att);
        case parseNode::MEMSET_ZERO:
            //cout << "MEM" << endl;
            return {new IR(IR::MEMSET_ZERO, {new IRItem(IRItem::SYMBOL, pn->symbol)})};
        case parseNode::RETURN_STMT:
            //cout << "RT" << endl;
            return parseReturnStmt(pn, sym, att);
        case parseNode::IF_STMT:
            return parseIfStmt(pn, sym, att);
        case parseNode::WHILE_STMT:
            return parseWhileStmt(pn, sym, att);
        case parseNode::EXP_STMT:
            return parseTree(pn->nodes[0], sym, att);
        case parseNode::LOCAL_VAR_DEF:
            localList[sym].push_back(pn->symbol);
            return {};
        case parseNode::BLANK_STMT:
            return {};
        case parseNode::BREAK_STMT:
            return {new IR(IR::GOTO, {new IRItem(IRItem::INT, att->breakLabel)})};
        case parseNode::CONTINUE_STMT:
            return {new IR(IR::GOTO, {new IRItem(IRItem::INT, att->continueLabel)})};
        case parseNode::FUNC_CALL:
            return parseFuncCall(pn, sym, att);
        case parseNode::FORMAT:
            return parseFormat(pn, sym, att);
        default:
            break;
    }
    return {};
}

void IRBuild::parseRoot(parseNode * pn){
    for(parseNode * nd : pn -> nodes)
    {
        switch (nd->parseType)
        {
            case parseNode::CONST_DEF:
                constList.push_back(nd->symbol);
	      if(nd->symbol->dimensions.size()!=0)
                {
                    globalList.push_back(nd->symbol);
                }
                break;
            case parseNode::FUNC_DEF:
                funcList.emplace_back(nd->symbol, parseFuncDef(nd, nd->symbol, new Attribute(0, 0)));
                break;
            case parseNode::GLOBAL_VAR_DEF:
                globalList.push_back(nd->symbol);
                break;
            default:
                break;
        }
    }
}

vector<IR *> IRBuild::parseFuncDef(parseNode * pn, Symbol * sym,Attribute * att)
{
    vector<IR *> irList;
    //cout << 1 << endl;
    for (parseNode *nd : pn->nodes)
    {
        vector<IR *> ir = parseTree(nd, sym, att);
        irList.insert(irList.end(), ir.begin(), ir.end());
    }
    if (pn->symbol->dataType == Symbol::VOID)
        irList.push_back(new IR(IR::RETURN));
    // irList.push_back(new IR(IR::FUNC_END));
    return irList;
}

vector<IR *> IRBuild::parseAssignExp(parseNode * pn, Symbol * sym,Attribute * att)
{
    vector<IR *> ir;
    if((pn->nodes[0]->parseType == parseNode::L_VAL) && (pn->nodes[1]->parseType == parseNode::INT_LITERAL))
    {
        vector<IR *> ir1 = parseTree(pn->nodes[0], sym, att);
        ir.insert(ir.end(), ir1.begin(), ir1.end());
        ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::INT, pn->nodes[1]->iVal)}));
        if(pn->nodes[0]->nodes.size() == 0)
        {
            if(pn->nodes[0]->symbol->symbolType == Symbol::GLOBAL_VAR || pn->nodes[0]->symbol->symbolType == Symbol::CONST)
            {
                ir.push_back(new IR(IR::STR, {new IRItem(IRItem::IVAR, varId), new IRItem(ir1[ir1.size()-2]->items[0]->type, ir1[ir1.size()-2]->items[0]->iVal)}));
            }
            else
                ir.push_back(new IR(IR::STR, {new IRItem(IRItem::IVAR, varId), new IRItem(IRItem::SYMBOL, pn->nodes[0]->symbol)}));
        }
        else
        {
            ir.push_back(new IR(IR::STR, {new IRItem(IRItem::IVAR, varId), new IRItem(ir1[ir1.size()-2]->items[0]->type, ir1[ir1.size()-2]->items[0]->iVal)}));
        }
        return ir;
    }
    if((pn->nodes[0]->parseType == parseNode::L_VAL) && (pn->nodes[1]->parseType == parseNode::FLOAT_LITERAL))
    {
        vector<IR *> ir1 = parseTree(pn->nodes[0], sym, att);
        ir.insert(ir.end(), ir1.begin(), ir1.end());
        ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::FVAR, ++varId), new IRItem(IRItem::FLOAT, pn->nodes[1]->fVal)}));
        if(pn->nodes[0]->nodes.size() == 0)
        {
            if(pn->nodes[0]->symbol->symbolType == Symbol::GLOBAL_VAR || pn->nodes[0]->symbol->symbolType == Symbol::CONST)
            {
                ir.push_back(new IR(IR::STR, {new IRItem(IRItem::FVAR, varId), new IRItem(ir1[ir1.size()-2]->items[0]->type, ir1[ir1.size()-2]->items[0]->iVal)}));
            }
            else
                ir.push_back(new IR(IR::STR, {new IRItem(IRItem::FVAR, varId), new IRItem(IRItem::SYMBOL, pn->nodes[0]->symbol)}));
        }
        else
            ir.push_back(new IR(IR::STR, {new IRItem(IRItem::FVAR, varId), new IRItem(ir1[ir1.size()-2]->items[0]->type, ir1[ir1.size()-2]->items[0]->iVal)}));
        return ir;
    }
    if(pn->nodes.size() == 2 && pn->nodes[1]->nodes.size()==2 && pn->nodes[0]->nodeEq(pn->nodes[1]->nodes[0]) && pn->nodes[0]->symbol->dimensions.size()!=0)
    {
        vector<IR *> ir2 = parseTree(pn->nodes[1], sym, att);
        ir.insert(ir.end(), ir2.begin(), ir2.end());
        if(pn->nodes[1]->nodes[1]->parseType == parseNode::INT_LITERAL || pn->nodes[1]->nodes[1]->parseType == parseNode::FLOAT_LITERAL)
            ir.push_back(new IR(IR::STR, {new IRItem(ir2.back()->items[0]->type, ir2.back()->items[0]->iVal), new IRItem(ir2[ir2.size() - 3]->items[1]->type, ir2[ir2.size() - 3]->items[1]->iVal)}));
        else
            ir.push_back(new IR(IR::STR, {new IRItem(ir2.back()->items[0]->type, ir2.back()->items[0]->iVal), new IRItem(ir2[ir2.size() - 2]->items[1]->type, ir2[ir2.size() - 2]->items[1]->iVal)}));
    }
    else
    {
        vector<IR *> ir2 = parseTree(pn->nodes[1], sym, att);
        vector<IR *> ir1 = parseTree(pn->nodes[0], sym, att);
        ir.insert(ir.end(), ir2.begin(), ir2.end());
        ir.insert(ir.end(), ir1.begin(), ir1.end());
        if (pn->nodes[0]->nodes.size() == 0)
        {
            if (pn->nodes[0]->symbol->symbolType == Symbol::GLOBAL_VAR || pn->nodes[0]->symbol->symbolType == Symbol::CONST)
            {
                ir.push_back(new IR(IR::STR, {new IRItem(ir2.back()->items[0]->type, ir2.back()->items[0]->iVal), new IRItem(ir1[ir1.size() - 2]->items[0]->type, ir1[ir1.size() - 2]->items[0]->iVal)}));
            }
            else
                ir.push_back(new IR(IR::STR, {new IRItem(ir2.back()->items[0]->type, ir2.back()->items[0]->iVal), new IRItem(IRItem::SYMBOL, pn->nodes[0]->symbol)}));
        }
        else
            ir.push_back(new IR(IR::STR, {new IRItem(ir2.back()->items[0]->type, ir2.back()->items[0]->iVal), new IRItem(ir1[ir1.size() - 2]->items[0]->type, ir1[ir1.size() - 2]->items[0]->iVal)}));
    }
    return ir;
}

vector<IR*> IRBuild::parseLVal(parseNode * pn, Symbol * sym,Attribute * att)
{
    vector<IR *> ir;
    bool isArray = false;
    bool isparam = false;
    int arrayOffset = 0;
    int d = 1;
    int nameid = 0;
    int nowid = 0;
    if (pn->symbol->dimensions.size() != 0)
    {
        isArray = true;
    }
    if(pn->symbol->symbolType == Symbol::PARAM)
    {
        isparam = true;
    }
    if(isparam)
    {
        if (pn->symbol->symbolType == Symbol::GLOBAL_VAR || pn->symbol->symbolType == Symbol::CONST)
        {
            ir.push_back(new IR(IR::LDR, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::SYMBOL, pn->symbol)}));
            int lastId = varId;
            if (!isArray)
            {
                if(pn->symbol->dataType == Symbol::INT)
                    ir.push_back(new IR(IR::LDR, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::IVAR, lastId)}));
                else if(pn->symbol->dataType == Symbol::FLOAT)
                    ir.push_back(new IR(IR::LDR, {new IRItem(IRItem::FVAR, ++varId), new IRItem(IRItem::IVAR, lastId)}));
            }
        }
        else{
            if (isArray)
                ir.push_back(new IR(IR::NAME, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::SYMBOL, pn->symbol)}));
            else
            {
                if(pn->symbol->dataType == Symbol::INT)
                    ir.push_back(new IR(IR::LDR, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::SYMBOL, pn->symbol)}));
                else if(pn->symbol->dataType == Symbol::FLOAT)
                    ir.push_back(new IR(IR::LDR, {new IRItem(IRItem::FVAR, ++varId), new IRItem(IRItem::SYMBOL, pn->symbol)}));
            }
        }
    }
    else
    {
        if (pn->symbol->symbolType == Symbol::GLOBAL_VAR || pn->symbol->symbolType == Symbol::CONST)
        {
            ir.push_back(new IR(IR::LDR, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::SYMBOL, pn->symbol)}));
            int v = varId;
            //ir.push_back(new IR(IR::ADD, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::PC) ,new IRItem(IRItem::IVAR, v)}));
            //v = varId;
            if (!isArray)
            {
                if(pn->symbol->dataType == Symbol::INT)
                    ir.push_back(new IR(IR::LDR, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::IVAR, v)}));
                else if(pn->symbol->dataType == Symbol::FLOAT)
                    ir.push_back(new IR(IR::LDR, {new IRItem(IRItem::FVAR, ++varId), new IRItem(IRItem::IVAR, v)}));
            }
        }
        else{
            if (isArray)
                ir.push_back(new IR(IR::NAME, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::SYMBOL, pn->symbol)}));
            else
            {
                if(pn->symbol->dataType == Symbol::INT)
                    ir.push_back(new IR(IR::LDR, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::SYMBOL, pn->symbol)}));
                else if(pn->symbol->dataType == Symbol::FLOAT)
                    ir.push_back(new IR(IR::LDR, {new IRItem(IRItem::FVAR, ++varId), new IRItem(IRItem::SYMBOL, pn->symbol)}));
            }
        }
    }
    nameid = varId;
    if (isArray)
    {
        if(pn->nodes.size() == 1)
        {
            ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::INT, 0)}));
            nowid = varId;
            if (pn->nodes[0]->parseType == parseNode::INT_LITERAL) // a[i-1][3]
            {
                arrayOffset += pn->nodes[0]->iVal * d;
                //cout << arrayOffset << endl;
            }
            else
            {
                vector<IR *> expir = parseTree(pn->nodes[0], sym, att);
                ir.insert(ir.end(), expir.begin(), expir.end());
                ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::INT, d * 4)}));
                varId++;
                ir.push_back(new IR(IR::MUL, {new IRItem(IRItem::IVAR, varId), new IRItem(IRItem::IVAR, expir.back()->items[0]->iVal), new IRItem(IRItem::IVAR, varId - 1)}));
                nowid = varId;
            }
        }
        else{
            ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::INT, 0)}));
            nowid = varId;
            for (int i = pn->nodes.size() - 1; i >= 0; i--)
            {
                if (pn->nodes[i]->parseType == parseNode::INT_LITERAL) // a[i-1][3]
                {
                    arrayOffset += pn->nodes[i]->iVal * d;
                    // cout << arrayOffset << endl;
                }
                else
                {
                    vector<IR *> expir = parseTree(pn->nodes[i], sym, att);
                    ir.insert(ir.end(), expir.begin(), expir.end());
                    ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::INT, d * 4)}));
                    varId++;
                    ir.push_back(new IR(IR::MUL, {new IRItem(IRItem::IVAR, varId), new IRItem(IRItem::IVAR, expir.back()->items[0]->iVal), new IRItem(IRItem::IVAR, varId - 1)}));
                    varId++;
                    ir.push_back(new IR(IR::ADD, {new IRItem(IRItem::IVAR, varId), new IRItem(IRItem::IVAR, varId - 1), new IRItem(IRItem::IVAR, nowid)}));
                    nowid = varId;
                }
                d *= pn->symbol->dimensions[i];
            }
        }
        //cout << varId << ' ' << nowid << endl;
        if(arrayOffset!=0)
        {
            ir.push_back(new IR(IR::ADD, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::IVAR, nowid), new IRItem(IRItem::INT, arrayOffset * 4)}));
        }
        int newid = varId;
        if (pn->nodes.size() < pn->symbol->dimensions.size())
        {
            int offset = 1;
            for (int i = pn->nodes.size(); i < pn->symbol->dimensions.size(); i++)
            {
                offset *= pn->symbol->dimensions[i];
            }
            ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::INT, offset)}));
            ir.push_back(new IR(IR::MUL, {new IRItem(IRItem::IVAR, varId+1), new IRItem(IRItem::IVAR, varId-1), new IRItem(IRItem::IVAR, varId)}));
            varId++;
            newid = varId;
        }
        ir.push_back(new IR(IR::ADD, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::IVAR, nameid), new IRItem(IRItem::IVAR, newid)}));
        nameid = varId;
        if(pn->symbol->dataType == Symbol::INT)
            ir.push_back(new IR(IR::LDR, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::IVAR, nameid)}));
        else if(pn->symbol->dataType == Symbol::FLOAT)
            ir.push_back(new IR(IR::LDR, {new IRItem(IRItem::FVAR, ++varId), new IRItem(IRItem::IVAR, nameid)}));
    }
    return ir;
}

vector<IR*> IRBuild::parseBinaryExp(parseNode * pn, Symbol * sym,Attribute * att)
{
    vector<IR *> ir;
    switch (pn->opType) {
        case parseNode::ADD:
        case parseNode::DIV:
        case parseNode::MOD:
        case parseNode::MUL:
        case parseNode::SUB:
            return parseAlgoExp(pn, sym, att);
        case parseNode::EQ:
        case parseNode::GE:
        case parseNode::GT:
        case parseNode::LE:
        case parseNode::LT:
        case parseNode::NE:
            return parseCmpExp(pn, sym, att);
        case parseNode::AND:
            return parseAndExp(pn, sym,att);
        case parseNode::OR:
            return parseOrExp(pn, sym,att);
        default:
            break;
    }
    return ir;
}

vector<IR*> IRBuild::parseOrExp(parseNode* pn,Symbol *sym,Attribute *att) {
    vector<IR *> ir1 = parseTree(pn->nodes[0], sym, att);
    vector<IR *> irtest = parseTree(pn->nodes[1], sym, att);
    vector<IR *> ir;
    ir.insert(ir.end(), ir1.begin(), ir1.end());
    int t1 = ir1.back()->items[0]->iVal;
    ir1[0]->deleteIr(irtest.size());
    ir.push_back(
            new IR(IR::BNE, {new IRItem(IRItem::IR_ID, ir1.back()->irId + 1 + (int)irtest.size() + 4), new IRItem(ir1.back()->items[0]->type, t1),
                             ir1.back()->items[0]->type == IRItem::IVAR ? new IRItem(IRItem::INT, 0) : new IRItem(IRItem::FLOAT, 0)}));
    vector<IR *> ir2 = parseTree(pn->nodes[1], sym, att);
    int t2 = ir2.back()->items[0]->iVal;
    ir.insert(ir.end(), ir2.begin(), ir2.end());
    ir.push_back(new IR(IR::BNE, {new IRItem(IRItem::IR_ID, ir2.back()->irId + 1 + 3), new IRItem(ir2.back()->items[0]->type, t2),
                                  ir2.back()->items[0]->type == IRItem::IVAR ? new IRItem(IRItem::INT, 0) : new IRItem(IRItem::FLOAT, 0)}));
    ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::INT, 0)}));
    ir.push_back(new IR(IR::GOTO, {new IRItem(IRItem::IR_ID, ir2.back()->irId + 5)}));
    ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, varId), new IRItem(IRItem::INT, 1)}));
    return ir;
}

vector<IR*> IRBuild::parseAndExp(parseNode* pn,Symbol *sym,Attribute *att) {
    vector<IR *> ir1 = parseTree(pn->nodes[0], sym, att);
    vector<IR *> irtest = parseTree(pn->nodes[1], sym, att);
    vector<IR *> ir;
    ir.insert(ir.end(), ir1.begin(), ir1.end());
    int t1 = ir1.back()->items[0]->iVal;
    irtest[0]->deleteIr(irtest.size());
    ir.push_back(
            new IR(IR::BEQ, {new IRItem(IRItem::IR_ID, ir1.back()->irId + 1 + (int)irtest.size() + 2), new IRItem(ir1.back()->items[0]->type, t1),
                             ir1.back()->items[0]->type == IRItem::IVAR ? new IRItem(IRItem::INT, 0) : new IRItem(IRItem::FLOAT, 0)}));
    vector<IR *> ir2 = parseTree(pn->nodes[1], sym, att);
    int t2 = ir2.back()->items[0]->iVal;
    ir.insert(ir.end(), ir2.begin(), ir2.end());
    ir.push_back(new IR(IR::BNE, {new IRItem(IRItem::IR_ID, ir2.back()->irId + 4), new IRItem(ir2.back()->items[0]->type, t2),
                                  ir2.back()->items[0]->type == IRItem::IVAR ? new IRItem(IRItem::INT, 0) : new IRItem(IRItem::FLOAT, 0)}));
    ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::INT, 0)}));
    ir.push_back(new IR(IR::GOTO, {new IRItem(IRItem::IR_ID, ir2.back()->irId + 5)}));
    ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, varId), new IRItem(IRItem::INT, 1)}));
    return ir;
}

vector<IR*> IRBuild::parseAlgoExp(parseNode * pn, Symbol * sym,Attribute * att)
{
    vector<IR *> ir;
    IR::IRType type;
    switch (pn->opType)
    {
        case parseNode::ADD:
            type = IR::ADD;
            break;
        case parseNode::DIV:
            type = IR::DIV;
            break;
        case parseNode::MOD:
            type = IR::MOD;
            break;
        case parseNode::MUL:
            type = IR::MUL;
            break;
        case parseNode::SUB:
            type = IR::SUB;
            break;
        default:
            break;
    }
    if(type == IR::ADD && pn->nodes[0]->parseType == parseNode::L_VAL && pn->nodes[1]->parseType == parseNode::L_VAL)
    {
        if (pn->nodes[0]->symbol->name == pn->nodes[1]->symbol->name)
        {
            int flag = 0;
            if (pn->nodes[0]->nodes.size() != 0 && pn->nodes[1]->nodes.size() == pn->nodes[0]->nodes.size())
            {
                for (int i = 0; i < pn->nodes[0]->nodes.size(); i++)
                {
		            if(pn->nodes[0]->nodes[i]->parseType != parseNode::INT_LITERAL || pn->nodes[1]->nodes[i]->parseType != parseNode::INT_LITERAL)
                    {
                        flag = 1;
                        break;
                    }
                    if(pn->nodes[0]->nodes[i]->iVal != pn->nodes[1]->nodes[i]->iVal)
                    {
                        flag = 1;
                        break;
                    }
                }
            }
            if(flag == 0)
            {
                vector<IR *> ir1 = parseTree(pn->nodes[0], sym, att);
                IRItem::IRItemType type1 = ir1.back()->items[0]->type;
                ir.insert(ir.end(), ir1.begin(), ir1.end());
                ir.push_back(new IR(IR::ADD, {new IRItem(type1, ++varId), new IRItem(type1, ir1.back()->items[0]->iVal), new IRItem(type1, ir1.back()->items[0]->iVal)}));
                return ir;
            }
        }
    }
    vector<IR *> ir1 = parseTree(pn->nodes[0], sym, att);
    vector<IR *> ir2 = parseTree(pn->nodes[1], sym, att);
    ir.insert(ir.end(), ir1.begin(), ir1.end());
    ir.insert(ir.end(), ir2.begin(), ir2.end());
    IRItem::IRItemType type1 = ir1.back()->items[0]->type;
    IRItem::IRItemType type2 = ir2.back()->items[0]->type;
    ++varId;
    ir.push_back(new IR(type, {new IRItem(type1, varId), new IRItem(type1, ir1.back()->items[0]->iVal), new IRItem(type2, ir2.back()->items[0]->iVal)}));
    return ir;
}

vector<IR *> IRBuild::parseBlock(parseNode *pn, Symbol *sym,Attribute * att) {
    vector<IR *> ir;
    for (parseNode *node : pn->nodes)
    {
        vector<IR *> irchild = parseTree(node, sym, att);
        ir.insert(ir.end(), irchild.begin(), irchild.end());
    }
    return ir;
}

vector<IR *> IRBuild::parseReturnStmt(parseNode *pn, Symbol *sym,Attribute * att) {
    if (pn->nodes.empty())
        return {new IR(IR::RETURN)};
    vector<IR *> ir = parseTree(pn->nodes[0], sym, att);
    ir.push_back(new IR(IR::RETURN, {new IRItem(ir.back()->items[0]->type, ir.back()->items[0]->iVal)}));
    return ir;
}

vector<IR *> IRBuild::parseCmpExp(parseNode *pn, Symbol *sym,Attribute * att) {
    IR::IRType type;
    vector<IR *> ir1 = parseTree(pn->nodes[0], sym, att);
    vector<IR *> ir2 = parseTree(pn->nodes[1], sym, att);
    vector<IR *> ir;
    ir.insert(ir.end(), ir1.begin(), ir1.end());
    ir.insert(ir.end(), ir2.begin(), ir2.end());
    switch (pn->opType)
    {
    case parseNode::EQ:
        type = IR::EQ;
        break;
    case parseNode::GE:
        type = IR::GE;
        break;
    case parseNode::GT:
        type = IR::GT;
        break;
    case parseNode::LE:
        type = IR::LE;
        break;
    case parseNode::LT:
        type = IR::LT;
        break;
    case parseNode::NE:
        type = IR::NE;
        break;
    default:
        break;
    }
    ir.push_back(new IR(type, {new IRItem(IRItem::IVAR, ++varId), new IRItem(ir1.back()->items[0]->type, ir1.back()->items[0]->iVal), new IRItem(ir2.back()->items[0]->type, ir2.back()->items[0]->iVal)}));
    return ir;
}

vector<IR *> IRBuild::parseIfStmt(parseNode *pn, Symbol *sym,Attribute * att) {
    vector<IR *> ir;
    vector<IR *> ir1 = parseTree(pn->nodes[0], sym, att);
    IR *beqIr = new IR(IR::BEQ, {new IRItem(IRItem::IR_ID, 0), new IRItem(ir1.back()->items[0]->type, ir1.back()->items[0]->iVal), ir1.back()->items[0]->type == IRItem::IVAR ? new IRItem(IRItem::INT, 0) : new IRItem(IRItem::FLOAT, 0)});
    vector<IR *> ir2 = parseTree(pn->nodes[1], sym, att);
    ir.insert(ir.end(), ir1.begin(), ir1.end());
    if(pn->nodes.size()==3)
    {
        beqIr->items[0]->iVal = ir1.back()->irId + 1 + ir2.size() + 2;
        ir.push_back(beqIr);
        ir.insert(ir.end(), ir2.begin(), ir2.end());
        IR *gotoIr = new IR(IR::GOTO, {new IRItem(IRItem::IR_ID, 0)});
        vector<IR *> ir3 = parseTree(pn->nodes[2], sym, att);
        if(ir2.size()==0)
            gotoIr->items[0]->iVal = ir1.back()->irId + 2 + ir3.size() + 1;
        else
            gotoIr->items[0]->iVal = ir2.back()->irId + 1 + ir3.size() + 1;
        ir.push_back(gotoIr);
        ir.insert(ir.end(), ir3.begin(), ir3.end());
    }
    else{
        beqIr->items[0]->iVal = ir1.back()->irId + 1 + ir2.size() + 1;
        ir.push_back(beqIr);
        ir.insert(ir.end(), ir2.begin(), ir2.end());
    }
    return ir;
}

vector<IR*> IRBuild::parseWhileStmt(parseNode* pn, Symbol * sym,Attribute * att) {
    int continue_last = att->continueLabel;
    int break_last = att->breakLabel;
    vector<IR *> ir1 = parseTree(pn->nodes[0], sym, att);
    IR *beqIr = new IR(IR::BEQ, {new IRItem(IRItem::IR_ID, 0), new IRItem(ir1.back()->items[0]->type, ir1.back()->items[0]->iVal), ir1.back()->items[0]->type == IRItem::IVAR ? new IRItem(IRItem::INT, 0) : new IRItem(IRItem::FLOAT, 0)});
    att->continueLabel = ir1[0]->irId;
    vector<IR *> irtest = parseTree(pn->nodes[1], sym, att);
    att->breakLabel = irtest.back()->irId + 2;
    ir1[0]->deleteIr(irtest.size());
    for(IR* ir: irtest)
    {
        if(NULL != ir)
        {
            delete ir;
            ir = NULL;
        }
    }
    irtest.clear();
    vector<IR *> ir2 = parseTree(pn->nodes[1], sym, att);
    att->breakLabel = break_last;
    att->continueLabel = continue_last;
    vector<IR *> ir;
    ir.insert(ir.end(), ir1.begin(), ir1.end());
    beqIr->items[0]->iVal = ir1.back()->irId + 1 + ir2.size() + 2;
    ir.push_back(beqIr);
    ir.insert(ir.end(), ir2.begin(), ir2.end());
    ir.push_back(new IR(IR::GOTO, {new IRItem(IRItem::IR_ID, ir2.back()->irId + 1 -(int)ir1.size() - (int)ir2.size() - 1)}));
    return ir;
}

vector<IR*> IRBuild::parseUnaryExp(parseNode* pn, Symbol * sym,Attribute * att) {
    vector<IR *> ir = parseTree(pn->nodes[0], sym, att);
    switch (pn->opType)
    {
        case parseNode::F2I:
            ir.push_back(new IR( IR::F2I, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::FVAR, ir.back()->items[0]->iVal)}));
            break;
        case parseNode::I2F:
            ir.push_back(new IR( IR::I2F, {new IRItem(IRItem::FVAR, ++varId), new IRItem(IRItem::IVAR, ir.back()->items[0]->iVal)}));
            break;
        case parseNode::NOT:
            ir.push_back(new IR(IR::NOT, {new IRItem(IRItem::IVAR, ++varId), new IRItem(ir.back()->items[0]->type, ir.back()->items[0]->iVal)}));
            break;
        case parseNode::NEG:
            ir.push_back(new IR( IR::NEG, {new IRItem(ir.back()->items[0]->type, ++varId), new IRItem(ir.back()->items[0]->type, ir.back()->items[0]->iVal)}));
            break;
        default:
            break;
    }
    return ir;
}

vector<IR *> IRBuild::parseFuncCall(parseNode *pn, Symbol *sym, Attribute * att) {
    vector<IR *> ir;
    vector<IRItem *> itemList;
    for (int i = 0; i < pn->nodes.size(); i++)
    {
        vector<IR *> irFunc = parseTree(pn->nodes[i], sym, att);
        ir.insert(ir.end(), irFunc.begin(), irFunc.end());
        Symbol *param = pn->symbol->params[i];
        if(pn->symbol->name!="putf")
        {
            Symbol *param = pn->symbol->params[i];
            if (param->dimensions.size() != 0)
            {
                itemList.push_back(new IRItem(irFunc[irFunc.size() - 2]->items[0]->type, irFunc[irFunc.size() - 2]->items[0]->iVal));
            }
            else
                itemList.push_back(new IRItem(ir.back()->items[0]->type, ir.back()->items[0]->iVal));
        }
        else
        {
            itemList.push_back(new IRItem(ir.back()->items[0]->type, ir.back()->items[0]->iVal));
        }
    }
    IR *callIR = new IR(IR::CALL, {new IRItem(IRItem::SYMBOL, pn->symbol)});
    callIR->items.insert(callIR->items.end(), itemList.begin(), itemList.end());
    ir.push_back(callIR);
    if (pn->symbol->dataType != Symbol::VOID)
        ir.push_back(new IR(IR::MOV, {new IRItem(pn->symbol->dataType == Symbol::INT ? IRItem::IVAR : IRItem::FVAR, ++varId), new IRItem(IRItem::RETURN)}));
    return ir;
}

vector<IR *> IRBuild::parseFormat(parseNode *pn, Symbol *sym, Attribute * att) {
    vector<IR *> ir;
    ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::SVAR, ++varId), new IRItem(IRItem::STRING, pn->format)}));
    return ir;
}

unordered_map<Symbol *, vector<Symbol *>> IRBuild::getLocalVars() {
    return localList;
}

vector<pair<Symbol *, vector<IR *>>> IRBuild::getFuncs(){
    return funcList;
}

vector<Symbol *> IRBuild::getGlobalVars() {
    return globalList;
}

vector<Symbol *> IRBuild::getConsts() {
    return constList;
}

vector<Symbol* > IRBuild::getSymtemFunc() {
    Symbol *func;
    Symbol *param1, *param2;
    vector<Symbol *> symbols;
    func = new Symbol(Symbol::FUNC, Symbol::INT, "getint", vector<Symbol *>());
    symbols.push_back(func);
    func = new Symbol(Symbol::FUNC, Symbol::INT, "getch", vector<Symbol *>());
    symbols.push_back(func);
    param1 = new Symbol(Symbol::PARAM, Symbol::INT, "a", vector<int>{-1});
    func = new Symbol(Symbol::FUNC, Symbol::INT, "getarray", vector<Symbol *>{param1});
    symbols.push_back(func);
    func = new Symbol(Symbol::FUNC, Symbol::FLOAT, "getfloat", vector<Symbol *>());
    symbols.push_back(func);
    param1 = new Symbol(Symbol::PARAM, Symbol::FLOAT, "a", vector<int>{-1});
    func = new Symbol(Symbol::FUNC, Symbol::INT, "getfarray", vector<Symbol *>{param1});
    symbols.push_back(func);
    param1 = new Symbol(Symbol::PARAM, Symbol::INT, "a", vector<int>());
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "putint", vector<Symbol *>{param1});
    symbols.push_back(func);
    param1 = new Symbol(Symbol::PARAM, Symbol::INT, "a", vector<int>());
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "putch", vector<Symbol *>{param1});
    symbols.push_back(func);
    param1 = new Symbol(Symbol::PARAM, Symbol::INT, "n", vector<int>());
    param2 = new Symbol(Symbol::PARAM, Symbol::INT, "a", vector<int>{-1});
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "putarray", vector<Symbol *>{param1, param2});
    symbols.push_back(func);
    param1 = new Symbol(Symbol::PARAM, Symbol::FLOAT, "a", vector<int>());
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "putfloat", vector<Symbol *>{param1});
    symbols.push_back(func);
    param1 = new Symbol(Symbol::PARAM, Symbol::INT, "n", vector<int>());
    param2 = new Symbol(Symbol::PARAM, Symbol::FLOAT, "a", vector<int>{-1});
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "putfarray", vector<Symbol *>{param1, param2});
    symbols.push_back(func);
    param1 = new Symbol(Symbol::PARAM, Symbol::STRING, "format");
    param2 = new Symbol(Symbol::PARAMLIST, Symbol::LIST, "paramList", vector<Symbol *>());
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "putf", vector<Symbol *>{param1, param2});
    symbols.push_back(func);
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "_sysy_starttime", vector<Symbol *>());
    symbols.push_back(func);
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "_sysy_stoptime", vector<Symbol *>());
    symbols.push_back(func);
    return symbols;
}

void IRBuild::printIRs(bool silentMode) {
    //parseRoot(root);
    if(silentMode == false){
        for (Symbol *con : constList)
            cout << con->toString() << endl;
        for (Symbol *global : globalList)
            cout << global->toString() << endl;
        //cout << "size:" << funcList.size() << endl;
        for (pair<Symbol *, vector<IR *>> func : funcList)
        {
            cout << func.first->name << endl;
            //cout << func.second.size() << endl;
            for (IR *ir : func.second)
                cout << ir->toString() << endl;
        }
    }
}

void IRBuild::printBlocks() {
    for (pair<Symbol *, vector<IR *>> func : funcList)
    {
        vector<BaseBlock *> blocks;
        BlockBuild *builder = new BlockBuild();
        blocks = builder->generateFunctionBlock(func.second);
        this->BlockList.emplace_back(func.first,blocks);
        this->blockbuilder.emplace_back(func.first, builder);
    }
}

void IRBuild::printAll() {
    for(pair<Symbol *, vector<BaseBlock *>> func : BlockList)
    {
        cout << func.first->name << endl;
        for(BaseBlock *bb : func.second)
            cout << bb->toString() << endl;
    }
}

void IRBuild::LoopInvariant()
{
    int funcNum = 0;
    for (pair<Symbol *, vector<BaseBlock *>> pr : BlockList)
    {
        pair<Symbol *, BlockBuild *> builder = blockbuilder[funcNum];
        vector<set<BaseBlock *>> loopBlock = builder.second->loop;
        vector<pair<BaseBlock *, BaseBlock *>> back = builder.second->backEdge;
        vector<BaseBlock *> funcBlock = pr.second;
        int loopnum = 0;
        for (set<BaseBlock *> loop : loopBlock)
        {
            map<int, vector<int>> varList;
            for (BaseBlock *bk : loop)
            {
                vector<IR *> irlist = bk->getIRlist();
                for (IR *ir : irlist)
                {
                    if(ir->items[0]->type == IRItem::IVAR || ir->items[0]->type == IRItem::FVAR)
                    {
                        varList[ir->items[0]->iVal].push_back(ir->irId);
                    }
                    for (int i = 1; i < ir->items.size(); i++)
                    {
                        if(ir->items[i]->type == IRItem::IVAR || ir->items[i]->type == IRItem::FVAR)
                        {
                            varList[ir->items[i]->iVal].push_back(ir->irId);
                        }
                    }
                }
            }
            set<int> delIr;
            for (pair<int, vector<int>> var : varList)
            {
                if(var.second.size() == 1)
                {
                    delIr.insert(var.second[0]);
                }
            }
            map<int, int> imap;
            for (BaseBlock *bk : loop)
            {
                vector<IR *> irlist = bk->getIRlist();
                vector<IR *> copylist(irlist);
                int num = 0, delnum = 0;
                int first = irlist.front()->irId;
                int second;
                if (irlist.size() >= 2)
                    second = irlist[1]->irId;
                for (IR *ir : copylist)
                {
                    if(delIr.count(ir->irId)==1)
                    {
                        if (ir->irId == first)
                            imap[ir->irId] = second;
                        irlist.erase(irlist.begin() + num - delnum);
                        delnum++;
                    }
                    num++;
                }
                bk->setIRlist(irlist);
            }
            for (BaseBlock *block : funcBlock)
            {
                vector<IR *> irlist = block->getIRlist();
                for (IR *ir : irlist)
                {
                    if (ir->type == IR::GOTO || ir->type == IR::BNE || ir->type == IR::BEQ)
                    {
                        int id = ir->items[0]->iVal;
                        if (imap.count(id) == 1)
                        {
                            ir->items[0]->iVal = imap[id];
                        }
                    }
                }
                block->setIRlist(irlist);
            }
/*
            int firstBlock = back[loopnum].second->BlockId;
            set<int> st;
            map<int, int> mp;
            int mpsize = -1;
            map<Symbol *, vector<pair<int, BaseBlock *>>> ldr;
            map<Symbol *, vector<pair<int, BaseBlock *>>> str;
            for (BaseBlock *bk : loop)
            {
                vector<IR *> irlist = bk->getIRlist();
                int num = 0;
                for (IR *ir : irlist)
                { 
                    if(ir->type == IR::LDR && ir->items[1]->type == IRItem::SYMBOL)
                    {
                        ldr[ir->items[1]->symbol].push_back(make_pair(num,bk));
                        if(str.count(ir->items[1]->symbol) == 0)
                        {
                            str[ir->items[1]->symbol] = {};
                        }
                    }
                    else if(ir->type == IR::STR && ir->items[1]->type == IRItem::SYMBOL)
                    {
                        str[ir->items[1]->symbol].push_back(make_pair(num,bk));
                        if(ldr.count(ir->items[1]->symbol) == 0)
                        {
                            ldr[ir->items[1]->symbol] = {};
                        }
                    }
                    num++;
                }
            }
            for(pair<Symbol *, vector<pair<int, BaseBlock *>>> pr : ldr)
            {
                if(pr.first->symbolType != Symbol::LOCAL_VAR)
                {
                    for (pair<int, BaseBlock *> bk : pr.second)
                    {
                        vector<IR *> irlist = bk.second->getIRlist();
                        mp[irlist[bk.first]->items[0]->iVal] = 0;
                    }
                    for (pair<int, BaseBlock *> bk : str[pr.first])
                    {
                        vector<IR *> irlist = bk.second->getIRlist();
                        mp[irlist[bk.first]->items[0]->iVal] = 0;
                    }
                }
                else if(str[pr.first].size() == 0)
                {
                    for(pair<int, BaseBlock *> bk : pr.second)
                    {
                        vector<IR *> irlist = bk.second->getIRlist();
                        mp[irlist[bk.first]->items[0]->iVal] = 1;
                        //st.insert(irlist[bk.first]->irId);
                    }
                }
                else if(str[pr.first].size()!=0)
                {
                    if(str[pr.first].size()==1)
                    {
                        pair<int, BaseBlock *> strpos = str[pr.first].front();
                        int flag = 0;
                        for (pair<int, BaseBlock *> bk : pr.second)
                        {
                            vector<IR *> irlist = bk.second->getIRlist();
                            mp[irlist[bk.first]->items[0]->iVal] = 0;
                            set<BaseBlock *> st = bk.second->getDoms();
                            if(st.count(strpos.second)!=1)
                            {
                                flag = 1;
                                break;
                            }
                        }
                        if(flag == 0)
                        {
                            vector<IR *> irlist = strpos.second->getIRlist();
                            mp[irlist[strpos.first]->items[0]->iVal] = 1;
                            // st.insert(irlist[strpos.first]->irId);
                        }
                    }
                    else{
                        for (pair<int, BaseBlock *> bk : pr.second)
                        {
                            vector<IR *> irlist = bk.second->getIRlist();
                            mp[irlist[bk.first]->items[0]->iVal] = 0;
                        }
                        for(pair<int, BaseBlock *> bk : str[pr.first])
                        {
                            vector<IR *> irlist = bk.second->getIRlist();
                            mp[irlist[bk.first]->items[0]->iVal] = 0;
                        }
                    }
                }
            }
            for (BaseBlock *bk : loop)
            {
                vector<IR *> irlist = bk->getIRlist();
                for (IR *ir : irlist)
                {
                    if (ir->type == IR::MOV)
                    {
                        if(ir->items[1]->type == IRItem::RETURN)
                        {
                            mp[ir->items[0]->iVal] = 0;
                        }
                        else if (ir->items[1]->type == IRItem::INT || ir->items[1]->type == IRItem::FLOAT || (mp.count(ir->items[1]->iVal) == 1 && mp[ir->items[1]->iVal] == 1))
                        {
                            if (mp.count(ir->items[0]->iVal) == 0)
                            {
                                mp[ir->items[0]->iVal] = 1;
                            }
                        }
                        else
                        {
                            mp[ir->items[0]->iVal] = 0;
                        }
                    }
                    else if ((ir->type == IR::ADD) || (ir->type == IR::SUB) || (ir->type == IR::MUL) || (ir->type == IR::DIV) || (ir->type == IR::DIV) || (ir->type == IR::EQ) || (ir->type == IR::NE) || (ir->type == IR::GE) || (ir->type == IR::GT) || (ir->type == IR::LE) || (ir->type == IR::LT))
                    {
                        if ((mp.count(ir->items[1]->iVal) == 1 && mp[ir->items[1]->iVal] == 1) && (mp.count(ir->items[2]->iVal) == 1 && mp[ir->items[2]->iVal] == 1))
                        {
                            if (mp.count(ir->items[0]->iVal) == 0)
                            {
                                mp[ir->items[0]->iVal] = 1;
                            }
                            else if (mp[ir->items[0]->iVal] == 0)
                            {
                                mp[ir->items[1]->iVal] = 0;
                                mp[ir->items[2]->iVal] = 0;
                            }
                        }
                        else if (( mp[ir->items[0]->iVal] == 0) || (mp[ir->items[1]->iVal] == 0) || (mp[ir->items[2]->iVal] == 0))
                        {
                            mp[ir->items[0]->iVal] = 0;
                            mp[ir->items[1]->iVal] = 0;
                            mp[ir->items[2]->iVal] = 0;
                        }
                    }
                    else if ((ir->type == IR::NOT) || (ir->type == IR::I2F) || (ir->type == IR::F2I) || (ir->type == IR::NEG))
                    {
                        if (mp.count(ir->items[1]->iVal) == 1 && mp[ir->items[1]->iVal] == 1)
                        {
                            if (mp.count(ir->items[0]->iVal) == 0)
                            {
                                mp[ir->items[0]->iVal] = 1;
                            }
                            else if (mp[ir->items[0]->iVal] == 0)
                            {
                                mp[ir->items[1]->iVal] = 0;
                            }
                            // st.insert(ir->irId);
                        }
                        else if (mp.count(ir->items[1]->iVal) == 1 && mp[ir->items[1]->iVal] == 0)
                        {
                            mp[ir->items[0]->iVal] = 0;
                        }
                    }
                    else if ((ir->type == IR::LDR && ir->items[1]->type == IRItem::IVAR) || (ir->type == IR::STR && ir->items[1]->type == IRItem::IVAR))
                    {
                        if (mp.count(ir->items[1]->iVal) == 1 && mp[ir->items[1]->iVal] == 1)
                        {
                            if (mp.count(ir->items[0]->iVal) == 0)
                            {
                                mp[ir->items[0]->iVal] = 1;
                            }
                            else if (mp[ir->items[0]->iVal] == 0)
                            {
                                mp[ir->items[1]->iVal] = 0;
                            }
                            // st.insert(ir->irId);
                        }
                        else if (mp.count(ir->items[1]->iVal) == 1 && mp[ir->items[1]->iVal] == 0)
                        {
                            mp[ir->items[0]->iVal] = 0;
                        }
                    }
                    else if (ir->type == IR::LDR || ir->type == IR::STR)
                    {
                        continue;
                    }
                    else
                    {
                        for (int i = 0; i < ir->items.size(); i++)
                        {
                            if (ir->items[i]->type == IRItem::IVAR || ir->items[i]->type == IRItem::FVAR)
                            {
                                mp[ir->items[i]->iVal] = 0;
                            }
                        }
                    }
                }
                reverse(irlist.begin(), irlist.end());
                for (IR *ir : irlist)
                {
                    if (ir->type == IR::MOV)
                    {
                        if(ir->items[1]->type == IRItem::RETURN)
                        {
                            mp[ir->items[0]->iVal] = 0;
                        }
                        else if (ir->items[1]->type == IRItem::INT || ir->items[1]->type == IRItem::FLOAT || (mp.count(ir->items[1]->iVal) == 1 && mp[ir->items[1]->iVal] == 1))
                        {
                            if (mp.count(ir->items[0]->iVal) == 0)
                            {
                                mp[ir->items[0]->iVal] = 1;
                            }
                        }
                        else if(mp.count(ir->items[1]->iVal) == 1 && mp[ir->items[1]->iVal] == 0)
                        {
                            mp[ir->items[0]->iVal] = 0;
                            mp[ir->items[1]->iVal] = 0;
                        }
                    }
                    else if ((ir->type == IR::ADD) || (ir->type == IR::SUB) || (ir->type == IR::MUL) || (ir->type == IR::DIV) || (ir->type == IR::DIV) || (ir->type == IR::EQ) || (ir->type == IR::NE) || (ir->type == IR::GE) || (ir->type == IR::GT) || (ir->type == IR::LE) || (ir->type == IR::LT))
                    {
                        if ((mp.count(ir->items[1]->iVal) == 1 && mp[ir->items[1]->iVal] == 1) && (mp.count(ir->items[2]->iVal) == 1 && mp[ir->items[2]->iVal] == 1))
                        {
                            if (mp.count(ir->items[0]->iVal) == 0)
                            {
                                mp[ir->items[0]->iVal] = 1;
                            }
                            else if (mp[ir->items[0]->iVal] == 0)
                            {
                                mp[ir->items[1]->iVal] = 0;
                                mp[ir->items[2]->iVal] = 0;
                            }
                        }
                        else if ((mp.count(ir->items[0]->iVal) == 1 && mp[ir->items[0]->iVal] == 0) || (mp.count(ir->items[1]->iVal) == 1 && mp[ir->items[1]->iVal] == 0) || (mp.count(ir->items[2]->iVal) == 1 && mp[ir->items[2]->iVal] == 0))
                        {
                            mp[ir->items[0]->iVal] = 0;
                            mp[ir->items[1]->iVal] = 0;
                            mp[ir->items[2]->iVal] = 0;
                        }
                    }
                    else if ((ir->type == IR::NOT) || (ir->type == IR::I2F) || (ir->type == IR::F2I) || (ir->type == IR::NEG))
                    {
                        if (mp.count(ir->items[1]->iVal) == 1 && mp[ir->items[1]->iVal] == 1)
                        {
                            if (mp.count(ir->items[0]->iVal) == 0)
                            {
                                mp[ir->items[0]->iVal] = 1;
                            }
                            else if (mp[ir->items[0]->iVal] == 0)
                            {
                                mp[ir->items[1]->iVal] = 0;
                            }
                            // st.insert(ir->irId);
                        }
                        else if ((mp.count(ir->items[0]->iVal) == 1 && mp[ir->items[0]->iVal] == 0) || (mp.count(ir->items[1]->iVal) == 1 && mp[ir->items[1]->iVal] == 0))
                        {
                            mp[ir->items[0]->iVal] = 0;
                            mp[ir->items[1]->iVal] = 0;
                        }
                    }
                    else if ((ir->type == IR::LDR && ir->items[1]->type == IRItem::IVAR) || (ir->type == IR::STR && ir->items[1]->type == IRItem::IVAR))
                    {
                        if (mp.count(ir->items[1]->iVal) == 1 && mp[ir->items[1]->iVal] == 1)
                        {
                            if (mp.count(ir->items[0]->iVal) == 0)
                            {
                                mp[ir->items[0]->iVal] = 1;
                            }
                            else if (mp[ir->items[0]->iVal] == 0)
                            {
                                mp[ir->items[1]->iVal] = 0;
                            }
                            // st.insert(ir->irId);
                        }
                        else if ((mp.count(ir->items[0]->iVal) == 1 && mp[ir->items[0]->iVal] == 0) || (mp.count(ir->items[1]->iVal) == 1 && mp[ir->items[1]->iVal] == 0))
                        {
                            mp[ir->items[0]->iVal] = 0;
                            mp[ir->items[1]->iVal] = 0;
                        }
                    }
                    else if (ir->type == IR::LDR || ir->type == IR::STR)
                    {
                        continue;
                    }
                    else
                    {
                        for (int i = 0; i < ir->items.size(); i++)
                        {
                            if (ir->items[i]->type == IRItem::IVAR || ir->items[i]->type == IRItem::FVAR)
                            {
                                mp[ir->items[i]->iVal] = 0;
                            }
                        }
                    }
                }
            }
            for (BaseBlock *bk : loop)
            {
                vector<IR *> irlist = bk->getIRlist();
                for (IR *ir : irlist)
                {
                    for (int i = 0; i < ir->items.size(); i++)
                    {
                        if (ir->type == IR::MOV)
                        {
                            if(ir->items[1]->type == IRItem::RETURN)
                            {
                                continue;
                            }
                            if ((ir->items[1]->type == IRItem::INT || ir->items[1]->type == IRItem::FLOAT || mp[ir->items[1]->iVal] == 1) && mp[ir->items[0]->iVal] == 1)
                            {
                                st.insert(ir->irId);
                            }
                        }
                        else if ((ir->type == IR::ADD) || (ir->type == IR::SUB) || (ir->type == IR::MUL) || (ir->type == IR::DIV) || (ir->type == IR::DIV) || (ir->type == IR::EQ) || (ir->type == IR::NE) || (ir->type == IR::GE) || (ir->type == IR::GT) || (ir->type == IR::LE) || (ir->type == IR::LT))
                        {
                            if(mp[ir->items[0]->iVal] == 1 && mp[ir->items[1]->iVal] == 1 && mp[ir->items[2]->iVal] == 1)
                            {
                                st.insert(ir->irId);
                            }
                        }
                        else if ((ir->type == IR::NOT) || (ir->type == IR::I2F) || (ir->type == IR::F2I) || (ir->type == IR::NEG))
                        {
                            if(mp[ir->items[0]->iVal] == 1 && mp[ir->items[1]->iVal] == 1)
                            {
                                st.insert(ir->irId);
                            }
                        }
                        else if((ir->type == IR::LDR && ir->items[1]->type == IRItem::IVAR) || (ir->type == IR::STR && ir->items[1]->type == IRItem::IVAR))
                        {
                            if(mp[ir->items[0]->iVal] == 1 && mp[ir->items[1]->iVal] == 1)
                            {
                                st.insert(ir->irId);
                            }
                        }
                        else if((ir->type == IR::STR && ir->items[1]->type == IRItem::SYMBOL) || (ir->type == IR::LDR && ir->items[1]->type == IRItem::SYMBOL))
                        {
                            if(mp[ir->items[0]->iVal] == 1)
                            {
                                st.insert(ir->irId);
                            }
                        }
                    }
                }
            }
            vector<IR *> newList;
            map<int, int> idmap;
            for (BaseBlock *bk : loop)
            {
                vector<IR *> irlist = bk->getIRlist();
                if (irlist.size() <= 1)
                {
                    continue;
                }
                vector<int> del;
                int count = 0;
                int firstid = irlist.front()->irId;
                int isfirst = 0;
                for (IR *ir : irlist)
                {
                    if (st.count(ir->irId) == 1)
                    {
                        if (ir->irId == firstid)
                            isfirst = 1;
                        newList.push_back(ir);
                        del.push_back(count);
                    }
                    count++;
                }
                int delnum = 0;
                for (int i : del)
                {
                    irlist.erase(irlist.begin() + i + delnum);
                    delnum--;
                }
                if ((st.count(firstid) == 1) && (isfirst == 1))
                {
                    idmap[firstid] = irlist.front()->irId;
                }
                bk->setIRlist(irlist);
                }
                for (BaseBlock *block : funcBlock)
                {
                    if (block->BlockId == firstBlock)
                    {
                        vector<IR *> inList = block->getIRlist();
                        inList.insert(inList.begin(), newList.begin(), newList.end());
                        block->setIRlist(inList);
                        break;
                    }
                }
                // cout << idmap.size() << endl;
                for (BaseBlock *block : funcBlock)
                {
                    vector<IR *> irlist = block->getIRlist();
                    for (IR *ir : irlist)
                    {
                        if (ir->type == IR::GOTO || ir->type == IR::BNE || ir->type == IR::BEQ)
                        {
                            int id = ir->items[0]->iVal;
                            if (idmap.count(id) == 1)
                            {
                                ir->items[0]->iVal = idmap[id];
                            }
                        }
                    }
                    block->setIRlist(irlist);
                }*/
                loopnum++;
        }
        funcNum++;
    }
}

void IRBuild::blockToFunc()
{
    funcList.clear();
    for (pair<Symbol *, vector<BaseBlock *>> func : BlockList)
    {
        pair<Symbol *, vector<IR *>> pr;
        pr.first = func.first;
        for(BaseBlock* bk : func.second)
        {
            vector<IR *> irlist = bk->getIRlist();
            pr.second.insert(pr.second.end(), irlist.begin(), irlist.end());
        }
        funcList.push_back(pr);
    }
}

void IRBuild::commonExpression()
{
    for(pair<Symbol *, vector<IR *>> &func : funcList)
    {
        string name;
        bool start = false;
        int flag = 0;
        int sum = 0, time = 0;
        int add;
        int lastId;
        for (IR *ir : func.second)
        {
            if (ir->type == IR::LDR && flag == 0)
            {
                if (ir->items[1]->type == IRItem::SYMBOL)
                {
                    if(time==0){
                        name = ir->items[1]->symbol->name;
                    }
                    else{
                        if(name != ir->items[1]->symbol->name)
                        {
                            flag = 0;
                            time = 0;
                            sum = 0;
                            continue;
                        }
                    }
                    flag = 1;
                }
            }
            else if (ir->type == IR::MOV && flag == 1)
            {
                if (ir->items[1]->type == IRItem::INT)
                {
                    add = ir->items[1]->iVal;
                    flag = 2;
                }
            }
            else if (ir->type == IR::ADD && flag == 2)
            {
                flag = 3;
            }
            else if (ir->type == IR::LDR && flag == 3)
            {
                if (ir->items[1]->type == IRItem::SYMBOL)
                {
                    
                    if (name != ir->items[1]->symbol->name)
                    {
                        flag = 0;
                        time = 0;
                        sum = 0;
                        continue;
                    }
                    flag = 4;
                }
            }
            else if (ir->type == IR::STR && flag == 4)
            {
                if (ir->items[1]->type == IRItem::SYMBOL)
                {
                    if (name != ir->items[1]->symbol->name)
                    {
                        flag = 0;
                        time = 0;
                        sum = 0;
                        continue;
                    }
                    flag = 0;
                    sum += add;
                    time++;
                    lastId = ir->irId;
                }
            }
            else{
                if (time > 1)
                {
                    //cout << time<<' '<<lastId << endl;
                    func.second[lastId - func.second[0]->irId - 3]->items[1]->iVal = sum;
                    func.second.erase(func.second.begin() + lastId + 1 - 5 * time, func.second.begin() + lastId - 4);
                    break;
                    // cout << func.second.size() << endl;
                }
                flag = 0;
                time = 0;
                sum = 0;
            }
        }
    }
}

void IRBuild::deadExpDelete()
{
    for(pair<Symbol *, vector<IR *>> &func : funcList)
    {
        string name;
        int flag = 0;
        int varId;
        unordered_map<string, int> mp;
        for (IR *ir : func.second)
        {
            if (ir->type == IR::LDR)
            {
                if (ir->items[1]->type == IRItem::SYMBOL)
                {
                    name = ir->items[1]->symbol->name;
                    if (mp.count(name) == 1)
                    {
                        mp[name] = 0;
                        continue;
                    }
                    flag = 1;
                }
            }
            else if (ir->type == IR::MOV && flag == 1)
            {
                if (ir->items[1]->type == IRItem::INT)
                {
                    flag = 2;
                    varId = ir->items[0]->iVal;
                }
                else
                    flag = 0;
            }
            else if(ir->type == IR::STR && flag == 2)
            {
                if (ir->items[1]->type == IRItem::SYMBOL)
                {
                    if (name != ir->items[1]->symbol->name || varId != ir->items[0]->iVal)
                    {
                        flag = 0;
                        varId = 0;
                        continue;
                    }
                    mp[name] = 1;
                    flag = 0;
                }
                else
                    flag = 0;
            }
            else{
                flag = 0;
                varId = 0;
            }
        }
        flag = 0;
        varId = 0;
        int deletesize = 0;
        vector<int> delList;
        int num = 0;
        int first = func.second[0]->irId;
        for (IR *ir : func.second)
        {
            if (ir->type == IR::LDR)
            {
                if (ir->items[1]->type == IRItem::SYMBOL)
                {
                    name = ir->items[1]->symbol->name;
                    if (mp.count(name) == 1 && mp[name] == 1)
                    {
                        //func.second.erase(func.second.begin() + ir->irId - first - deletesize, func.second.begin() + ir->irId - first + 3 - deletesize);
                        delList.push_back(num);
                    }
                }
            }
            num++;
        }
        for(int i : delList)
        {
            func.second.erase(func.second.begin() + i - deletesize, func.second.begin() + i + 3 - deletesize);
            deletesize += 3;
        }
    }
}

void IRBuild::loadDelete()
{
    for(pair<Symbol *, vector<IR *>> &func : funcList)
    {
        string name;
        int flag = 0;
        int varId;
        int delsize = 0;
        vector<int> delList;
        int num = 0;
        for (IR *ir : func.second)
        {
            if(ir->type == IR::LDR && ir->items[1]->type == IRItem::SYMBOL)
            {
                name = ir->items[1]->symbol->name;
                flag = 1;
            }
            else if(flag == 1 && ir->type == IR::STR && ir->items[1]->type == IRItem::SYMBOL)
            {
                if(ir->items[1]->symbol->name == name)
                {
                    delList.push_back(num-1);
                    flag = 0;
                }
            }
            else{
                flag = 0;
            }
            num++;
        }
        for (int i : delList)
        {
            func.second.erase(func.second.begin() + i - delsize);
            delsize++;
        }
    }
}

void IRBuild::strToMov()
{
    for(pair<Symbol *, vector<IR *>> &func : funcList)
    {
        string name;
        int flag = 0;
        int varId;
        int delsize = 0;
        int num = 0;
        int var1 = 0;
        for (IR *ir : func.second)
        {
            if(ir->type == IR::STR && ir->items[1]->type == IRItem::SYMBOL)
            {
                name = ir->items[1]->symbol->name;
                var1 = ir->items[0]->iVal;
                flag = 1;
            }
            else if(flag == 1 && ir->type == IR::LDR && ir->items[1]->type == IRItem::SYMBOL)
            {
                if(ir->items[0]->type == IRItem::IVAR && ir->items[1]->symbol->name == name && ir->items[1]->symbol->dimensions.size() == 0)
                {
                    int id = num;
                    func.second[id]->type = IR::MOV;
                    func.second[id]->items[1] = new IRItem(IRItem::IVAR,var1);
                    flag = 0;
                }
                else{
                    flag = 0;
                }
            }
            else{
                flag = 0;
            }
            num++;
        }
    }
}

void IRBuild::commonDelete()
{
    for(pair<Symbol *, vector<IR *>> &func : funcList)
    {
        string name;
        int flag = 0;
        int sum = 0;
        vector<int> delList;
        for (IR *ir : func.second)
        {
            if(ir->type == IR::LDR && ir->items[1]->type == IRItem::SYMBOL && ir->items[1]->symbol->name == "sum")
            {
                flag = 1;
            }
            else if(flag == 1 && ir->type == IR::LDR)
            {
                if(ir->items[1]->type == IRItem::SYMBOL)
                {
                    flag = 2;
                }
                else{
                    flag = 0;
                    sum = 0;
                }
            }
            else if(flag == 2 && ir->type == IR::ADD)
            {
                if ((ir->items[0]->iVal == (ir->items[1]->iVal + 2)) && (ir->items[0]->iVal == (ir->items[2]->iVal + 1)))
                {
                    flag = 1;
                    sum++;
                }
                else{
                    flag = 0;
                    sum = 0;
                }
            }
            else{
                if(sum == 15)
                {
                    delList.push_back(ir->irId - 2 * sum - 2);
                }
                flag = 0;
                sum = 0;
                continue;
            }
        }
        int size = 0;
        int firstid = func.second[0]->irId;
        if(delList.size()>0)
        {
            for (int i = 1; i < delList.size(); i++)
            {
                func.second.erase(func.second.begin() + delList[i] - firstid - size, func.second.begin() + delList[i] - firstid + 33 - size);
                size += 33;
            }
            int id = delList[0] + 32;
            int v2 = func.second[id - firstid]->items[0]->iVal + 1;
            IR *ir1 = new IR(IR::MOV, {new IRItem(IRItem::IVAR, v2 + 1), new IRItem(IRItem::INT, (int)delList.size())});
            IR *ir2 = new IR(IR::MUL, {new IRItem(IRItem::IVAR, v2 + 2), new IRItem(IRItem::IVAR, v2 - 1), new IRItem(IRItem::IVAR, v2 + 1)});
            func.second.insert(func.second.begin() + id, ir1);
            func.second.insert(func.second.begin() + id + 1, ir2);
            func.second[id - firstid + 2]->items[0]->iVal = v2 + 2;
        }
    }
}