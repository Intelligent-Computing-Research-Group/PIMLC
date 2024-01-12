/**  
 * @file    placement.cpp
 * @brief   Task placement strategy
 * @author  Chenyu Tang
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

double calculateStandardDeviation(bigint arr[], double m, int n) {
    double mean, stdDeviation = 0.0;
    //calculate standard deviation
    for (int i = 0; i < n; i++) {
        stdDeviation += pow(arr[i] - m, 2);
    }

    return sqrt(stdDeviation / n);
}

double calculateMean(bigint arr[], int n) {
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
/**
 * @brief uint placeAtEarlestPE
 * @details place specific task to the processor with est
 * @retval [0,pnum-1] - The target task
 * @retval pnum - this is a source node and will not be assigned in the future (when assigning its succs)
 * @retval UINT_MAX - this stage will no longer provide a proper target, should start another stage
 * @retval othervalues - unexpected return values
 */

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
        return pnum;    // if this node is a source node, return pnum
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
            bigint predt = predfinishtime[j] + (PIMConf::getCommWeight(getCommLevel(pnum, i, predpeid[j])));
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

StageProcessors* CPDynamicWeights(BooleanDag *G, int pnum, std::multimap<bigint, uint> &ranklist, std::set<uint> &maincluster)
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
            pid = placeAcdtoCPDynamicWeights(G, *p, maincluster, taskid, ranklist.size());
            if (pid != UINT_MAX) {
                // if (pnum == 64) {
                //     printf("[%u][%c] ", taskid, (maincluster.find(taskid)==maincluster.end())?'n':'m');
                //     for (uint i = 0; i < pnum; ++i) {
                //         printf("%u ", (*p)->getTaskNum(i));
                //     }
                //     printf("\n\n");
                // }
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
        double temp = len / PIMConf::getBlockRows();
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
        if (max <= arr[i]) {
            max = arr[i];
            idx = i;
        }
    }
    return idx;
}

uint placeAcdtoDynamicWeights(BooleanDag *G, StageProcessors *P, uint taskid, uint tasksleft)
{
    static uint blocknums = PIMConf::getBlockNums();
    static bigint oplatency = PIMConf::getComputeLatency();
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
        avgblkcost = oplatency;
        avgcnt = 0;
    }
    /**
     * curcost: determine a pe, (check each pred, find best src pe)->map, get blk time, update avg
     * futurecost: check each succ, check their pred pos, calc exp blk time
     * stddeviation: normalcalc
     * maxrow: the change of max row
    */
    uint threads = blocknums / pnum;
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
                        uint maxthreads = PIMConf::getCopyThreads(level) > threads ? threads : PIMConf::getCopyThreads(level);
                        cplat += PIMConf::getCommWeight(level) * (threads / maxthreads);
                    }
                }
                else {
                    newmidlatency[srcpeid] = midlat[i];
                    if (i == maxidx) {
                        uint level = getCommLevel(pnum, i, srcpeid);
                        uint maxthreads = PIMConf::getCopyThreads(level) > threads ? threads : PIMConf::getCopyThreads(level);
                        cplat += PIMConf::getCommWeight(level) * (threads / maxthreads);
                    }
                }
            }
        }
        newmidlatency[i] += oplatency;

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
            maxrow += oplatency;
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

uint placeAcdtoCPDynamicWeights(BooleanDag *G, StageProcessors *P, std::set<uint> &maincluster, uint taskid, uint tasksleft)
{
    static uint blocknums = PIMConf::getBlockNums();
    static bigint oplatency = PIMConf::getComputeLatency();
    Vertice *v = G->getvertice(taskid);
    uint pnum = P->getpnum();
    uint prednum = v->prednum;
    if (prednum == 0) {
        return pnum;
    }
    uint pid;   ///< return value
    /* random start */
    pid = rand() % pnum;
    if (P->checkPlaceable(G, pid, taskid)) {
        P->dynamicWeightsAssignTask(G, taskid, pid);
        return pid;
    }
    else {
        return UINT_MAX;
    }
    /* random end */

    if (maincluster.find(taskid) != maincluster.end()) {
        if (P->checkPlaceable(G, pnum-1, taskid)) {
            P->dynamicWeightsAssignTask(G, taskid, pnum-1);
            return pnum-1;
        }
        else {
            return UINT_MAX;
        }
    }

    double cost = 1e308;
    /**
     * curcost: determine a pe, (check each pred, find best src pe)->map, get blk time, update avg
     * futurecost: check each succ, check their pred pos, calc exp blk time
     * stddeviation: normalcalc
     * maxrow: the change of max row
    */
    uint threads = blocknums / pnum;
    uint succnum = v->succnum;
    ProcessElem* pe;
    bigint *midlat = P->getMidLatency();
    double mean = calculateMean(midlat, pnum);
    double curstdDeviation = calculateStandardDeviation(midlat, mean, pnum);
    // if (pnum == 64 && taskid == 189) {
    //     printf("\n!!old 189!!\n");
    //     for (uint i = 0; i < pnum; ++i) {
    //         printf("%lld ", midlat[i]);
    //     }
    //     printf("\n");
    // }
    uint maxidx = getMaxidx(midlat, pnum);
    bigint copylat;
    ///< Try each pe as target
    int placeable = 0;
    bool afterfirstcompensate, compensate_mark;

    compensate_mark = false;
    bigint eft = LONG_LONG_MAX;
    uint eftcnt = 0;
    for (uint i = 0u; i < pnum; ++i) {
        afterfirstcompensate = false;
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
        double stddeviation, maxrow;
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
                if (newselfblk) {
                    curmaxpelat += minblkt;
                    newmidlatency[i] = curmaxpelat;
                    if (srcpeid == pnum-1) {
                        uint level = getCommLevel(pnum, i, srcpeid);
                        uint maxthreads = PIMConf::getCopyThreads(level) > threads ? threads : PIMConf::getCopyThreads(level);
                        cplat += PIMConf::getCommWeight(level) * (threads / maxthreads);
                    }
                }
                else {
                    newmidlatency[srcpeid] = curmaxpelat;
                    if (i == pnum-1) {
                        uint level = getCommLevel(pnum, i, srcpeid);
                        uint maxthreads = PIMConf::getCopyThreads(level) > threads ? threads : PIMConf::getCopyThreads(level);
                        cplat += PIMConf::getCommWeight(level) * (threads / maxthreads);
                    }
                }
            }
        }
        newmidlatency[i] += oplatency;

        ///< get stddev change
        stddeviation = calculateStandardDeviation(newmidlatency, pnum);
        // if (pnum == 64 && taskid == 189) {
        //     printf("\n!!new 189!!\n");
        //     for (uint i = 0; i < pnum; ++i) {
        //         printf("%lld ", newmidlatency[i]);
        //     }
        //     printf("\n");
        // }

        ///< get max row change
        maxrow = cplat;
        if (pnum-1 == i || cplat > 0) {
            maxrow += oplatency;
        }
        // printf("Trying to assign %u to %u, weights: ", taskid, i);
        if (midlat[i] < midlat[maxidx] - 2*stddeviation) {
            if (compensate_mark == false) {
                compensate_mark = true;
            }
            else {
                afterfirstcompensate = true;
            }
        }
        else if (compensate_mark) {
            delete[] newmidlatency;
            continue;
        }
        tmpcost = totalCost(0, 0, stddeviation-curstdDeviation, maxrow, P, tasksleft);

        if (newmidlatency[i] < eft) {
            eft = newmidlatency[i];
            eftcnt = 1;
        }
        else if (newmidlatency[i] == eft) {
            eftcnt++;
        }
        if ((compensate_mark&&(!afterfirstcompensate)) || (afterfirstcompensate&&(tmpcost<cost)) || ((!compensate_mark)&&(tmpcost<cost))) {
            cost = tmpcost;
            pid = i;
        }
        // if (pnum == 64) {
        //     printf("%u(%lf,%lf) ",i, stddeviation, maxrow);
        // }
        
        delete[] newmidlatency;
    }
    // if (pnum == 64) {
    //     printf("\n");
    // }
    if (placeable) {
        // printf("assign to %u\n", pid);
        P->dynamicWeightsAssignTask(G, taskid, pid);
    }
    else {
        return UINT_MAX;
    }
    return pid;
}

