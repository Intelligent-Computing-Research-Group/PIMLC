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

int getCriticalPath(BooleanDag *dag, std::vector<uint> &vis)
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
        std::cout << dag->getvertice(k)->name << std::endl;
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
                std::cout << now->name << " " << dag->getvertice(v)->name << " ";
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
                std::cout << dag->getvertice(v)->name << " ";
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
void getCriticalPath(BooleanDag *dag)
{
    int size = dag->getsize();

    // if there is only one node, output it
    if (size == 1)
    {
        std::cout << dag->getvertice(0)->name << std::endl;
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
                std::cout << now->name << " " << dag->getvertice(v)->name << " ";
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
                std::cout << dag->getvertice(v)->name << " ";
                k = v;
                break;
            }
        }
    }
    std::cout << std::endl;
    return;
}

void linearClustering(BooleanDag *dag)
{
    int size = dag->getsize();
    std::vector<uint> vis(size, 0);
    int cnt = 0;
    // cnt = getCriticalPath(dag, vis);
    // std::cout << cnt << std::endl;
    while (cnt < size)
    {
        int k = getCriticalPath(dag, vis);
        cnt += k;
        if (k == 0)
            break;
        // std::cout << cnt << std::endl;
    }
    return;
}