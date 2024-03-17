/**  
 * @file    placement.h
 * @brief   Task placement strategy
 * @author  Chenyu Tang
 * @version 2.0
 * @date    2022-11-09
 * @note    
 */
#ifndef _PLACEMENT_
#define _PLACEMENT_

#include "booleandag.h"
#include "procelem.h"

/**
 * @brief int getCommLevel
 * @details whether the two processors are in the same mat, in the same bank, cross-bank or any other levels
 */
int getCommLevel(uint totalpnum, uint p1, uint p2);

StageProcessors* HEFT(BooleanDag *G, int pnum, std::multimap<bigint, uint> &ranklist);

/**
 * @brief uint placeAtEarlestPE
 * @details place specific task to the processor with est
 * @retval [0,pnum-1] - The target task
 * @retval pnum - this is a source node and will not be assigned in the future (when assigning its succs)
 * @retval UINT_MAX - this stage will no longer provide a proper target, should start another stage
 * @retval othervalues - unexpected return values
 */
uint placeAtEarlestPE(BooleanDag *G, StageProcessors *P, uint taskid);

StageProcessors* DynamicWeights(BooleanDag *G, int pnum, std::multimap<bigint, uint> &ranklist);

StageProcessors* CPDynamicWeights(BooleanDag *G, int pnum, std::multimap<bigint, uint> &ranklist, std::set<uint> &maincluster);

uint placeAcdtoDynamicWeights(BooleanDag *G, StageProcessors *P, uint taskid, uint tasksleft);
uint placeAcdtoCPDynamicWeights(BooleanDag *G, StageProcessors *P, std::set<uint> &maincluster, uint taskid, uint tasksleft);
#endif
