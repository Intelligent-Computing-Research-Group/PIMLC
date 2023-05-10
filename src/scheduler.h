/**  
 * @file    scheduler.h
 * @brief   Schedule the whole DAG and map it to HW
 * @author  Chenu Tang
 * @version 2.3
 * @date    2022-11-18
 * @note    
 */
#include "booleandag.h"
#include "procelem.h"


///< MESHSIZE is the max length
///< Explore 2^i's cost and find the best i


typedef struct Schedule {
    bigint latency;
    bigint oplatency;
    double energy;
    int chunksize;
    StageProcessors *p;
} Schedule;

Schedule rankuHEFTSchedule(BooleanDag *G, uint workload);

Schedule rankuDynamicWeightsSchedule(BooleanDag *G, uint workload);

Schedule rankuCPDynamicWeightsSchedule(BooleanDag *G, uint workload);

void printInst(Schedule *s, uint offset, uint chunksize);