/**  
 * @file    placement.h
 * @brief   Task placement strategy
 * @author  Chenu Tang
 * @version 2.0
 * @date    2022-11-09
 * @note    
 */
#ifndef _PLACEMENT_
#define _PLACEMENT_

#include "booleandag.h"
#include "procelem.h"

int getCommLevel(uint totalpnum, uint p1, uint p2);

StageProcessors* HEFT(BooleanDag *G, int pnum, std::multimap<bigint, uint> &ranklist);

uint placeAtEarlestPE(BooleanDag *G, StageProcessors *P, uint taskid);

StageProcessors* DynamicWeights(BooleanDag *G, int pnum, std::multimap<bigint, uint> &ranklist);

StageProcessors* CPDynamicWeights(BooleanDag *G, int pnum, std::multimap<bigint, uint> &ranklist, std::set<uint> &maincluster);

uint placeAcdtoDynamicWeights(BooleanDag *G, StageProcessors *P, uint taskid, uint tasksleft);
uint placeAcdtoCPDynamicWeights(BooleanDag *G, StageProcessors *P, std::set<uint> &maincluster, uint taskid, uint tasksleft);
#endif