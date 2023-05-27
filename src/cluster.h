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
int getCriticalPath(BooleanDag *dag, std::vector<uint> &vis, std::set<uint> &path);
void linearClustering(BooleanDag *dag, std::vector<std::vector<uint>> &clusters);
bigint getLatency(BooleanDag *dag, std::set<uint> &path);
Schedule clusterScheduling(BooleanDag *dag, int workload);

#endif