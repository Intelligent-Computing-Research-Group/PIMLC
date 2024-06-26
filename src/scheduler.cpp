/**  
 * @file    scheduler.cpp
 * @brief   Schedule the whole DAG and map it to HW
 * @author  Chenyu Tang
 * @version 4.0
 * @date    2023-02-09
 * @note    
 */

#include "scheduler.h"
#include "priority.h"
#include "placement.h"
#include "procelem.h"

Schedule schedule(BooleanDag *G, uint workload, bigint rank(BooleanDag*, bigint*), BooleanDag* placement(BooleanDag*,int, std::multimap<bigint, uint>&));


Schedule rankuHEFTSchedule(BooleanDag *G, uint workload)
{
    // Init
    Schedule sche;
    sche.chunksize = workload;
    bigint *priority;
    int pnum = pcfg.block_nums / workload;
    bigint totalms = 0;
    if (pnum <= 0) {
        exit(-1);
    }
    uint size = G->getsize();

    // priority - ranku
    priority = Priority::ranku(G);
    G->setPriority(priority);
    std::multimap<bigint, uint> ranklist;   ///< <rank, taskid>
    
    for (uint i = 0; i < size; ++i) {
        ranklist.insert(std::make_pair(priority[i], i));    ///< ordered list(map)
    }

    // task placement - HEFT
    StageProcessors* p = HEFT(G, pnum, ranklist);
    sche.p = p;
    sche.latency = 0;
    sche.iolatency = 0;
    sche.oplatency = 0;
    sche.energy = 0;
    sche.ioenergy = 0;
    while (p) {
        p->calcLatency();
        p->calcEnergy();
        sche.latency += p->getLatency();
        sche.iolatency += p->getIOLatency();
        sche.oplatency += p->getOPLatency();
        sche.energy += p->getEnergy();
        sche.ioenergy += p->getIOEnergy();
        // printf("%lld %lf\n", sche.latency, sche.energy);
        p = p->next;
    }

    delete[] priority;
    return sche;
}


Schedule rankuDynamicWeightsSchedule(BooleanDag *G, uint workload)
{
    // Init
    Schedule sche;
    sche.chunksize = workload;
    bigint *priority;
    int pnum = pcfg.block_nums / workload;
    bigint totalms = 0;
    if (pnum <= 0) {
        exit(-1);
    }
    uint size = G->getsize();

    // priority - ranku
    priority = Priority::ranku(G);
    G->setPriority(priority);
    std::multimap<bigint, uint> ranklist;   ///< <rank, taskid>
    
    for (uint i = 0; i < size; ++i) {
        ranklist.insert(std::make_pair(priority[i], i));    ///< ordered list(map)
    }

    // task placement - DynamicWeights
    StageProcessors* p = DynamicWeights(G, pnum, ranklist);
    sche.p = p;
    sche.latency = 0;
    sche.iolatency = 0;
    sche.oplatency = 0;
    sche.energy = 0;
    sche.ioenergy = 0;
    while (p) {
        // p->calcLatency();
        p->calcEnergy();
        sche.latency += p->getLatency();
        sche.iolatency += p->getIOLatency();
        sche.oplatency += p->getOPLatency();
        sche.energy += p->getEnergy();
        sche.ioenergy += p->getIOEnergy();
        // printf("%lld %lf\n", sche.latency, sche.energy);
        p = p->next;
    }

    delete[] priority;
    return sche;
}

Schedule rankuCPDynamicWeightsSchedule(BooleanDag *G, uint workload)
{
    // Init
    Schedule sche;
    sche.chunksize = workload;
    bigint *priority;
    int pnum = pcfg.block_nums / workload;
    bigint totalms = 0;
    if (pnum <= 0) {
        exit(-1);
    }
    uint size = G->getsize();

    // priority - ranku
    std::set<uint> maincluster;
    G->getMainCluster(maincluster, pcfg.commWeight[pcfg.levels]*2 / pcfg.compute_latency);
    priority = Priority::ranku(G);
    G->setPriority(priority);
    std::multimap<bigint, uint> ranklist;   ///< <rank, taskid>
    
    for (uint i = 0; i < size; ++i) {
        ranklist.insert(std::make_pair(priority[i], i));    ///< ordered list(map)
    }

    // task placement - DynamicWeights
    StageProcessors* p = CPDynamicWeights(G, pnum, ranklist, maincluster);
    sche.p = p;
    sche.latency = 0;
    sche.iolatency = 0;
    sche.oplatency = 0;
    sche.energy = 0;
    sche.ioenergy = 0;
    while (p) {
        // p->calcLatency();
        p->calcEnergy();
        sche.latency += p->getLatency();
        sche.iolatency += p->getIOLatency();
        sche.oplatency += p->getOPLatency();
        sche.energy += p->getEnergy();
        sche.ioenergy += p->getIOEnergy();
        // printf("%lld %lf\n", sche.latency, sche.energy);
        p = p->next;
    }

    delete[] priority;
    return sche;
}


void printInst(Schedule *s, uint offset, uint chunksize)
{
    StageProcessors **p = &(s->p);
    int stage = 0;
    printf("# memoffset %d\n", offset);
    printf("# meshcols %d\n", chunksize);
    printf("# meshrows %d\n", (*p)->getpnum());
    while ((*p)) {
        (*p)->printInstructions(stage++);
        p = &(*p)->next;
    }
}
