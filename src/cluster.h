#ifndef _CLUSTER_
#define _CLUSTER_
#include "booleandag.h"
#include "scheduler.h"
#include "priority.h"
#include "placement.h"
#include "procelem.h"

#include <vector>
#include <stack>
#include <queue>
#include <iostream>
#include <set>

void printDAG(BooleanDag *dag);
int getCriticalPath(BooleanDag *dag, std::vector<uint> &vis, std::vector<uint> &path);
void getCriticalPath(BooleanDag *dag, std::vector<uint> &path);
void linearClustering(BooleanDag *dag, std::vector<std::vector<uint>> &clusters);
bigintg getLatency(BooleanDag *dag, std::vector<uint> &path);
void clusterScheduling(BooleanDag *dag, std::vector<std::vector<uint>> &clusters, int workload);

#endif