#include <algorithm>
#include <iostream>
#include <queue>
#include <utility>

#include "IRBuild.h"

using namespace std;

IRBuild::IRBuild(parseNode *root, vector<Symbol *> &symbols) {
    this->isProcessed = false;
    this->varId = 0;
    this->root = root;
    this->symbols = symbols;
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
            if(pn->nodes[0]->symbol->symbolType == Symbol::GLOBAL_VAR)
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
            if(pn->nodes[0]->symbol->symbolType == Symbol::GLOBAL_VAR)
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
    vector<IR *> ir2 = parseTree(pn->nodes[1], sym, att);
    vector<IR *> ir1 = parseTree(pn->nodes[0], sym, att);
    ir.insert(ir.end(), ir2.begin(), ir2.end());
    ir.insert(ir.end(), ir1.begin(), ir1.end());
    if (pn->nodes[0]->nodes.size() == 0)
    {
        if(pn->nodes[0]->symbol->symbolType == Symbol::GLOBAL_VAR)
        {
            ir.push_back(new IR(IR::STR, {new IRItem(ir2.back()->items[0]->type, ir2.back()->items[0]->iVal), new IRItem(ir1[ir1.size()-2]->items[0]->type, ir1[ir1.size()-2]->items[0]->iVal)}));
        }
        else
            ir.push_back(new IR(IR::STR, {new IRItem(ir2.back()->items[0]->type, ir2.back()->items[0]->iVal), new IRItem(IRItem::SYMBOL, pn->nodes[0]->symbol)}));
    }
    else
        ir.push_back(new IR(IR::STR, {new IRItem(ir2.back()->items[0]->type, ir2.back()->items[0]->iVal), new IRItem(ir1[ir1.size() - 2]->items[0]->type, ir1[ir1.size() - 2]->items[0]->iVal)}));
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
        if (pn->symbol->symbolType == Symbol::GLOBAL_VAR)
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
        if (pn->symbol->symbolType == Symbol::GLOBAL_VAR)
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
        ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::INT, 0)}));
        nowid = varId;
        for (int i = pn->nodes.size() - 1; i >= 0; i--)
        {
            if (pn->nodes[i]->parseType == parseNode::INT_LITERAL) // a[i-1][3]
            {
                arrayOffset += pn->nodes[i]->iVal * d;
                //cout << arrayOffset << endl;
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
        //cout << varId << ' ' << nowid << endl;
        ir.push_back(new IR(IR::ADD, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::IVAR, nowid), new IRItem(IRItem::INT, arrayOffset * 4)}));
        int newid = varId;
        if (pn->nodes.size() < pn->symbol->dimensions.size())
        {
            int offset = 1;
            for (int i = pn->nodes.size(); i < pn->symbol->dimensions.size(); i++)
            {
                offset *= pn->symbol->dimensions[i];
            }
            ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, ++varId), new IRItem(IRItem::INT, offset * 4)}));
            ir.push_back(new IR(IR::MUL, {new IRItem(IRItem::IVAR, varId-1), new IRItem(IRItem::IVAR, varId-1), new IRItem(IRItem::IVAR, varId)}));
            newid = varId - 1;
        }
        ir.push_back(new IR(IR::ARR, {new IRItem(IRItem::IVAR, nameid), new IRItem(IRItem::IVAR, newid)}));
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
            new IR(IR::BNE, {new IRItem(IRItem::IR_ID, ir1.back()->irId + 1 + (int)irtest.size() + 4),
                             new IRItem(ir1.back()->items[0]->type, t1),
                             ir1.back()->items[0]->type == IRItem::IVAR
                             ? new IRItem(IRItem::INT, 0)
                             : new IRItem(IRItem::FLOAT, 0)}));
    vector<IR *> ir2 = parseTree(pn->nodes[1], sym, att);
    int t2 = ir2.back()->items[0]->iVal;
    ir.insert(ir.end(), ir2.begin(), ir2.end());
    ir.push_back(new IR(IR::BNE, {new IRItem(IRItem::IR_ID, ir2.back()->irId + 1 + 3),
                                  new IRItem(ir2.back()->items[0]->type, t2),
                                  ir2.back()->items[0]->type == IRItem::IVAR
                                  ? new IRItem(IRItem::INT, 0)
                                  : new IRItem(IRItem::FLOAT, 0)}));
    ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, ++varId),
                                  new IRItem(IRItem::INT, 0)}));
    ir.push_back(new IR(IR::GOTO, {new IRItem(IRItem::IR_ID, ir2.back()->irId + 5)}));
    ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, varId),
                                  new IRItem(IRItem::INT, 1)}));
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
            new IR(IR::BEQ, {new IRItem(IRItem::IR_ID, ir1.back()->irId + 1 + (int)irtest.size() + 2),
                             new IRItem(ir1.back()->items[0]->type, t1),
                             ir1.back()->items[0]->type == IRItem::IVAR
                             ? new IRItem(IRItem::INT, 0)
                             : new IRItem(IRItem::FLOAT, 0)}));
    vector<IR *> ir2 = parseTree(pn->nodes[1], sym, att);
    int t2 = ir2.back()->items[0]->iVal;
    ir.insert(ir.end(), ir2.begin(), ir2.end());
    ir.push_back(new IR(IR::BNE, {new IRItem(IRItem::IR_ID, ir2.back()->irId + 4),
                                  new IRItem(ir2.back()->items[0]->type, t2),
                                  ir2.back()->items[0]->type == IRItem::IVAR
                                  ? new IRItem(IRItem::INT, 0)
                                  : new IRItem(IRItem::FLOAT, 0)}));
    ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, ++varId),
                                  new IRItem(IRItem::INT, 0)}));
    ir.push_back(new IR(IR::GOTO, {new IRItem(IRItem::IR_ID, ir2.back()->irId + 5)}));
    ir.push_back(new IR(IR::MOV, {new IRItem(IRItem::IVAR, varId),
                                  new IRItem(IRItem::INT, 1)}));
    return ir;
}

vector<IR*> IRBuild::parseAlgoExp(parseNode * pn, Symbol * sym,Attribute * att)
{
    vector<IR *> ir1 = parseTree(pn->nodes[0], sym, att);
    vector<IR *> ir2 = parseTree(pn->nodes[1], sym, att);
    vector<IR *> ir;
    ir.insert(ir.end(), ir1.begin(), ir1.end());
    ir.insert(ir.end(), ir2.begin(), ir2.end());
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
    ir.push_back(new IR(type, {new IRItem(IRItem::IVAR, varId++), new IRItem(ir1.back()->items[0]->type, ir1.back()->items[0]->iVal), new IRItem(ir2.back()->items[0]->type, ir2.back()->items[0]->iVal)}));
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
    vector<IR *> ir1 = parseTree(pn->nodes[0], sym, att);
    IR *beqIr = new IR(IR::BEQ, {new IRItem(IRItem::IR_ID, 0), new IRItem(ir1.back()->items[0]->type, ir1.back()->items[0]->iVal), ir1.back()->items[0]->type == IRItem::IVAR ? new IRItem(IRItem::INT, 0) : new IRItem(IRItem::FLOAT, 0)});
    att->continueLabel = ir1[0]->irId;
    vector<IR *> irtest = parseTree(pn->nodes[1], sym, att);
    att->breakLabel = irtest.back()->irId + 2;
    ir1[0]->deleteIr(irtest.size());
    vector<IR *> ir2 = parseTree(pn->nodes[1], sym, att);
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
            ir.push_back(new IR(
                    IR::F2I, {new IRItem(IRItem::IVAR, ++varId),
                              new IRItem(IRItem::FVAR, ir.back()->items[0]->iVal)}));
            break;
        case parseNode::I2F:
            ir.push_back(new IR(
                    IR::I2F, {new IRItem(IRItem::FVAR, ++varId),
                              new IRItem(IRItem::IVAR, ir.back()->items[0]->iVal)}));
            break;
        case parseNode::NOT:
            ir.push_back(new IR(IR::NOT, {new IRItem(IRItem::IVAR, ++varId),
                                          new IRItem(ir.back()->items[0]->type,
                                                     ir.back()->items[0]->iVal)}));
            break;
        case parseNode::NEG:
            ir.push_back(new IR(
                    IR::NEG,
                    {new IRItem(ir.back()->items[0]->type, ++varId),
                     new IRItem(ir.back()->items[0]->type, ir.back()->items[0]->iVal)}));
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
    // getch
    func = new Symbol(Symbol::FUNC, Symbol::INT, "getch", vector<Symbol *>());
    symbols.push_back(func);
    // getarray
    param1 = new Symbol(Symbol::PARAM, Symbol::INT, "a", vector<int>{-1});
    func = new Symbol(Symbol::FUNC, Symbol::INT, "getarray",
                      vector<Symbol *>{param1});
    symbols.push_back(func);
    // getfloat
    func =
            new Symbol(Symbol::FUNC, Symbol::FLOAT, "getfloat", vector<Symbol *>());
    symbols.push_back(func);
    // getfarray
    param1 = new Symbol(Symbol::PARAM, Symbol::FLOAT, "a", vector<int>{-1});
    func = new Symbol(Symbol::FUNC, Symbol::INT, "getfarray",
                      vector<Symbol *>{param1});
    symbols.push_back(func);
    // putint
    param1 = new Symbol(Symbol::PARAM, Symbol::INT, "a", vector<int>());
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "putint",
                      vector<Symbol *>{param1});
    symbols.push_back(func);
    // putch
    param1 = new Symbol(Symbol::PARAM, Symbol::INT, "a", vector<int>());
    func =
            new Symbol(Symbol::FUNC, Symbol::VOID, "putch", vector<Symbol *>{param1});
    symbols.push_back(func);
    // putarray
    param1 = new Symbol(Symbol::PARAM, Symbol::INT, "n", vector<int>());
    param2 = new Symbol(Symbol::PARAM, Symbol::INT, "a", vector<int>{-1});
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "putarray", vector<Symbol *>{param1, param2});
    symbols.push_back(func);
    // putfloat
    param1 = new Symbol(Symbol::PARAM, Symbol::FLOAT, "a", vector<int>());
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "putfloat", vector<Symbol *>{param1});
    symbols.push_back(func);
    // putfarray
    param1 = new Symbol(Symbol::PARAM, Symbol::INT, "n", vector<int>());
    param2 = new Symbol(Symbol::PARAM, Symbol::FLOAT, "a", vector<int>{-1});
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "putfarray", vector<Symbol *>{param1, param2});
    symbols.push_back(func);

    param1 = new Symbol(Symbol::PARAM, Symbol::STRING, "format");
    param2 = new Symbol(Symbol::PARAMLIST, Symbol::LIST, "paramList", vector<Symbol *>());
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "putf", vector<Symbol *>{param1, param2});
    symbols.push_back(func);
    // _sysy_starttime
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "_sysy_starttime", vector<Symbol *>());
    symbols.push_back(func);
    // _sysy_stoptime
    func = new Symbol(Symbol::FUNC, Symbol::VOID, "_sysy_stoptime", vector<Symbol *>());
    symbols.push_back(func);
    return symbols;
}

void IRBuild::printIRs(bool silentMode) {
    //cout << 1 << endl;
    parseRoot(root);
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
