/**  
 * @file    placement.cpp
 * @brief   Task placement strategy
 * @author  Chenu Tang
 * @version 4.0
 * @date    2022-11-18
 * @note    
 */

#include "placement.h"
#include <limits>
#include <cmath>
#include <cstring>

double calculateStandardDeviation(bigint arr[], int n) {
    double sum = 0.0, mean, stdDeviation = 0.0;

    //calculate mean
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    mean = sum / n;

    //calculate standard deviation
    for (int i = 0; i < n; i++) {
        stdDeviation += pow(arr[i] - mean, 2);
    }

    return sqrt(stdDeviation / n);
}

StageProcessors* HEFT(BooleanDag *G, int pnum, std::multimap<bigint, uint> &ranklist)
{
    StageProcessors *stages = NULL;
    StageProcessors *prior = NULL;
    StageProcessors **p = &stages;
    uint stagecnt = 0;

    uint size = G->getsize();

    bool *assigned = new bool[size];
    for (uint i = 0; i < size; ++i) {
        assigned[i] = false;
    }

    while (ranklist.size()) {
        ++stagecnt;
        std::multimap<bigint, uint>::iterator iter = ranklist.begin();
        uint taskid;
        uint pid = 0;
        *p = new StageProcessors;
        (*p)->init(pnum);
        (*p)->prior = prior;

        uint taskcnt = 0;
        while (ranklist.size() && pid != UINT_MAX) {
            iter = ranklist.begin();
            taskid = iter->second;
            pid = placeAtEarlestPE(G, *p, taskid);
            if (pid != UINT_MAX) {
                ranklist.erase(iter);
                ++taskcnt;
                assigned[taskid] = true;
                (*p)->releaseMem(G, taskid, assigned);
            }
        }
        (*p)->assignFinish();
        // (*p)->printScheduleByPE();
        // (*p)->printInstructions(stagecnt-1);
        prior = *p;
        p = &((*p)->next);
    }
    delete[] assigned;
    return stages;
}

uint placeAtEarlestPE(BooleanDag *G, StageProcessors *P, uint taskid)
{
    uint pid;   ///< return value
    bigint est = INT_MAX;
    uint pnum = P->getpnum();
    uint prednum;
    ProcessElem* pe;
    Vertice *v = G->getvertice(taskid);
    prednum = v->prednum;
    if (prednum == 0) {
        return pnum;
    }

    uint *predpeid = new uint[prednum];
    bigint *predfinishtime = new bigint[prednum];
    bigint *predcommcost = new bigint[prednum];


    for (uint i = 0u; i < prednum; ++i) {
        Edge *e = v->predecessors[i];
        uint predid = e->src->id;
        Assignment* predassignment = P->getAssignmentByTask(predid);

        if (predassignment) {
            predpeid[i] = predassignment->pid;
            predfinishtime[i] = predassignment->finishtime;
            predcommcost[i] = e->weight;
        }
        else {
            if (P->prior) {
                predassignment = P->prior->getAssignmentByTask(predid);
            }
            if (predassignment && P->prior->getLine(predid)>=P->getOverwritepos(predassignment->pid)) {
                predpeid[i] = predassignment->pid;
            }
            else {
                predpeid[i] = pnum;
            }
            
            predfinishtime[i] = 0;
            predcommcost[i] = e->weight;
        }
    }

    ///< Try each pe as target
    int placeable = 0;
    for (uint i = 0u; i < pnum; ++i) {
        pe = P->getPE(i);
        if (P->checkPlaceable(G, i, taskid)) {
            placeable = 1;
        }
        else {
            continue;
        }
        bigint avail = pe->opeft;
        for (uint j = 0u; j < prednum; ++j) {
            bigint predt = predfinishtime[j] + (CommWeight[getCommLevel(pnum, i, predpeid[j])]);
            avail = avail > predt ? avail : predt;
        }
        if (avail < est) {
            est = avail;
            pid = i;
        }
    }
    if (placeable) {
        P->assignTask(G, taskid, pid, est, est+v->weight);
    }
    else {
        return UINT_MAX;
    }
    return pid;
}



StageProcessors* DynamicWeights(BooleanDag *G, int pnum, std::multimap<bigint, uint> &ranklist)
{
    StageProcessors *stages = NULL;
    StageProcessors *prior = NULL;
    StageProcessors **p = &stages;
    uint stagecnt = 0;

    uint size = G->getsize();

    bool *assigned = new bool[size];
    for (uint i = 0; i < size; ++i) {
        assigned[i] = false;
    }

    while (ranklist.size()) {
        ++stagecnt;
        std::multimap<bigint, uint>::iterator iter = ranklist.begin();
        uint taskid;
        uint pid = 0;
        *p = new StageProcessors;
        (*p)->init(pnum);
        (*p)->prior = prior;

        uint taskcnt = 0;
        while (ranklist.size() && pid != UINT_MAX) {
            iter = ranklist.begin();
            taskid = iter->second;
            pid = placeAcdtoDynamicWeights(G, *p, taskid, ranklist.size());
            if (pid != UINT_MAX) {
                ranklist.erase(iter);
                ++taskcnt;
                assigned[taskid] = true;
                (*p)->releaseMem(G, taskid, assigned);
            }
        }
        (*p)->assignFinish();
        // (*p)->printScheduleByPE();
        // (*p)->printInstructions(stagecnt-1);
        prior = *p;
        p = &((*p)->next);
    }
    delete[] assigned;
    return stages;
}

double totalCost(double curcost, double futurecost, double stddeviation, double maxrow, StageProcessors *P, uint tasksleft)
{
    double cost;
    double pct = 0;
    double w1, w2, w3, w4;
    uint pnum = P->getpnum();
    for (uint i = 0u; i < pnum; ++i) {
        double len = P->getPE(i)->cache.size();
        double temp = len / BLOCKROW;
        pct = temp > pct ? temp : pct;
    }
    uint tasknum = P->getTaskNum();
    double predicttasksleft = tasknum*(1.0-pct);
    if (tasksleft < predicttasksleft) {
        pct = (tasknum *1.0) / (tasknum+tasksleft);
    }
    if (pct < 0.5) {
        // w3 = 2 * pct * pct;
        w3 = pct;
    }
    else {
        // w3 = 2 * (pct-1) * (pct-1);
        w3 = 1 - pct;
    }
    w4 = pct * pct;
    double wtmp = 1 - w3 - w4;
    w1 = wtmp * (0.6*pct+0.2);
    w2 = wtmp * (-0.6*pct+0.8);

    // printf("%lf(%lf) %lf(%lf) %lf(%lf) %lf(%lf)\n", curcost, w1, futurecost, w2, stddeviation*pnum, w3, maxrow, w4);
    cost = curcost * w1 + futurecost * w2 + stddeviation * pnum * w3 + maxrow * w4;
    return cost;
}

uint getMaxidx(bigint *arr, uint n)
{
    uint idx = 0u;
    if (n == 0u) {
        return idx;
    }
    bigint max = arr[idx];
    for (uint i = 1u; i < n; ++i) {
        if (max < arr[i]) {
            max = arr[i];
            idx = i;
        }
    }
    return idx;
}

uint placeAcdtoDynamicWeights(BooleanDag *G, StageProcessors *P, uint taskid, uint tasksleft)
{
    Vertice *v = G->getvertice(taskid);
    uint pnum = P->getpnum();
    uint prednum = v->prednum;
    if (prednum == 0) {
        return pnum;
    }
    uint pid;   ///< return value
    double cost = 1e308;
    static double avgblkcost = 0;
    static double avgcnt = 0;

    if (P->getTaskNum() == 0) {
        avgblkcost = OPLATENCY;
        avgcnt = 0;
    }
    /**
     * curcost: determine a pe, (check each pred, find best src pe)->map, get blk time, update avg
     * futurecost: check each succ, check their pred pos, calc exp blk time
     * stddeviation: normalcalc
     * maxrow: the change of max row
    */
    uint threads = MESHSIZE / pnum;
    uint succnum = v->succnum;
    ProcessElem* pe;
    bigint *midlat = P->getMidLatency();
    double curstdDeviation = calculateStandardDeviation(midlat, pnum);
    uint maxidx = getMaxidx(midlat, pnum);
    bigint copylat;
    uint copycnt;
    ///< Try each pe as target
    int placeable = 0;


    for (uint i = 0u; i < pnum; ++i) {
        double tmpcost;
        
        bigint *newmidlatency = new bigint[pnum];
        bigint curmaxpelat = midlat[i];
        bigint cplat = 0;
        memcpy(newmidlatency, midlat, sizeof(bigint)*pnum);

        pe = P->getPE(i);
        if (P->checkPlaceable(G, i, taskid)) {
            placeable = 1;
        }
        else {
            continue;
        }
        double curcost, futurecost, stddeviation, maxrow;
        uint cptime = 0u;
        curcost = 0;
        futurecost = 0;
        stddeviation = 0;
        maxrow = 0;

        ///< get curcost - check each pred, find best src pe
        for (uint j = 0u; j < prednum; ++j) {
            uint predid = v->predecessors[j]->src->id;
            bigint minblkt = BIGINT_MAX;
            bool newselfblk = false;
            ProcessElem *srcpe;
            uint srcpeid;

            ///< find local data -> do not need copy / load
            if (pe->cache.find(predid) != pe->cache.end()) {
                continue;
            }

            ///< find src with min blk time
            for (uint k = 0; k < pnum; ++k) {
                srcpe = P->getPE(k);
                if (k != i && srcpe->cache.find(predid)!=srcpe->cache.end()) {
                    ///< find this task in cache
                    if (midlat[k] > midlat[i] && midlat[k] <= curmaxpelat) {
                        ///< do not need extra blk
                        newselfblk = false;
                        minblkt = 0;
                        srcpeid = srcpe->id;
                        break;
                    }
                    else if (midlat[k] > midlat[i]) {
                        ///< need more self blk
                        bigint dist = midlat[k] - curmaxpelat;
                        if (dist < minblkt) {
                            newselfblk = true;
                            minblkt = dist;
                            srcpeid = srcpe->id;
                        }
                    }
                    else {
                        ///< need src pe's blk
                        bigint dist = midlat[i] - midlat[k];
                        if (dist < minblkt) {
                            newselfblk = false;
                            minblkt = dist;
                            srcpeid = srcpe->id;
                        }
                    }
                }
            }

            ///< need copy(not load) - update current max self blk time
            if (minblkt < BIGINT_MAX) {
                curcost += minblkt;
                ++cptime;
                if (newselfblk) {
                    curmaxpelat += minblkt;
                    newmidlatency[i] = curmaxpelat;
                    if (srcpeid == maxidx) {
                        uint level = getCommLevel(pnum, i, srcpeid);
                        uint maxthreads = MaxCopyThread[level] > threads ? threads : MaxCopyThread[level];
                        cplat += CommWeight[level] * (threads / maxthreads);
                    }
                }
                else {
                    newmidlatency[srcpeid] = midlat[i];
                    if (i == maxidx) {
                        uint level = getCommLevel(pnum, i, srcpeid);
                        uint maxthreads = MaxCopyThread[level] > threads ? threads : MaxCopyThread[level];
                        cplat += CommWeight[level] * (threads / maxthreads);
                    }
                }
            }
        }
        newmidlatency[i] += OPLATENCY;

        ///< get futurecost - check each pred, find best src pe
        for (uint j = 0u; j < succnum; ++j) {
            Vertice *succ = v->successors[j]->dest;
            uint succid = succ->id;
            uint succprednum = succ->prednum;
            for (uint k = 0u; k < succprednum; ++k) {
                if (succ->predecessors[k]->src->id != taskid) {
                    if (pe->cache.find(succ->predecessors[k]->src->id) == pe->cache.end()) {
                        futurecost += avgblkcost;
                    }
                }
            }
        }

        ///< get stddev change
        stddeviation = calculateStandardDeviation(newmidlatency, pnum) - curstdDeviation;

        ///< get max row change
        maxrow = cplat;
        if (maxidx == i || cplat > 0) {
            maxrow += OPLATENCY;
        }
        // printf("Trying to assign %u to %u, weights: ", taskid, i);
        tmpcost = totalCost(curcost, futurecost, stddeviation, maxrow, P, tasksleft);
        if (tmpcost < cost) {
            cost = tmpcost;
            pid = i;
            copylat = curcost;
            copycnt = cptime;
        }
        delete[] newmidlatency;
    }
    if (placeable) {
        if (copycnt > 0u) {
            avgblkcost = avgblkcost * avgcnt + copylat;
            avgcnt += copycnt;
            avgblkcost /= avgcnt;
        }
        // printf("assign to %u\n", pid);
        P->dynamicWeightsAssignTask(G, taskid, pid);
    }
    else {
        return UINT_MAX;
    }
    return pid;
}

