#ifndef _CLUSTER_
#define _CLUSTER_
#include "booleandag.h"
#include <vector>
#include <stack>
#include <queue>
#include <iostream>
#include <set>

void printDAG(BooleanDag *dag);
int getCriticalPath(BooleanDag *dag, std::vector<uint> &vis);
void getCriticalPath(BooleanDag *dag);
void linearClustering(BooleanDag *dag);

#endif