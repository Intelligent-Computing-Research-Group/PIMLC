/**  
 * @file    ILP.cpp
 * @brief   Wrappers for coin-or Cbc
 * @author  Chenu Tang
 * @version 2.3
 * @date    2022-11-18
 * @note    
 */

#include "ILP.h"

CutSolver::CutSolver()
{
    model = NULL;
    solver = NULL;
    numcols = 0;
    obj = NULL;
    start = NULL;
    index = NULL;
    values = NULL;
    rowub = NULL;
    collb = NULL;
    colub = NULL;
}

CutSolver::CutSolver(const double workload, const uint cols, const double *w)
{
    model = new OsiClpSolverInterface;
    numcols = cols;
    obj = new double[cols];
    start = new int[cols+1];
    index = new int[cols];
    values = new double[cols];
    rowub = new double;
    collb = new double[cols];
    colub = new double[cols];

    for (int i = 0; i < cols; ++i) {
        obj[i] = w[i];
        start[i] = i;
        index[i] = 0;
        values[i] = -NPOWEROF2(i);
        collb[i] = 0;
        colub[i] = (workload-values[i]-1) / (-values[i]);
    }
    start[cols] = cols;
    *rowub = -workload; // ∑ki2^i >= n --> ∑-ki2^i <= -n
    model->setLogLevel(0);
    model->loadProblem(numcols, 1, start, index, values, 
        collb, colub, obj, NULL, rowub);

    for (uint i = 0; i < cols; ++i) {
        model->setInteger(i);
        model->setObjSense(1.0);    // Minimise costs
    }

    solver = new CbcModel(*model);
    solver->setLogLevel(0);
    solver->branchAndBound();
}

CutSolver::~CutSolver()
{
    if (solver) delete solver;
    if (model) delete model;
    if (obj) delete obj;
    if (start) delete start;
    if (index) delete index;
    if (values) delete values;
    if (rowub) delete rowub;
    if (collb) delete collb;
    if (colub) delete colub;
}

// Visitors
const double * CutSolver::getSolution()
{
    if (solver) {
        return solver->getColSolution();
    }
    return NULL;
}

double CutSolver::getObjValue()
{
    if (solver) {
        return solver->getObjValue();
    }
    return 0.0;
}

bool CutSolver::isOptimal()
{
    if (solver) {
        return solver->isProvenOptimal();
    }
    return false;
}