#include <cstdio>
#include "coin/OsiClpSolverInterface.hpp"
#include "coin/CbcModel.hpp"
#include "coin/CoinModel.hpp"
#include <cmath>

#define LOG2(x) (x&0x80000000?31:(x&0x40000000?30:(x&0x20000000?29:(x&0x10000000?28:(x&0x08000000?27:(x&0x04000000?26:(x&0x02000000?25:(x&0x01000000?24: \
                (x&0x00800000?23:(x&0x00400000?22:(x&0x00200000?21:(x&0x00100000?20:(x&0x00080000?19:(x&0x00040000?18:(x&0x00020000?17:(x&0x00010000?16: \
                (x&0x00008000?15:(x&0x00004000?14:(x&0x00002000?13:(x&0x00001000?12:(x&0x00000800?11:(x&0x00000400?10:(x&0x00000200?9:(x&0x00000100?8: \
                (x&0x00000080?7:(x&0x00000040?6:(x&0x00000020?5:(x&0x00000010?4:(x&0x00000008?3:(x&0x00000004?2:(x&0x00000002?1:(x&0x00000001?0:-1))))))))))))))))))))))))))))))))

int main() {
    const int numcols = 7;
    const int numrows = 1;
    double obj[] = {100, 180, 320, 560, 960, 1600, 2400}; // obj: Max 100x0 + 180x1 + 320x2 + 560x3 + 960x4 + 1600x5 + 2400x6

    // Column-major sparse "A" matrix
    int start[] = {0, 1, 2, 3, 4, 5, 6, 7};      // where in index columns start
    int index[] = {0, 0, 0, 0, 0, 0, 0}; // row indexs for the columns
    double values[] = {-1, -2, -4, -8, -16, -32, -64}; // the values in the sparse matrix
    double rowlb[]  = {-COIN_DBL_MAX};  // null -> -inf
    double rowub[]  = {-300};
    // xi <= 0
    double collb[] = {0, 0, 0, 0, 0, 0, 0};
    double colub[] = {300, 150, 75, 38, 19, 10, 5};


    OsiClpSolverInterface model;
    model.setLogLevel(0);
    model.loadProblem(numcols, numrows, start, index, values, 
                        collb, colub, obj, NULL, rowub);
    // model.loadProblem(numcols, numrows, start, index, values, 
    //                     collb, NULL, obj, NULL, rowub);
    model.setInteger(0); // Sets x0 to integer
    model.setInteger(1); // Sets x1 to integer
    model.setInteger(2); // Sets x2 to integer
    model.setInteger(3); // Sets x3 to integer
    model.setInteger(4); // Sets x4 to integer
    model.setInteger(5); // Sets x5 to integer
    model.setInteger(6); // Sets x6 to integer
    model.setObjSense(1.0); // Minimise

    CbcModel solver(model);
    solver.setLogLevel(0);
    solver.branchAndBound();
    bool optimal = solver.isProvenOptimal();
    const double *val = solver.getColSolution();
    double res = solver.getObjValue();
    printf("Solution res: %g, %g %g %g %g %g %g %g %d\n", res, val[0], val[1], val[2], val[3], val[4], val[5], val[6], optimal ? 1 : 0);

    return 0;
}