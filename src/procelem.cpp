/**  
 * @file    procelem.cpp
 * @brief   Processing Element Implementation
 * @author  Chenu Tang
 * @version 4.0
 * @date    2022-02-09
 * @note    
 */


#include "procelem.h"
#include <string.h>



StageProcessors::StageProcessors()
{
    pnum = 0;
    PE = NULL;
    loadlatency = NULL;
    oplatency = NULL;
    midlatency = NULL;
    storelatency = NULL;
    loadlatency = NULL;
    storelatency = NULL;
    next = NULL;
    prior = NULL;
}

StageProcessors::~StageProcessors()
{
    if (PE) {
        delete[] PE;
    }
    if (loadlatency) {
        delete[] loadlatency;
    }
    if (oplatency) {
        delete[] oplatency;
    }
    if (midlatency) {
        delete[] midlatency;
    }
    if (storelatency) {
        delete[] storelatency;
    }
    if (loadenergy) {
        delete[] loadenergy;
    }
    if (midenergy) {
        delete[] midenergy;
    }
    if (storeenergy) {
        delete[] storeenergy;
    }
    inst.clear();
}

int StageProcessors::init(uint pn)
{
    if (PE) {
        delete[] PE;
    }
    pnum = pn;
    schedule.clear();
    PE = new ProcessElem[pnum];
    for (uint i = 0; i < pn; ++i) {
        PE[i].id = i;
    }
    loadlatency = new bigint[pn];
    oplatency = new bigint[pn];
    midlatency = new bigint[pn];
    storelatency = new bigint[pn];
    loadenergy = new double[pn];
    midenergy = new double[pn];
    storeenergy = new double[pn];
    memset((void*)(loadlatency), 0, pnum*sizeof(bigint));
    memset((void*)(oplatency), 0, pnum*sizeof(bigint));
    memset((void*)(midlatency), 0, pnum*sizeof(bigint));
    memset((void*)(storelatency), 0, pnum*sizeof(bigint));
    next = NULL;
    prior = NULL;
    return 1;
}

int StageProcessors::clean()
{
    if (PE) {
        delete[] PE;
    }
    if (loadlatency) {
        delete[] loadlatency;
    }
    if (oplatency) {
        delete[] oplatency;
    }
    if (midlatency) {
        delete[] midlatency;
    }
    if (storelatency) {
        delete[] storelatency;
    }
    if (loadenergy) {
        delete[] loadenergy;
    }
    if (midenergy) {
        delete[] midenergy;
    }
    if (storeenergy) {
        delete[] storeenergy;
    }
    pnum = 0;
    schedule.clear();
    PE = NULL;
    loadlatency = NULL;
    oplatency = NULL;
    midlatency = NULL;
    storelatency = NULL;
    loadlatency = NULL;
    storelatency = NULL;
    next = NULL;
    prior = NULL;
    inst.clear();
    return 1;
}

inline InstructionNameSpace::InstructionType Vtype2Instop(VerticeType vt)
{
    switch (vt) {
        case VINV: {
            return InstructionNameSpace::INV;
            break;
        }
        case VAND: {
            return InstructionNameSpace::AND;
            break;
        }
        case VOR: {
            return InstructionNameSpace::OR;
            break;
        }
        case VXOR: {
            return InstructionNameSpace::XOR;
            break;
        }
        case VXOR3: {
            return InstructionNameSpace::XOR3;
            break;
        }
        case VMAJ: {
            return InstructionNameSpace::MAJ;
            break;
        }
        default: {
            return InstructionNameSpace::NOP;
            break;
        }
    }
}

int StageProcessors::checkPlaceable(BooleanDag *G, uint peid, uint taskid)
{
    Vertice *v = G->getvertice(taskid);
    uint prednum = v->prednum;
    ProcessElem* pe = PE + peid;

    int load = 0;
    int copy = 0;

    for (uint i = 0; i < prednum; ++i) {
        if (pe->cache.find(v->predecessors[i]->src->id) != pe->cache.end()) {
            ;
        }
        else if (getAssignmentByTask(taskid)) {
            copy++;
        }
        else {
            load++;
        }
    }
    if (pe->overwriteflag + load > BLOCKROW || pe->cache.size() + copy + load + 1 > BLOCKROW) {
        return 0;
    }
    return 1;
}

// TODO: Check conflict with existing assignment
int StageProcessors::assignTask(BooleanDag* G, uint taskid, uint PEid, bigint starttime, bigint finishtime)
{
    Assignment a;
    a.pid = PEid;
    a.tid = taskid;
    a.starttime = starttime;
    a.finishtime = finishtime;
    schedule.insert(std::make_pair(taskid, a));
    int rmvbound = G->getinputsize()+G->getoutputsize();

    ProcessElem *pe = PE+PEid;
    pe->opeft = pe->opeft > finishtime ? pe->opeft : finishtime;
    // TODO: Possible insertion in the future
    // for (std::vector<Assignment*>::iterator iter = pe->tasks.begin(); iter != pe->tasks.end(); ++iter) {
    //     if (starttime < (*iter)->starttime) {
    //         pe->tasks.insert(iter, &schedule[taskid]);
    //         return 1;
    //     }
    // }
    pe->tasks.push_back(&schedule[taskid]);
 

    Vertice *v = G->getvertice(taskid);
    uint prednum = v->prednum;
    InstructionNameSpace::Instruction opinst;
    opinst.taskid = taskid;
    opinst.op = Vtype2Instop(v->type);
    for (int i = 0; i < 3; ++i) {
        if (v->invflags&(1<<i)) {
            opinst.invflag[i] = true;
        }
    }
    for (uint i = 0u; i < prednum; ++i) {
        Edge *prede = v->predecessors[i];
        Vertice *predv = prede->src;
        uint predid = predv->id;
        Assignment* predassignment;
        if (pe->cache.find(predid) == pe->cache.end()) {
            // Did not find local data
            // need copy / load and assign new row
            predassignment = getAssignmentByTask(predid);
            if (predassignment) {
                // in mesh
                InstructionNameSpace::Instruction copyinst;
                uint srcpeid = predassignment->pid;
                uint srcidx = (PE+srcpeid)->cache[predid];
                opinst.src[i] = MESHADDR(PEid, pe->smallestfreeidx);
                pe->cache.insert(std::make_pair(v->predecessors[i]->src->id, pe->smallestfreeidx));
                pe->line[pe->smallestfreeidx] = v->predecessors[i]->src->id;
                copyinst.taskid = taskid;
                copyinst.op = InstructionNameSpace::COPY;
                copyinst.dest = MESHADDR(PEid, pe->smallestfreeidx);
                copyinst.src[0] = MESHADDR(srcpeid, srcidx);

                inst.push_back(copyinst);
                while (++(pe->smallestfreeidx) < BLOCKROW && pe->line[pe->smallestfreeidx] < UINT_MAX);  // next smallestfreeidx

                // std::deque<InstructionNameSpace::Instruction>::iterator it = inst.begin();
                // while (it != inst.end()) {
                //     if (it->taskid == predv->id && it->op >= InstructionNameSpace::INV) {
                //         break;  // find pred op position
                //     }
                //     ++it;
                // }
                // if (it == inst.end()) {
                //     exit(-1);
                // }
                // inst.insert(++it, copyinst);
            }
            else {
                // if (prior) {
                //     predassignment = prior->getAssignmentByTask(predid);
                // }
                // if (predassignment && prior->getLine(predid)>=getOverwritepos(predassignment->pid)) {
                //     uint srcpeid = predassignment->pid;
                //     uint srcidx = (prior->PE+srcpeid)->cache[predid];   // no need to use getLine
                //     // if (srcpeid == pe->id) {
                //     //     // in this pe, no need to copy
                //     //     opinst.src[i] = MESHADDR(PEid, srcidx);
                //     //     pe->cache.insert(std::make_pair(predassignment->tid, srcidx));
                //     //     pe->line[srcidx] = predassignment->tid;
                //     //     if (pe->smallestfreeidx == srcidx) {
                //     //         while (++(pe->smallestfreeidx) < BLOCKROW && pe->line[pe->smallestfreeidx] < UINT_MAX);  // next smallestfreeidx
                //     //     }
                //     // }
                //     // else {
                //         // need copy
                //     InstructionNameSpace::Instruction copyinst;
                //     opinst.src[i] = MESHADDR(PEid, pe->smallestfreeidx);
                    
                //     pe->cache.insert(std::make_pair(v->predecessors[i]->src->id, pe->smallestfreeidx));
                //     pe->line[pe->smallestfreeidx] = v->predecessors[i]->src->id;
                //     copyinst.taskid = taskid;
                //     copyinst.op = InstructionNameSpace::COPY;
                //     copyinst.dest = MESHADDR(PEid, pe->smallestfreeidx);
                //     copyinst.src[0] = MESHADDR(srcpeid, srcidx);
                //     inst.push_back(copyinst);
                //     while (++(pe->smallestfreeidx) < BLOCKROW && pe->line[pe->smallestfreeidx] < UINT_MAX);  // next smallestfreeidx
                //     // }

                //     // remove STORE
                //     bool rmv = true;
                //     for (uint k = 0; k < predv->succnum; ++k) {
                //         if (predv->successors[k]->dest->id != taskid && G->getPriority(predv->successors[k]->dest->id) <= G->getPriority(taskid)) {
                //             rmv = false;
                //         }
                //     }
                //     if (rmv && predv->id > rmvbound) {
                //         prior->removeStoreInst(predv->id);
                //     }
                // }
                // else {
                    InstructionNameSpace::Instruction loadinst;
                    opinst.src[i] = MESHADDR(PEid, pe->overwriteflag);
                    pe->cache.insert(std::make_pair(v->predecessors[i]->src->id, pe->overwriteflag));
                    pe->line[pe->overwriteflag] = v->predecessors[i]->src->id;
                    loadinst.taskid = taskid;
                    loadinst.op = InstructionNameSpace::LOAD;
                    loadinst.dest = MESHADDR(PEid,pe->overwriteflag);
                    loadinst.src[0] = MESHADDR(pnum, predid);   //TODO: ADD MEM ADDR

                    inst.push_front(loadinst);
                    if (pe->smallestfreeidx == pe->overwriteflag) {
                        ++(pe->smallestfreeidx);
                        pe->overwriteflag = pe->smallestfreeidx;
                    }
                    else {
                        ++(pe->overwriteflag);
                    }
                // }
                // in memory

                // std::deque<InstructionNameSpace::Instruction>::iterator it = inst.begin();
                // while (it != inst.end() && it->op != InstructionNameSpace::LOAD) ++it;
                // inst.insert(it, loadinst);
            }
            pe->overwriteflag = pe->overwriteflag > pe->smallestfreeidx ? pe->overwriteflag : pe->smallestfreeidx;
        }
        else {
            opinst.src[i] = MESHADDR(PEid, pe->cache[predid]);
        }
    }

    opinst.dest = MESHADDR(PEid, pe->smallestfreeidx);
    inst.push_back(opinst);
    pe->cache.insert(std::make_pair(taskid, pe->smallestfreeidx));
    pe->line[pe->smallestfreeidx] = taskid;
    while (++(pe->smallestfreeidx) < BLOCKROW && pe->line[pe->smallestfreeidx] < UINT_MAX);  // next smallestfreeidx
    pe->overwriteflag = pe->overwriteflag > pe->smallestfreeidx ? pe->overwriteflag : pe->smallestfreeidx;

    return 1;
}


typedef struct SyncNode {
    bigint synctime;
    InstructionNameSpace::Instruction inst;
    SyncNode *next;
    SyncNode() : synctime(0ll), next(NULL){};
    SyncNode(bigint t) : synctime(t), next(NULL){};
} SyncNode;

int cleanSyncNode(SyncNode *node)
{
    SyncNode *next = node;
    while (node) {
        next = node->next;
        delete node;
        node = next;
    }
    return 1;
}

int insertSyncNode(SyncNode *head, SyncNode *node)
{
    SyncNode *p = head;
    while (p->next && p->next->synctime <= node->synctime) {
        p = p->next;
    }
    if (p->next) {
        SyncNode *tmp = p->next;
        p->next = node;
        node->next = tmp;
    }
    else {
        p->next = node;
    }
    return 1;
}

int StageProcessors::dynamicWeightsAssignTask(BooleanDag* G, uint taskid, uint PEid)
{
    Assignment a;
    a.pid = PEid;
    a.tid = taskid;
    a.starttime = midlatency[PEid];
    // printf("[%d]\n", taskid);
    int rmvbound = G->getinputsize()+G->getoutputsize();
    ProcessElem *pe = PE+PEid;
    Vertice *v = G->getvertice(taskid);
    uint prednum = v->prednum;
    bigint curmaxpelat = midlatency[PEid];
    SyncNode *sync = new SyncNode;
    uint threads = MESHSIZE / pnum;
    InstructionNameSpace::Instruction opinst;
    opinst.taskid = taskid;
    opinst.op = Vtype2Instop(v->type);
    for (int i = 0; i < 3; ++i) {
        if (v->invflags&(1<<i)) {
            opinst.invflag[i] = true;
        }
    }
    for (uint i = 0u; i < prednum; ++i) {
        Edge *prede = v->predecessors[i];
        Vertice *predv = prede->src;
        uint predid = predv->id;
        Assignment* predassignment;
        bool newselfblk = false;
        ProcessElem *srcpe;
        uint srcpeid;
        bigint minblkt = BIGINT_MAX;

        if (pe->cache.find(predid) == pe->cache.end()) {
            // Did not find local data
            // need copy / load and assign new row
            for (uint k = 0; k < pnum; ++k) {
                srcpe = PE+k;
                if (k != PEid && srcpe->cache.find(predid)!=srcpe->cache.end()) {
                    ///< find this task in cache
                    if (midlatency[k] > midlatency[PEid] && midlatency[k] <= curmaxpelat) {
                        ///< do not need extra blk
                        newselfblk = false;
                        minblkt = 0;
                        srcpeid = srcpe->id;
                        break;
                    }
                    else if (midlatency[k] > midlatency[PEid]) {
                        ///< need more self blk
                        bigint dist = midlatency[k] - curmaxpelat;
                        if (dist < minblkt) {
                            newselfblk = true;
                            minblkt = dist;
                            srcpeid = srcpe->id;
                        }
                    }
                    else {
                        ///< need src pe's blk
                        bigint dist = midlatency[PEid] - midlatency[k];
                        if (dist < minblkt) {
                            newselfblk = false;
                            minblkt = dist;
                            srcpeid = srcpe->id;
                        }
                    }
                }
            }

            if (minblkt < BIGINT_MAX) {
                ///< need copy - update current max self blk time
                SyncNode *copyinst = new SyncNode;
                uint srcidx = (PE+srcpeid)->cache[predid];
                opinst.src[i] = MESHADDR(PEid, pe->smallestfreeidx);
                pe->cache.insert(std::make_pair(v->predecessors[i]->src->id, pe->smallestfreeidx));
                pe->line[pe->smallestfreeidx] = v->predecessors[i]->src->id;
                copyinst->inst.taskid = taskid;
                copyinst->inst.op = InstructionNameSpace::COPY;
                copyinst->inst.dest = MESHADDR(PEid, pe->smallestfreeidx);
                copyinst->inst.src[0] = MESHADDR(srcpeid, srcidx);
                copyinst->synctime = midlatency[srcpeid];
                insertSyncNode(sync, copyinst);
                while (++(pe->smallestfreeidx) < BLOCKROW && pe->line[pe->smallestfreeidx] < UINT_MAX);  // next smallestfreeidx
                
                if (newselfblk) {
                    curmaxpelat += minblkt;
                }
            }
            else {
                ///< need load
                InstructionNameSpace::Instruction loadinst;
                opinst.src[i] = MESHADDR(PEid, pe->overwriteflag);
                pe->cache.insert(std::make_pair(v->predecessors[i]->src->id, pe->overwriteflag));
                pe->line[pe->overwriteflag] = v->predecessors[i]->src->id;
                loadinst.taskid = taskid;
                loadinst.op = InstructionNameSpace::LOAD;
                loadinst.dest = MESHADDR(PEid,pe->overwriteflag);
                loadinst.src[0] = MESHADDR(pnum, predid);   //TODO: ADD MEM ADDR

                inst.push_front(loadinst);
                loadlatency[PEid] += LOADLATENCY * threads;
                // printf("    LOAD %d %d %d %d\n", loadinst.src[0], loadinst.src[1], loadinst.src[2], loadinst.dest);
                if (pe->smallestfreeidx == pe->overwriteflag) {
                    ++(pe->smallestfreeidx);
                    pe->overwriteflag = pe->smallestfreeidx;
                }
                else {
                    ++(pe->overwriteflag);
                }
            }
            pe->overwriteflag = pe->overwriteflag > pe->smallestfreeidx ? pe->overwriteflag : pe->smallestfreeidx;
        }
        else {
            opinst.src[i] = MESHADDR(PEid, pe->cache[predid]);
        }
    }
    SyncNode *tmp, *p = sync->next;
    uint p1, p2, level;
    while (p) {
        tmp = p->next;
        inst.push_back(p->inst);
        // printf("    COPY %d %d %d %d\n", p->inst.src[0], p->inst.src[1], p->inst.src[2], p->inst.dest);
        p1 = p->inst.src[0] / BLOCKROW;
        p2 = p->inst.dest / BLOCKROW;
        level = getCommLevel(pnum, p1, p2);
        bigint max = midlatency[p1] > midlatency[p2] ? midlatency[p1] : midlatency[p2];
        uint maxthreads = MaxCopyThread[level] > threads ? threads : MaxCopyThread[level];
        if (level > 0) {
            max += CommWeight[level] * (threads / maxthreads);
        }
        else {
            max += OPLATENCY;
        }
        midlatency[p1] = max;
        midlatency[p2] = max;
        delete p;
        p = tmp;
    }
    delete sync;
    opinst.dest = MESHADDR(PEid, pe->smallestfreeidx);
    inst.push_back(opinst);
    // printf("    OP %d %d %d %d\n", opinst.src[0], opinst.src[1],opinst.src[2], opinst.dest);
    pe->cache.insert(std::make_pair(taskid, pe->smallestfreeidx));
    pe->line[pe->smallestfreeidx] = taskid;
    midlatency[PEid] += OPLATENCY;
    oplatency[PEid] += OPLATENCY;
    while (++(pe->smallestfreeidx) < BLOCKROW && pe->line[pe->smallestfreeidx] < UINT_MAX);  // next smallestfreeidx
    pe->overwriteflag = pe->overwriteflag > pe->smallestfreeidx ? pe->overwriteflag : pe->smallestfreeidx;
    a.finishtime = midlatency[PEid];
    schedule.insert(std::make_pair(taskid, a));
    pe->tasks.push_back(&schedule[taskid]);
    return 1;
}

// int checkAllFinish;

int StageProcessors::releaseMem(BooleanDag* g, uint taskid, bigint *priority)
{
    Vertice *v = g->getvertice(taskid);
    uint prednum = v->prednum;
    bigint prio = priority[taskid];
    uint releasebound = g->getinputsize()+g->getoutputsize();
    for (uint i = 0u; i < prednum; ++i) {
        Vertice *pred = v->predecessors[i]->src;
        if (pred->id <= releasebound) {
            continue;
        }
        uint succnum = pred->succnum;
        bool releaseable = true;
        for (uint j = 0u; j < succnum; ++j) {
            // still have vertice not assigned
            if (pred->successors[j]->dest->id != taskid && priority[pred->successors[j]->dest->id] <= prio) {
                releaseable = false;
            }
        }
        if (releaseable) {
            for (uint j = 0u; j < pnum; ++j) {
                std::map<uint, uint>::iterator it = PE[j].cache.find(pred->id);
                if (it != PE[j].cache.end()) {
                    PE[j].line[it->second] = UINT_MAX;
                    if (it->second < PE[j].smallestfreeidx) {
                        PE[j].smallestfreeidx = it->second;
                    }
                    PE[j].cache.erase(it);
                }
            }
        }
    }
    return 1;
}


int StageProcessors::releaseMem(BooleanDag* g, uint taskid, bool *assigned)
{
    Vertice *v = g->getvertice(taskid);
    uint prednum = v->prednum;
    uint releasebound = g->getinputsize()+g->getoutputsize();
    for (uint i = 0u; i < prednum; ++i) {
        Vertice *pred = v->predecessors[i]->src;
        if (pred->id <= releasebound) {
            continue;
        }
        uint succnum = pred->succnum;
        bool releaseable = true;
        for (uint j = 0u; j < succnum; ++j) {
            // still have vertice not assigned
            if (assigned[pred->successors[j]->dest->id] == false) {
                releaseable = false;
            }
        }
        if (releaseable) {
            for (uint j = 0u; j < pnum; ++j) {
                std::map<uint, uint>::iterator it = PE[j].cache.find(pred->id);
                if (it != PE[j].cache.end()) {
                    PE[j].line[it->second] = UINT_MAX;
                    if (it->second < PE[j].smallestfreeidx) {
                        PE[j].smallestfreeidx = it->second;
                    }
                    PE[j].cache.erase(it);
                }
            }
        }
    }
    return 1;
}


int StageProcessors::assignFinish()
{
    ProcessElem* p;
    uint threads = MESHSIZE / pnum;
    for (uint i = 0u; i < pnum; ++i) {
        p = PE+i;
        std::vector<Assignment*>::iterator it;
        for (it = p->tasks.begin(); it != p->tasks.end(); ++it) {
            std::map<uint, uint>::iterator mp;
            mp = p->cache.find((*it)->tid);
            if (mp != p->cache.end()) {
                InstructionNameSpace::Instruction storeinst;
                storeinst.taskid = mp->first;
                storeinst.op = InstructionNameSpace::STORE;
                storeinst.src[0] = MESHADDR(i,mp->second);
                storeinst.dest = MESHADDR(pnum, mp->first);
                storelatency[storeinst.src[0]/BLOCKROW] += STORELATENCY * threads;
                inst.push_back(storeinst);
            }
        }
    }
    return 1;
}


int StageProcessors::calcLatency()
{
    memset((void*)(loadlatency), 0, pnum*sizeof(bigint));
    memset((void*)(oplatency), 0, pnum*sizeof(bigint));
    memset((void*)(midlatency), 0, pnum*sizeof(bigint));
    memset((void*)(storelatency), 0, pnum*sizeof(bigint));
    uint threads = MESHSIZE / pnum;
    std::deque<InstructionNameSpace::Instruction>::iterator it;
    for (it = inst.begin(); it < inst.end(); ++it) {
        if (it->op == InstructionNameSpace::LOAD) {
            loadlatency[it->dest/BLOCKROW] += LOADLATENCY * threads;
        }
        else if (it->op == InstructionNameSpace::STORE) {
            storelatency[it->src[0]/BLOCKROW] += STORELATENCY * threads;
        }
        else if (it->op == InstructionNameSpace::COPY) {
            uint p1, p2, level;
            p1 = it->src[0] / BLOCKROW;
            p2 = it->dest / BLOCKROW;
            level = getCommLevel(pnum, p1, p2);
            bigint max = midlatency[p1] > midlatency[p2] ? midlatency[p1] : midlatency[p2];
            uint maxthreads = MaxCopyThread[level] > threads ? threads : MaxCopyThread[level];
            if (level > 0) {
                max += CommWeight[level] * (threads / maxthreads);
            }
            else {
                max += OPLATENCY;
            }
            midlatency[p1] = max;
            midlatency[p2] = max;
        }
        else {
            uint p = it->src[0] / BLOCKROW;
            midlatency[p] += OPLATENCY;
            oplatency[p] += OPLATENCY;
        }
    }
    return 1;
}

int StageProcessors::calcEnergy()
{
    memset((void*)(loadenergy), 0, pnum*sizeof(double));
    memset((void*)(midenergy), 0, pnum*sizeof(double));
    memset((void*)(storeenergy), 0, pnum*sizeof(double));
    uint threads = MESHSIZE / pnum;
    std::deque<InstructionNameSpace::Instruction>::iterator it;
    for (it = inst.begin(); it < inst.end(); ++it) {
        if (it->op == InstructionNameSpace::LOAD) {
            loadenergy[it->dest/BLOCKROW] += LOADLATENCY * threads;
        }
        else if (it->op == InstructionNameSpace::STORE) {
            storeenergy[it->src[0]/BLOCKROW] += STORELATENCY * threads;
        }
        else if (it->op == InstructionNameSpace::COPY) {
            uint p1, p2, level;
            p1 = it->src[0] / BLOCKROW;
            p2 = it->dest / BLOCKROW;
            level = getCommLevel(pnum, p1, p2);
            if (level > 0) {
                midenergy[p1] += Readenergy[level] * threads;
                midenergy[p2] += WriteEnergy[level] * threads;
            }
            else {
                midenergy[p1] += OPENERGY * threads;
            }
        }
        else {
            uint p = it->src[0] / BLOCKROW;
            midenergy[p] += OPENERGY * threads;
        }
    }
    return 1;
}


/* Setters */
int StageProcessors::removeStoreInst(uint taskid)
{
    std::deque<InstructionNameSpace::Instruction>::iterator it;
    for (it = inst.begin(); it != inst.end(); ++it) {
        if (it->op == InstructionNameSpace::STORE && it->taskid == taskid) {
            inst.erase(it);
            // it->op = InstructionNameSpace::NOP;
            return 1;
        }
    }
    return 0;
}


/* Visitors */
uint StageProcessors::getTaskNum() const
{
    return schedule.size();
}

bigint StageProcessors::getLatency() const
{
    int i = 0;
    bigint ll, ml, sl;

    ll = 0;
    ml = 0;
    sl = 0;
    for (uint i = 0u; i < pnum; ++i) {
        ll += loadlatency[i];
        ml = ml > midlatency[i] ? ml : midlatency[i];
        sl += storelatency[i];
    }
    // printf("%lld %lld %lld\n", ll, ml, sl);
    return ll + ml + sl;
}

bigint StageProcessors::getOPLatency() const
{
    bigint op;

    op = 0;
    for (uint i = 0u; i < pnum; ++i) {
        op += oplatency[i];
    }
    op /= pnum;
    return op;
}

double StageProcessors::getEnergy() const
{
    double e = 0.0;
    for (uint i = 0u; i < pnum; ++i) {
        e += loadenergy[i] + midenergy[i] + storeenergy[i];
    }
    e += LEACKAGEENERGY * getLatency();
    return e;
}

uint const& StageProcessors::getpnum() const
{
    return pnum;
}

bigint const& StageProcessors::getPEMidLatency(const uint &id) const
{
    return midlatency[id];
}

bigint* const& StageProcessors::getMidLatency() const
{
    return midlatency;
}

ProcessElem* StageProcessors::getPE(uint id)
{
    return PE+id;
}

StageProcessors* StageProcessors::getNext()
{
    return next;
}

StageProcessors* StageProcessors::getPrior()
{
    return prior;
}


uint StageProcessors::getPEByTask(uint taskid)
{
    std::map<uint, Assignment>::iterator it = schedule.find(taskid);
    if (it == schedule.end()) {
        return UINT_MAX;
    }
    return schedule.at(taskid).pid;
}

uint StageProcessors::getTaskByPE(uint PEid, int n)
{
    if (!PE) {
        exit(-1);
    }
    ProcessElem * pe;
    pe = PE+PEid;
    if (n >= 0 && n < pe->tasks.size()) {
        return pe->tasks[n]->tid;
    }
    return pe->tasks.at(pe->tasks.size()-1)->tid;
}

uint StageProcessors::getLine(uint taskid)
{
    uint peid = getPEByTask(taskid);
    if (peid < UINT_MAX) {
        return (PE+peid)->cache[taskid];
    }
    return BLOCKROW;
}

uint const& StageProcessors::getOverwritepos(uint peid) const
{
    return (PE+peid)->overwriteflag;
}

Assignment* StageProcessors::getAssignmentByTask(uint taskid)
{
    std::map<uint, Assignment>::iterator it = schedule.find(taskid);

    if (it != schedule.end()) {
        return &it->second;
    }
    return NULL;
}

void StageProcessors::getTime2SpatialUtil(std::map<bigint, uint> &res, bigint offset)
{
    memset((void*)(midlatency), 0, pnum*sizeof(bigint));
    bigint midstart = 0;
    uint threads = MESHSIZE / pnum;

    bool *assigned = new bool[pnum * BLOCKROW];
    for (uint i = 0; i < pnum * BLOCKROW; ++i) {
        assigned[i] = false;
    }

    std::deque<InstructionNameSpace::Instruction>::iterator it;

    bigint curtime;
    std::map<bigint, uint>::iterator resit;
    it = inst.begin();
    while (it->op == InstructionNameSpace::LOAD) {
        assigned[it->dest] = true;
        midstart += LOADLATENCY * threads;
        curtime = midstart;
        resit = res.find(curtime+offset);
        if (resit != res.end()) {
            do {
                (resit->second) += threads;
                ++resit;
            } while(resit != res.end());
        }
        else {
            res.insert(std::make_pair(curtime+offset, threads));
            resit = res.find(curtime+offset);
            (--resit);
            uint last = resit->first <= offset ? 0u : resit->second;
            (++resit)->second += last;
            ++resit;
            while (resit != res.end()) {
                (resit->second) += threads;
                ++resit;
            }
        }
        ++it;
    }

    while (it->op != InstructionNameSpace::STORE) {
        uint inc = assigned[it->dest] ? 0u : 1u;
        assigned[it->dest] = true;
        if (it->op == InstructionNameSpace::COPY) {
            uint p1, p2, level;
            p1 = it->src[0] / BLOCKROW;
            p2 = it->dest / BLOCKROW;
            level = getCommLevel(pnum, p1, p2);
            curtime = midlatency[p1] > midlatency[p2] ? midlatency[p1] : midlatency[p2];
            uint maxthreads = MaxCopyThread[level] > threads ? threads : MaxCopyThread[level];
            if (level > 0) {
                curtime += CommWeight[level] * (threads / maxthreads);
            }
            else {
                curtime += OPLATENCY;
            }
            midlatency[p1] = curtime;
            midlatency[p2] = curtime;
        }
        else {
            uint p = it->src[0] / BLOCKROW;
            midlatency[p] += OPLATENCY;
            oplatency[p] += OPLATENCY;
            curtime = midlatency[p];
        }
        if (inc == 1u) {
            curtime += midstart;
            resit = res.find(curtime+offset);
            if (resit != res.end()) {
                do {
                    (resit->second) += threads;
                    ++resit;
                } while(resit != res.end());
            }
            else {
                res.insert(std::make_pair(curtime+offset, threads));
                resit = res.find(curtime+offset);
                (--resit);
                uint last = resit->first <= offset ? 0u : resit->second;
                (++resit)->second += last;
                ++resit;
                while (resit != res.end()) {
                    (resit->second) += threads;
                    ++resit;
                }
            }
        }
        ++it;
    }

    delete[] assigned;
}


void StageProcessors::printScheduleByTasks()
{
    printf("--------Schedule of each Task BEGIN--------\n");

    uint size = schedule.size();
    Assignment *a;
    for (uint i = 0u; i < size; ++i) {
        a = getAssignmentByTask(i);
        printf("    Task %u is assigned to PE No.%u. Start at %lld. End at %lld.\n", a->tid, a->pid, a->starttime, a->finishtime);
    }
    printf("--------Schedule of each Task END--------\n");
}

void StageProcessors::printScheduleByPE()
{
    ProcessElem *pe;
    std::vector<Assignment*>::iterator ait;
    int tasknum;
    printf("--------Schedule of each PE BEGIN--------\n");
    for (uint i = 0u; i < pnum; ++i) {
        pe = PE+i;
        printf("No.%u PE's schedule:\n", i);
        for (ait = pe->tasks.begin(); ait != pe->tasks.end(); ++ait) {
            printf("\t[Task No.%u] Start at %lld. End at %lld.\n", (*ait)->tid, (*ait)->starttime, (*ait)->finishtime);
        }
    }
    printf("--------Schedule of each PE END--------\n");
}

void StageProcessors::printInstructions(int stage)
{
    std::deque<InstructionNameSpace::Instruction>::iterator it;
    // printf("Stage %d:\n", stage);
    for (it = inst.begin(); it < inst.end(); ++it) {
        int pe[4] = {it->src[0]/BLOCKROW, it->src[1]/BLOCKROW,it->src[2]/BLOCKROW,it->dest/BLOCKROW};
        // printf("\t%-8s\t%s%d[%d] %s%d[%d] %s%d[%d] %d[%d]", \
        //     InstructionNameSpace::instname[(int)(it->op)], \
        //     it->invflag[0]?"~":"", it->src[0], pe[0], \
        //     it->invflag[1]?"~":"", it->src[1], pe[1], \
        //     it->invflag[2]?"~":"", it->src[2], pe[2], \
        //     it->dest, pe[3]);
        printf("\t%-8s\t%s%d %s%d %s%d %d", \
            InstructionNameSpace::instname[(int)(it->op)], \
            it->invflag[0]?"~":"", it->src[0], \
            it->invflag[1]?"~":"", it->src[1], \
            it->invflag[2]?"~":"", it->src[2], \
            it->dest);
        if (it->op >= InstructionNameSpace::INV) {
            printf("\t// Task.%d finished", it->taskid);
        }
        printf("\n");
    }
    // printf("Stage finish.\n\n");
}