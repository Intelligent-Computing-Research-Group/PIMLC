/**  
 * @file    booleandag.cpp
 * @brief   Basic implementation of boolean dag
 * @author  Chenu Tang
 * @version 3.0
 * @date    2023-02-09
 * @note    
 */

#include <cstdio>
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