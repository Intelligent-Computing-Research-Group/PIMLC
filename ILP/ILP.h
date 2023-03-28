/**  
 * @file    ILP.h
 * @brief   Wrappers for coin-or Cbc
 * @author  Chenu Tang
 * @version 2.1
 * @date    2022-11-10
 * @note    
 */
#ifndef __ILP__
#define __ILP__
#include <cstdio>
#include "coin/OsiClpSolverInterface.hpp"
#include "coin/CbcModel.hpp"
#include "coin/CoinModel.hpp"

#define LOG2(x) (x&0x80000000?31:(x&0x40000000?30:(x&0x20000000?29:(x&0x10000000?28:(x&0x08000000?27:(x&0x04000000?26:(x&0x02000000?25:(x&0x01000000?24: \
                (x&0x00800000?23:(x&0x00400000?22:(x&0x00200000?21:(x&0x00100000?20:(x&0x00080000?19:(x&0x00040000?18:(x&0x00020000?17:(x&0x00010000?16: \
                (x&0x00008000?15:(x&0x00004000?14:(x&0x00002000?13:(x&0x00001000?12:(x&0x00000800?11:(x&0x00000400?10:(x&0x00000200?9:(x&0x00000100?8: \
                (x&0x00000080?7:(x&0x00000040?6:(x&0x00000020?5:(x&0x00000010?4:(x&0x00000008?3:(x&0x00000004?2:(x&0x00000002?1:(x&0x00000001?0:-1))))))))))))))))))))))))))))))))

#define NPOWEROF2(x) (1<<x)

class CutSolver {
private:
    OsiClpSolverInterface *model;
    CbcModel *solver;
    int numcols;
    double *obj;
    int *start;
    int *index;
    double *values;
    double *rowub;
    double *collb;
    double *colub;
public:
    CutSolver();
    CutSolver(const double workload, const uint cols, const double *w);
    ~CutSolver();

    // Visitors
    const double *  getSolution();
    double  getObjValue();
    bool    isOptimal();
};

#endif