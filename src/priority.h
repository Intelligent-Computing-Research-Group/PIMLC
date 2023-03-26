/**  
 * @file    priority.h
 * @brief   Compute the priority of each vertice in a DAG
 * @author  Chenu Tang
 * @version 2.0
 * @date    2022-11-09
 * @note    
 */
#ifndef _PRIORITY_
#define _PRIORITY_
#include "booleandag.h"
namespace Priority {
    /* Basic Functions */
    bigint* ranku(BooleanDag *g, bigint *value=NULL);     ///< calculate the upward rank of each vertice in a DAG (static usually)
    bigint* bleavel(BooleanDag *g, bigint *value=NULL);   ///< calculate the bottom level of each vertice in a DAG - same as ranku (static usually)
    bigint* rankd(BooleanDag *g, bigint *value=NULL);     ///< calculate the downward rank of each vertice in a DAG
    bigint* tleavel(BooleanDag *g, bigint *value=NULL);   ///< calculate the top level of each vertice in a DAG - same as rankd

    /* Recursive Functions */
    bigint ranku(BooleanDag *g, uint id, bigint *value);      ///< calculate the upward rank of a vertice in a DAG in a recursive way
    bigint bleavel(BooleanDag *g, uint id, bigint *value);    ///< calculate the bottom level of a vertice in a DAG in a recursive way
    bigint rankd(BooleanDag *g, uint id, bigint *value);      ///< calculate the downward rank of a vertice in a DAG in a recursive way
    bigint tleavel(BooleanDag *g, uint id, bigint *value);    ///< calculate the top level of a vertice in a DAG in a recursive way
}
#endif