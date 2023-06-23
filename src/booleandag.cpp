/**  
 * @file    booleandag.cpp
 * @brief   Basic implementation of boolean dag
 * @author  Chenu Tang
 * @version 3.0
 * @date    2023-02-09
 * @note    
 */

#include <cstdio>
#include <iostream>
#include "booleandag.h"

///< WIP
int destroySingleEdge(Edge *e)
{
    delete e;
    return 1;
}

///< WIP
int cleanSingleVertice(Vertice *v)
{
    if (v->prednum) {

    }
    if (v->succnum) {
        
    }
    v->id = -1;
    v->type = NORMAL;
    v->weight = 0;
    return 1;
}


/* Constructor & Destructor */
BooleanDag::BooleanDag()
{
    V = NULL;
    size = 0u;
    inputsize = 0u;
    outputsize = 0u;
    prio = NULL;
}

BooleanDag::BooleanDag(uint n)
{
    size = n;
    if (!n) {
        V = NULL;
        prio = NULL;
    }
    else {
        V = new Vertice[n];
        prio = new bigint[n];
    }
}

BooleanDag::~BooleanDag()
{
    if (V) delete[] V;
    if (prio) delete[] prio;
}



/* Construction related */
int BooleanDag::init()
{
    if (size > 0) {
        destroy();
    }
    V = NULL;
    prio = NULL;
    size = 0u;
    return 1;
}

int BooleanDag::init(uint n, uint input, uint output)
{
    if (size > 0) {
        destroy();
    }
    size = n;
    inputsize = input;
    outputsize = output;
    if (!n) {
        V = NULL;
        prio = NULL;
    }
    else {
        V = new Vertice[n];
        prio = new bigint[n];
    }
    return 1;
}

///< WIP
int BooleanDag::destroy()
{
    if (size > 0) {
        delete[] V;
        E.clear();
        delete[] prio;
    }
    return 1;
}

int BooleanDag::addVertice(bigint weight, const std::string &s)
{
    for (uint i = 0; i < size; ++i) {
        if ((V+i)->weight < 0) {
            (V+i)->id = i;
            (V+i)->weight = weight;
            (V+i)->name = s;
            return 1;
        }
    }
    return 0;
}

int BooleanDag::addVertice(uint id, bigint weight, const std::string &s)
{
    Vertice *v = V+id;
    if (v->weight >= 0) {
        cleanSingleVertice(v);
    }
    v->id = id;
    v->weight = weight;
    v->name = s;
    return 1;
}

int BooleanDag::addVertice(uint id, bigint weight, VerticeType type, int inv, const std::string &s)
{
    Vertice *v = V+id;
    if (v->weight >= 0) {
        cleanSingleVertice(v);
    }
    v->type = type;
    v->id = id;
    v->weight = weight;
    v->name = s;
    v->invflags = inv;
    return 1;
}

int BooleanDag::addEdge(uint src, uint dest, bigint weight)
{
    Edge e;
    e.weight = weight;
    e.src = V+src;
    e.dest = V+dest;
    E.push_back(e);
    return 1;
}

int BooleanDag::linkDAG()
{
    int edgenum = (int)(E.size());
    std::vector<Edge>::iterator iter;
    for (iter = E.begin(); iter !=E.end(); ++iter) {
        iter->src->succnum++;
        iter->dest->prednum++;
    }

    Vertice *v;
    for (uint i = 0u; i < size; ++i) {
        v = V+i;
        if (v->prednum) {
            v->predecessors = new Edge*[v->prednum];
        }
        if (v->succnum) {
            v->successors = new Edge*[v->succnum];
        }
    }

    for (iter = E.begin(); iter !=E.end(); ++iter) {
        *(iter->src->successors++) = &(*iter);
        *(iter->dest->predecessors++) = &(*iter);
    }

    for (uint i = 0u; i < size; ++i) {
        v = V+i;
        v->predecessors -= v->prednum;
        v->successors -= v->succnum;
    }
    return 1;
}



/* Setters */
int BooleanDag::setPriority(bigint *arr)
{
    if (!prio)
        prio = new bigint[size];
    for (uint i = 0; i < size; ++i) {
        prio[i] = arr[i];
    }
    return 1;
}


/* Vistors */
uint const& BooleanDag::getsize() const
{
    return size;
}

uint const& BooleanDag::getoutputsize() const
{
    return outputsize;
}

uint const& BooleanDag::getinputsize() const
{
    return inputsize;
}

Vertice* BooleanDag::getvertice(uint id)
{
    return V+id;
}

bigint BooleanDag::getPriority(uint id)
{
    return prio[id];
}

int BooleanDag::getCriticalPath(std::set<uint> & res)
{
    uint size = getsize();
    int cnt = 0;

    // if there is only one node, output it
    if (size == 1) {
        res.insert(res.end(), 0);
        return 1;
    }
    std::vector<int> vis(size, 0);
    std::queue<uint> q;

    // find the nodes with no predecessors
    std::vector<int> indegree(size, 0);
    for (uint i = 0; i < size; i++) {
        indegree[i] = getvertice(i)->prednum;
        if (indegree[i] == 0)
            q.push(i);
    }

    int max_ve = -1;
    std::stack<uint> topo;
    std::vector<int> ve(size, 0);
    while (!q.empty()) {
        uint u = q.front();
        q.pop();
        topo.push(u);
        Vertice *now = getvertice(u);
        for (uint i = 0; i < now->succnum; i++) {
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
        Vertice *now = getvertice(u);
        for (uint i = 0; i < now->succnum; i++)
        {
            uint v = now->successors[i]->dest->id;
            vl[u] = std::min(vl[u], vl[v] - (int)now->weight - (int)now->successors[i]->weight);
        }
    }

    int k = 0;
    int e, l, found = 0;
    // find the first egde with e = l
    while (k < size && found == 0)
    {
        Vertice *now = getvertice(k);
        e = ve[k];
        for (uint i = 0; i < now->succnum; i++)
        {
            uint v = now->successors[i]->dest->id;
            l = vl[v] - now->weight - now->successors[i]->weight;
            // std::cout << now->name << " " << e << " " << l << std::endl;
            if (e == l) {
                found = 1, vis[k] = 1, vis[v] = 1;
                res.insert(res.end(), v);
                // std::cout << now->name << " " << getvertice(v)->name << " ";
                ++cnt;
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
        Vertice *now = getvertice(k);
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
                res.insert(res.end(), v);
                // std::cout << getvertice(v)->name << " ";
                ++cnt;
                k = v;
                break;
            }
        }
    }
    return cnt;
}




/// @brief 
/// @param cluster 
/// @param depth 
/// @return 
typedef struct MinDepthPreds {
    // uint id;
    uint mindepth;
    std::set<Vertice*> preds;
} MinDepthPreds;

// template <typename T>
// void clearqueue(std::queue<T> &q)
// {
//     std::queue<T> empty;
//     std::swap(q, empty);
// }

int BooleanDag::getMainCluster(std::set<uint> & cluster, uint depth)
{
    if (depth < 1) {
        return cluster.size();
    }
    if (cluster.size() < 1) {
        getCriticalPath(cluster);
    }
    std::vector<bool> visited(size, false);
    for (auto it = cluster.begin(); it != cluster.end(); ++it) {
        visited[*it] = true;
    }
    std::map<Vertice*, MinDepthPreds> records;
    std::set<uint> ext;
    std::queue<Vertice*> q;
    MinDepthPreds r;
    for (auto it = cluster.begin(); it != cluster.end(); ++it) {
        Vertice *v = V + *it;
        uint succnum = v->succnum;
        for (uint i = 0; i < succnum; ++i) {
            Vertice *succ = v->successors[i]->dest;
            if (cluster.find(succ->id) != cluster.end() || ext.find(succ->id) != ext.end() || records.find(succ) != records.end()) continue;
            records.insert(std::make_pair(succ, r));
            q.push(succ);
        }
    }
    do {
        /** childs of nodes in extend set from last loop **/
        for (auto it = ext.begin(); it != ext.end(); ++it) {
            Vertice *v = V + *it;
            uint succnum = v->succnum;
            for (uint i = 0; i < succnum; ++i) {
                Vertice *succ = v->successors[i]->dest;
                if (cluster.find(succ->id) != cluster.end() || ext.find(succ->id) != ext.end() || records.find(succ) != records.end()) continue;
                records.insert(std::make_pair(succ, r));
                q.push(succ);
            }
        }
        /** init the extend set **/
        ext.clear();
        r.mindepth = 0;

        ///< start from these nodes
        for (uint i = 0; i < depth; ++i) {
            uint qsize = q.size();
            for (uint j = 0; j < qsize; ++j) {
                Vertice *node = q.front();
                q.pop();
                uint nodesuccnum = node->succnum;
                for (uint k = 0; k < nodesuccnum; ++k) {
                    Vertice *succ = node->successors[k]->dest;
                    if (cluster.find(succ->id) != cluster.end() || ext.find(succ->id) != ext.end()) {
                        ///< found in cluster/ext, blongs to main cluster
                        ///< add its ancestors into ext set
                        std::queue<Vertice*> qpreds;
                        qpreds.push(node);
                        while (!qpreds.empty()) {
                            Vertice *cur = qpreds.front();
                            qpreds.pop();
                            ext.insert(cur->id);
                            auto rec = records.find(cur);
                            for (auto it = rec->second.preds.begin(); it != rec->second.preds.end(); ++it) {
                                qpreds.push(*it);
                            }
                        }
                    }
                    else if (i+1 < depth){
                        ///< else record it and push it into the queue for deeper search
                        auto it = records.find(succ);
                        if (it != records.end()) {
                            if (it->second.mindepth == i+1) {
                                it->second.preds.insert(node);
                            }
                        }
                        else {
                            MinDepthPreds r;
                            r.mindepth = i+1;
                            r.preds.insert(node);
                            records.insert(std::make_pair(succ, r));
                            q.push(succ);
                        }
                    }
                }
            }
        }
        cluster.insert(ext.begin(), ext.end());
        { std::queue<Vertice*> empty; std::swap(q, empty); };
        records.clear();
    } while(ext.size());
    return cluster.size();
}

void BooleanDag::traversePrint()
{
    Vertice *v;
    Edge *e;
    printf("--------Boolean Dag Traverse BEGIN--------\n\n");
    for (uint i = 0; i < size; ++i) {
        v = V+i;
        printf("[Vertice No.%u(%s)] weight:%lld\n", i, v->name.c_str(), v->weight);

        printf("\tPredecessors:");
        for (uint j = 0; j < v->prednum; ++j) {
            e = *(v->predecessors+j);
            printf(" %u(%lld)", e->src->id, e->weight);
        }
        printf("\n");

        printf("\tSuccessors:");
        for (uint j = 0; j < v->succnum; ++j) {
            e = *(v->successors+j);
            printf(" %u(%lld)", e->dest->id, e->weight);
        }
        printf("\n");

        printf("\n");
    }
    printf("--------Boolean Dag Traverse END--------\n");
}