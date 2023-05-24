#include "cluster.h"
// #define _CLUSTER_SHOW_EDGE_TYPE_

void printDAG(BooleanDag *dag)
{
    // print all egdes
    int cnt = 0;
    for (uint i = 0u; i < dag->getsize(); i++)
    {
        if (dag->getvertice(i)->succnum == 0)
        {
            std::cout << dag->getvertice(i)->name << " has no successor" << std::endl;
            continue;
        }
        cnt += dag->getvertice(i)->succnum;
        for (uint j = 0u; j < dag->getvertice(i)->succnum; j++)
        {
            std::cout << dag->getvertice(i)->name << " " << dag->getvertice(i)->successors[j]->dest->name << " " << dag->getvertice(i)->successors[j]->weight << std::endl;
            // printf("%u %u\n", i, dag->getvertice(i)->successors[j]->dest->id);
#ifdef _CLUSTER_SHOW_EDGE_TYPE_
            switch (dag->getvertice(i)->successors[j]->dest->type)
            {
            case 0:
                printf("NORMAL\n");
                break;
            case 1:
                printf("CONSTANT\n");
                break;
            case 2:
                printf("VINV\n");
                break;
            case 3:
                printf("VAND\n");
                break;
            case 4:
                printf("VOR\n");
                break;
            case 5:
                printf("VXOR\n");
                break;
            case 6:
                printf("VXOR3\n");
                break;
            case 7:
                printf("VMAJ\n");
                break;
            default:
                printf("UNKNOWN\n");
                break;
            }
#endif
        }
    }
    printf("DAG size: %u\n", dag->getsize() - 1);
    printf("DAG edge number: %u\n", cnt);
    return;
}

int getCriticalPath(BooleanDag *dag, std::vector<uint> &vis, std::vectot<uint> &path)
{
    int size = dag->getsize();

    std::queue<uint> q;

    // BFS, target the subgraph in vis as 2
    int k = 0, cnt = 0;
    while (k < size && vis[k] == 1)
        k++;
    q.push(k);
    cnt++;
    while (!q.empty())
    {
        uint u = q.front();
        q.pop();
        vis[u] = 2;
        Vertice *now = dag->getvertice(u);
        for (uint i = 0; i < now->succnum; i++)
        {
            uint v = now->successors[i]->dest->id;
            if (vis[v] != 0)
                continue;
            q.push(v);
            cnt++;
        }
        for (uint i = 0; i < now->prednum; i++)
        {
            uint v = now->predecessors[i]->src->id;
            if (vis[v] != 0)
                continue;
            q.push(v);
            cnt++;
        }
    }
    q = std::queue<uint>();

    // output vis
    // std::cout << "vis: ";
    // for (uint i = 0; i < size; i++)
    //     std::cout << vis[i] << " ";
    // std::cout << std::endl;

    // if the subgraph is a single node, return
    if (cnt == 1)
    {
        vis[k] = 1;
        path.push_back(k);
        // std::cout << dag->getvertice(k)->name << std::endl;
        return 1;
    }

    // find the nodes with no predecessors
    std::vector<int> indegree(size, 0);
    for (uint i = 0; i < size; i++)
    {
        Vertice *now = dag->getvertice(i);
        if (vis[i] != 2)
            continue;
        for (uint j = 0; j < now->prednum; j++)
            if (vis[now->predecessors[j]->src->id] == 2)
                indegree[i]++;
        if (indegree[i] == 0)
        {
            // std::cout << now->name << std::endl;
            q.push(i);
        }
    }

    int max_ve = -1;
    std::stack<uint> topo;
    std::vector<int> ve(size, 0);
    // topological sort and update ve
    // ve[v] = max{ve[u] + weight(u) + weight(u, v)}
    while (!q.empty())
    {
        uint u = q.front();
        q.pop();
        topo.push(u);
        Vertice *now = dag->getvertice(u);
        for (uint i = 0; i < now->succnum; i++)
        {
            uint v = now->successors[i]->dest->id;
            if (vis[v] != 2)
                continue;
            if (--indegree[v] == 0)
                q.push(v);
            ve[v] = std::max(ve[v], ve[u] + (int)now->weight + (int)now->successors[i]->weight);
            max_ve = std::max(max_ve, ve[v]);
        }
    }
    // update vl
    // vl[u] = min{vl[v] - weight(u) - weight(u, v)}
    std::vector<int> vl(size, max_ve);
    while (!topo.empty())
    {
        uint u = topo.top();
        topo.pop();
        Vertice *now = dag->getvertice(u);
        for (uint i = 0; i < now->succnum; i++)
        {
            uint v = now->successors[i]->dest->id;
            if (vis[v] != 2)
                continue;
            vl[u] = std::min(vl[u], vl[v] - (int)now->weight - (int)now->successors[i]->weight);
        }
    }

    // output ve vl
    // std::cout << "ve: ";
    // for (uint i = 0; i < size; i++)
    //     std::cout << ve[i] << " ";
    // std::cout << std::endl;
    // std::cout << "vl: ";
    // for (uint i = 0; i < size; i++)
    //     std::cout << vl[i] << " ";
    // std::cout << std::endl;

    cnt = 0;
    // e[i->j] = ve[i], k[i->j] = vl[j] - weight[i->j]
    // weight[i->j] = weight(i) + weight(i->j)
    k = 0;
    int e, l, found = 0;
    // find the first egde with e = l
    while (k < size && found == 0)
    {
        if (vis[k] != 2)
        {
            k++;
            continue;
        }
        Vertice *now = dag->getvertice(k);
        e = ve[k];
        for (uint i = 0; i < now->succnum; i++)
        {
            uint v = now->successors[i]->dest->id;
            if (vis[v] != 2)
                continue;
            l = vl[v] - now->weight - now->successors[i]->weight;
            // std::cout << now->name << " " << e << " " << l << std::endl;
            if (e == l)
            {
                found = 1, vis[k] = 1, vis[v] = 1, cnt += 2;
                path.push_back(k);
                path.push_back(v);
                // std::cout << now->name << " " << dag->getvertice(v)->name << " ";
                k = v;
                break;
            }
        }
        if (found == 0)
            k++;
    }
    // DFS to find the critical path
    while (k < size && found == 1)
    {
        found = 0;
        Vertice *now = dag->getvertice(k);
        e = ve[k];
        for (uint i = 0; i < now->succnum; i++)
        {
            uint v = now->successors[i]->dest->id;
            if (vis[v] != 2)
                continue;
            l = vl[v] - now->successors[i]->dest->weight - now->successors[i]->weight;
            if (e == l)
            {
                found = 1, vis[v] = 1, cnt++;
                path.push_back(v);
                // std::cout << dag->getvertice(v)->name << " ";
                k = v;
                break;
            }
        }
    }
    std::cout << std::endl;
    // set unvisited nodes to 0
    for (uint i = 0; i < size; i++)
        if (vis[i] == 2)
            vis[i] = 0;
    return cnt;
}

// overload for non-iter use
void getCriticalPath(BooleanDag *dag, std::vector<uint> &path)
{
    int size = dag->getsize();

    // if there is only one node, output it
    if (size == 1)
    {
        path.push_back(0);
        // std::cout << dag->getvertice(0)->name << std::endl;
        return;
    }
    std::vector<int> vis(size, 0);
    std::queue<uint> q;

    // find the nodes with no predecessors
    std::vector<int> indegree(size, 0);
    for (uint i = 0; i < size; i++)
    {
        indegree[i] = dag->getvertice(i)->prednum;
        if (indegree[i] == 0)
            q.push(i);
    }

    int max_ve = -1;
    std::stack<uint> topo;
    std::vector<int> ve(size, 0);
    // topological sort and update ve
    // ve[v] = max{ve[u] + weight(u) + weight(u, v)}
    while (!q.empty())
    {
        uint u = q.front();
        q.pop();
        topo.push(u);
        Vertice *now = dag->getvertice(u);
        for (uint i = 0; i < now->succnum; i++)
        {
            uint v = now->successors[i]->dest->id;
            if (--indegree[v] == 0)
                q.push(v);
            ve[v] = std::max(ve[v], ve[u] + (int)now->weight + (int)now->successors[i]->weight);
            max_ve = std::max(max_ve, ve[v]);
        }
    }
    // update vl
    // vl[u] = min{vl[v] - weight(u) - weight(u, v)}
    std::vector<int> vl(size, max_ve);
    while (!topo.empty())
    {
        uint u = topo.top();
        topo.pop();
        Vertice *now = dag->getvertice(u);
        for (uint i = 0; i < now->succnum; i++)
        {
            uint v = now->successors[i]->dest->id;
            vl[u] = std::min(vl[u], vl[v] - (int)now->weight - (int)now->successors[i]->weight);
        }
    }

    // output ve vl
    // std::cout << "ve: ";
    // for (uint i = 0; i < size; i++)
    //     std::cout << ve[i] << " ";
    // std::cout << std::endl;
    // std::cout << "vl: ";
    // for (uint i = 0; i < size; i++)
    //     std::cout << vl[i] << " ";
    // std::cout << std::endl;

    // e[i->j] = ve[i], k[i->j] = vl[j] - weight[i->j]
    // weight[i->j] = weight(i) + weight(i->j)
    int k = 0;
    int e, l, found = 0;
    // find the first egde with e = l
    while (k < size && found == 0)
    {
        Vertice *now = dag->getvertice(k);
        e = ve[k];
        for (uint i = 0; i < now->succnum; i++)
        {
            uint v = now->successors[i]->dest->id;
            l = vl[v] - now->weight - now->successors[i]->weight;
            // std::cout << now->name << " " << e << " " << l << std::endl;
            if (e == l)
            {
                found = 1, vis[k] = 1, vis[v] = 1;
                path.push_back(k);
                path.push_back(v);
                // std::cout << now->name << " " << dag->getvertice(v)->name << " ";
                k = v;
                break;
            }
        }
        if (found == 0)
            k++;
    }
    // DFS to find the critical path
    while (k < size && found == 1)
    {
        found = 0;
        Vertice *now = dag->getvertice(k);
        e = ve[k];
        for (uint i = 0; i < now->succnum; i++)
        {
            uint v = now->successors[i]->dest->id;
            if (vis[v] != 0)
                continue;
            l = vl[v] - now->successors[i]->dest->weight - now->successors[i]->weight;
            if (e == l)
            {
                found = 1, vis[v] = 1;
                path.push_back(v);
                // std::cout << dag->getvertice(v)->name << " ";
                k = v;
                break;
            }
        }
    }
    std::cout << std::endl;
    return;
}

void linearClustering(BooleanDag *dag, std::vector<std::vector<uint>> &clusters)
{
    int size = dag->getsize();
    std::vector<uint> vis(size, 0);
    int cnt = 0;
    // cnt = getCriticalPath(dag, vis);
    // std::cout << cnt << std::endl;
    while (cnt < size)
    {
        std::vector<uint> path;
        int k = getCriticalPath(dag, vis, path);
        cnt += k;
        if (k == 0)
            break;
        else
            clusters.push_back(path);
        // std::cout << cnt << std::endl;
    }
    return;
}

bigint getLatency(BooleanDag *dag, std::vector<uint> &path)
{
    bigint latency = 0;
    for (uint i = 0; i < path.size() - 1; i++)
    {
        Vertice *now = dag->getvertice(path[i]);
        for (uint j = 0; j < now->succnum; j++)
        {
            if (now->successors[j]->dest->id == path[i + 1])
            {
                latency += now->successors[j]->weight;
                break;
            }
        }
    }
    return latency;
}

uint placeAtEarleast(BooleanDag *dag, StageProcessors *P, uint taskid)
{
    uint pid;
    bigint est = INT_MAX;
    uint pnum = P->getpnum();
    uint predum;
    ProcessElem *pe;
    Vertice *v = G->getvertice(taskid);
    predum = v->prednum;
    // ?
    // if the task has no predecessors, then it can be placed at any processor
    if (predum == 0)
        return pnum;

    uint *predpeid = new uint[predum];
    bigint *predfinishtime = new bigint[predum];
    bigint *predcommcost = new bigint[predum];

    // ？
    // get the finish time of predecessors in order to test?
    for (uint i = 0u; i < predum; i++)
    {
        Edge *e = v->predecessors[i];
        uint predid = e->src->id;
        Assignment *predassignment = P->getAssignmentByTask(predid);

        if (predassignment)
        {
            predpeid[i] = predassignment->pid;
            predfinishtime[i] = predassignment->finishtime;
            predcommcost[i] = e->weight;
        }
        else
        {
            if (P->prior)
                predassignment = P->prior->getAssignmentByTask(predid);
            if (predassignment && P->prior->getLine(predid) >= P->getOverwritepos(predassignment->pid))
                predpeid[i] = predassignment->pid;
            else
                predpeid[i] = pnum;

            predfinishtime[i] = 0;
            predcommcost[i] = e->weight;
        }
    }

    // try each pe as target
    int placeable = 0;
    for (uint i = 0u; i < pnum; i++)
    {
        pe = P->getpe(i);
        if (P->checkPlaceable(G, i, taskid))
            placeable = 1;
        else
            continue;
        bigint avail = pe->opeft;
        for (uint j = 0u; j < prednum; j++)
        {
            bigint predt = predfinishtime[j] + (CommWeight(getCommLevel(pnum, i, predpeid[j])));
            avail = avail > predt ? avail : predt;
        }
        if (avail < est)
        {
            est = avail;
            pid = i;
        }
    }
    if (placeable)
        P->assignTask(G, taskid, pid, est, est + v->weight);
    else
        return UINT_MAX;
    return pid;
}

Schedule clusterScheduling(BooleanDag *dag, std::vector<std::vector<uint>> &clusters, int workload)
{
    // init
    Schedule sche;
    sche.chunksize = workload;
    int pnum = MESHSIZE / workload;
    if (pnum <= 0)
        exit(-1);
    bigint totalms = 0;

    // get clusters
    std::vector<std::vector<uint>> clusters;
    linearClustering(dag, clusters);

    // calc latency
    std::vector<bigint> latencies(clusters.size()) for (uint i = 0; i < clusters.size(); i++)
        latencies[i] = getLatency(dag, clusters[i]);

    // sort clusters by latency in descending order
    auto _cmp = [](std::pair<bigint, uint> a, std::pair<bigint, uint> b)
    { return a.first > b.first; };
    std::priority_queue<std::pair<bigint, uint>, std::vector<std::pair<bigint, uint>>, decltype(_cmp)> q(_cmp);
    for (uint i = 0; i < clusters.size(); i++)
        q.push(std::make_pair(latencies[i], i));
    // place lace clusters in decending order of latency
    // how?

    // clusters vector2set for fast search
    std::vector<std::set<uint>> clusters_set(clusters.size());
    for (uint i = 0; i < clusters.size(); i++)
        for (uint j = 0; j < clusters[i].size(); j++)
            clusters_set[i].insert(clusters[i][j]);

    // task placement, tasks in the same cluster are placed in the same PE

    StageProcessors *stages = NULL;
    StageProcessors *prior = NULL;
    StageProcessors **p = &stages;
    (*p)->init(pnum);
    uint stagecnt = 0;

    int size = dag->getsize();
    std::vector<uint> cluster2pe(clusters.size(), -1);
    std::vector<int> indegree(size, 0);
    std::queue<uint> q;

    bool *assigned = new bool[size];
    for (uint i = 0; i < size; i++)
        assigned[i] = false;

    // place "free" tasks
    for (uint i = 0; i < size; i++)
    {
        Vertice *now = dag->getvertice(i);
        indegree[i] = now->prednum;
        if (indegree[i] == 0)
            q.push(i);
    }
    while (!q.empty())
    {
        uint u = q.front();
        q.pop();

        for (uint i = 0; i < dag->getvertice(u)->succnum; i++)
        {
            uint v = dag->getvertice(u)->successors[i]->dest->id;
            indegree[v]--;
            if (indegree[v] == 0)
                q.push(v);
        }
        // if the node is in a cluster already placed in a pe, place it in the same pe
        if (cluster2pe[u] != -1)
        {
            int pnow = cluster2pe[u];
            // calc est

            prior = *p;
            p = &((*p)->next);
        }
        else
        {
            // if not, place it in the pe with the earliest available time
            int pid = placeAtEarleast(G, *p, taskid);
            if (pid != UINT_MAX)
            {
                assigned[taskid] = true;
                (*p)->releaseMem(G, taskid, assigned);
                // tag all tasks in the same cluster
                for (int i = 0; i < clusters.size(); i++)
                    if (clusters_set[i].find(u) != clusters_set[i].end())
                    {
                        for (auto it : clusters_set[i])
                            cluster2pe[it] = pid;
                        break;
                    }
                (*p)->assignFinish();
            }
            prior = *p;
            p = &((*p)->next);
        }
    }
    sche.p = stages;
    sche.latency = 0;
    sche.oplatency = 0;
    sche.energy = 0;
    while (stages)
    {
        stages->calcEnergy();
        sche.latency += stages->getLatency();
        sche.oplatency += stages->getOPLatency();
        sche.energy += stages->getEnergy();

        stages = stages->next;
    }
    // stages->printScheduleByTasks();
    // stages->printScheduleByPEs();

    return sche;
}