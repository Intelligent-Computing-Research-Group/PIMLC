/**  
 * @file    scheduler.h
 * @brief   Schedule the whole DAG and map it to HW
 * @author  Chenyu Tang
 * @version 2.3
 * @date    2022-11-18
 * @note    
 */
#include "booleandag.h"
#include "procelem.h"


///< Explore 2^i's cost and find the best i

/**
 * @brief Schedule info
 */
typedef struct Schedule {
    bigint latency;     ///< total latency
    bigint oplatency;   ///< total latency (operations)
    double energy;      ///< total energy
    int chunksize;      ///< the size of chunk in this schedule
    StageProcessors *p; ///< the head of stage list
} Schedule;

Schedule rankuHEFTSchedule(BooleanDag *G, uint workload);

Schedule rankuDynamicWeightsSchedule(BooleanDag *G, uint workload);

Schedule rankuCPDynamicWeightsSchedule(BooleanDag *G, uint workload);

void printInst(Schedule *s, uint offset, uint chunksize);