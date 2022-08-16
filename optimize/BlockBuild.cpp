#include <algorithm>
#include <iostream>
#include <queue>
#include <utility>
#include <map>

#include "BlockBuild.h"

using namespace std;

vector<BaseBlock *> BlockBuild::generateFunctionBlock(vector<IR *> IRList){
    vector<BaseBlock *> blocklist;
    vector<int> blockbegin;
    vector<int> blockNum;
    map<int, int> blockmp;
    int blockNode = 0;
    IR *irtemp;
    blockbegin.push_back(IRList[0]->irId);
    int irnum = 0;
    for (IR *ir : IRList)
    {
        if (ir->type == IR::BEQ || ir->type == IR::BNE)
        {
            blockbegin.push_back((irnum + 1) >= IRList.size() ? ir->irId + 1 : IRList[irnum + 1]->irId);
            if (ir->items[0]->iVal <= IRList.back()->irId)
                blockbegin.push_back(ir->items.at(0)->iVal);
        }
        else if(ir->type==IR::GOTO){
            if (IRList[irnum - 1]->type == IR::RETURN)
            {
                blockbegin.push_back((irnum + 1) >= IRList.size() ? ir->irId + 1 : IRList[irnum + 1]->irId);
            }
            else{
                blockbegin.push_back((irnum + 1) >= IRList.size() ? ir->irId + 1 : IRList[irnum + 1]->irId);
                blockbegin.push_back(ir->items.at(0)->iVal);
            }
        }
        irnum++;
    }
    sort(blockbegin.begin(),blockbegin.end());
    blockbegin.erase(unique(blockbegin.begin(),blockbegin.end()),blockbegin.end());
    blockbegin.push_back(IRList.back()->irId+1);
    int firstBlockNode = blockNode;
    for (int i = 0; i < blockbegin.size() - 1; i++)
    {
        vector<IR*> bbir;
        BaseBlock* bb = new BaseBlock(blockNode++);
        blockmp[blockbegin[i]] = i;
        int flag = 0;
        for (IR *ir : IRList)
        {
            if(ir->irId == blockbegin[i])
            {
                flag = 1;
            }
            else if(ir->irId >= blockbegin[i+1])
            {
                break;
            }
            if(flag == 1)
            {
                bbir.push_back(ir);
                blockNum.push_back(blockNode-1);
            }
        }
        bb->setIRlist(bbir);
        blocklist.push_back(bb);
    }
    blockmp[blockbegin[blockbegin.size() - 1]] = -1;
    int blocknum = 0;
    for (BaseBlock *bb : blocklist)
    {
        irtemp = bb->getLastIR();
        if(irtemp == nullptr) break;
        if (irtemp->type == IR::BEQ || irtemp->type == IR::BNE)
        {
            int jumpId = irtemp->items.at(0)->iVal;
            if (blockmp[jumpId] != -1)
            {
                bb->insertFlowOut(blocklist[blockmp[jumpId]]);
                blocklist[blockmp[jumpId]]->insertFlowIn(bb);
            }
            int nextId = (blocknum + 1) >= blocklist.size() ? irtemp->irId+1 : blocklist[blocknum + 1]->getFirstIR()->irId;
            if(blockmp[nextId] == -1)
                break;
            bb->insertFlowOut(blocklist[blockmp[nextId]]);
            blocklist[blockmp[nextId]]->insertFlowIn(bb);
        }
        else if(irtemp->type==IR::GOTO){
            vector<IR *> irlist = bb->getIRlist();
            if (irlist.size()>1 && irlist[irlist.size() - 2]->type != IR::RETURN)
            {
                int jumpId = irtemp->items.at(0)->iVal;
                bb->insertFlowOut(blocklist[blockmp[jumpId]]);
                blocklist[blockmp[jumpId]]->insertFlowIn(bb);
            }
            else if(irlist.size()==1)
            {
                int jumpId = irtemp->items.at(0)->iVal;
                bb->insertFlowOut(blocklist[blockmp[jumpId]]);
                blocklist[blockmp[jumpId]]->insertFlowIn(bb);
            }
        }
        else{
            int nextId = (blocknum + 1) >= blocklist.size() ? irtemp->irId+1 : blocklist[blocknum + 1]->getFirstIR()->irId;
            if(blockmp[nextId] == -1)
                break;
            bb->insertFlowOut(blocklist[blockmp[nextId]]);
            blocklist[blockmp[nextId]]->insertFlowIn(bb);
        }
        blocknum++;
    }
    for (BaseBlock *bk : blocklist)
    {
        if(bk->BlockId == 0)
        {
            bk->addDom(bk);
        }
        else{
            for(BaseBlock* b : blocklist)
            {
                bk->addDom(b);
            }
        }
    }
    for (int i = 1; i <= blocklist.size();i++)
    {
        for (int j = 0; j < blocklist.size(); j++)
        {
            vector<BaseBlock *> in = blocklist[j]->getBlockIn();
            vector<BaseBlock *> dom;
            map<int,int> mp;
            for (int k = 0; k < in.size(); k++)
            {
                set<BaseBlock *> dm = blocklist[in[k]->BlockId]->getDoms();
                //cout << in[k]->BlockId << ':' << dm.size() << endl;
                for (BaseBlock *bk : dm)
                {
                    mp[bk->BlockId]++;
                    //cout << bk->BlockId << endl;
                }
            }
            for(pair<int,int> p : mp)
            {
                if(p.second == in.size())
                {
                    //cout << p.first << endl;
                    dom.push_back(blocklist[p.first]);
                }
            }
            blocklist[j]->clearDom();
            for (BaseBlock *bk : dom)
            {
                blocklist[j]->addDom(bk);
            }
            blocklist[j]->addDom(blocklist[j]);
        }
    }

    for (int i = 0; i < blocklist.size(); i++)
    {
        set<BaseBlock *> dm = blocklist[i]->getDoms();
        for(BaseBlock* flow : dm)
        {
            vector<BaseBlock *> outList = flow->getBlockIn();
            for (BaseBlock *out : outList)
            {
                if (out->BlockId == i)
                {
                    backEdge.push_back(make_pair(out,flow));
                }
            }
        }
    }
    for(pair<BaseBlock*,BaseBlock*> edge : backEdge)
    {
        set<BaseBlock *> st;
        st.insert(edge.first);
        st.insert(edge.second);
        vector<BaseBlock *> addlist;
        vector<BaseBlock *> newList;
        int ssize = 0;
        addlist.push_back(edge.first);
        while (st.size()!=ssize)
        {
            ssize = st.size();
            newList.clear();
            for (BaseBlock *add : addlist)
            {
                vector<BaseBlock *> father = add->getBlockIn();
                int num = st.size();
                for (BaseBlock *bk : father)
                {
                    st.insert(bk);
                    if (st.size() > num)
                    {
                        newList.push_back(bk);
                        num++;
                    }
                }
            }
            addlist.clear();
            addlist.insert(addlist.begin(), newList.begin(), newList.end());
        }
        loop.push_back(st);
    }
    return blocklist;
}