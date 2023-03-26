/**  
 * @file    priority.cpp
 * @brief   Compute the priority of each vertice in a DAG
 * @author  Chenu Tang
 * @version 4.0
 * @date    2022-10-17
 * @note    
 */

#include "priority.h"
#include "booleandag.h"
#include <cstdio>

bigint *Priority::ranku(BooleanDag *g, bigint *value)
{
    int size = g->getsize();
    if (!value) {
        value = new bigint[size];
    }
    for (uint i = 0u; i < size; ++i) {
        *(value+i) = 1;
    }
    for (uint i = 0u; i < size; ++i) {
        if (value[i] > 0) {
            ranku(g, i, value);
        }
    }
    return value;
}

bigint Priority::ranku(BooleanDag *g, uint id, bigint *value)
{
    if (value[id] <= 0) {
        return value[id];
    }
    Vertice *v = g->getvertice(id);
    if (v->succnum == 0) {
        return (value[id] = -v->weight);
    }
    bigint max = v->weight;
    bigint currank;
    Edge *e;
    for (uint i = 0; i < v->succnum; ++i) {
        e = *(v->successors+i);
        currank = v->weight + e->weight - ranku(g, e->dest->id, value);
        max = max > currank ? max : currank;
    }
    return (value[id] = -max);
}

bigint *Priority::rankd(BooleanDag *g, bigint *value)
{
    int size = g->getsize();
    if (!value) {
        value = new bigint[size];
    }
    for (uint i = 0u; i < size; ++i) {
        *(value+i) = -1;
    }
    for (uint i = 0u; i < size; ++i) {
        rankd(g, i, value);
    }
    return value;
}

bigint Priority::rankd(BooleanDag *g, uint id, bigint *value)
{
    if (value[id] >= 0) {
        return value[id];
    }
    Vertice *v = g->getvertice(id);
    if (v->prednum == 0) {
        return (value[id] = 0);
    }
    bigint max = 0;
    bigint currank;
    Edge *e;
    for (uint i = 0; i < v->prednum; ++i) {
        e = *(v->predecessors+i);
        currank = e->src->weight + e->weight + rankd(g, e->src->id, value);
        max = max > currank ? max : currank;
    }
    return (value[id] = max);
}
