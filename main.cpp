#include "./src/pimconfig.h"
#include "./src/booleandag.h"
#include "./src/scheduler.h"
#include "./src/importdag.h"
#include "./ILP/ILP.h"
#include <cstdio>
#include <ctime>
#include <string>

using namespace std;

/**
 * @brief main
 * 
 * @param argc arg count
 * @param argv 0:executable file name, 1:inputfile(*.v) 2:workloadsize
 * @return int 
 */
int main(int argc, char *argv[])
{
    pcfg.setGlobalPIMConf(argv[2]);
    string algorithm = "LBCP";  // defalt: LBCP
    if (argc < 3) {
        return 0;
    }
    if (argc >= 4) {
        algorithm = argv[4];
    }
    clock_t begintime;
    begintime = clock();
    const char *inputfile = argv[1];
    uint workload = atoi(argv[3]);
    uint size = (workload+pcfg.chip_num-1) / pcfg.chip_num;
    uint Bsize = (size + pcfg.block_cols - 1) / pcfg.block_cols;
    uint blocknums = pcfg.block_nums;
    uint max_pimthreads = pcfg.max_threads;

    BooleanDag *G = v2booleandag(inputfile);

    // uint top = (Bsize < blocknums ? Bsize : blocknums);
    uint searchbound = LOG2(blocknums);
    if (NPOWEROF2(searchbound) < Bsize && NPOWEROF2(searchbound) < blocknums) ++searchbound;
    // printf("Bsize:%d, searchbound:%d\n", Bsize, searchbound);

    Schedule *sche = new Schedule[LOG2(blocknums)+1];
    double *cost = new double[LOG2(blocknums)+1];

    for (uint i = 0u; i <= searchbound; ++i) {
        if (blocknums/NPOWEROF2(i) > max_pimthreads) {
            cost[i] = 1e20;
            continue;
        }
        if (algorithm == "HEFT") {
            sche[i] = rankuHEFTSchedule(G, NPOWEROF2(i));
        }
        else if (algorithm == "DW") {
            sche[i] = rankuDynamicWeightsSchedule(G, NPOWEROF2(i));
        }
        else {
            sche[i] = rankuCPDynamicWeightsSchedule(G, NPOWEROF2(i));
        }
        cost[i] = sche[i].latency;
    }

    CutSolver solver(Bsize, searchbound+1, cost);

    bool optimal = solver.isOptimal();
    const double *val = solver.getSolution();
    double res = solver.getObjValue();
    // printf("cost: %g %g %g %g %g %g %g\n", cost[0], cost[1], cost[2], cost[3], cost[4], cost[5], cost[6]);
    // printf("Solution res: %g, %g %g %g %g %g %g %g %d\n", res, val[0], val[1], val[2], val[3], val[4], val[5], val[6], optimal ? 1 : 0);

    uint offset = 0;
    uint chunksize;
    bigint latency = 0;
    double energy = 0.0;
    bigint simdlatency = 0;
    double simdenergy = 0.0;

    printf("# compileCPUTime %ldms\n", (clock()-begintime) / (CLOCKS_PER_SEC/1000));
    printf("# blockrows %u\n", pcfg.block_rows);
    printf("# blockcols %u\n", pcfg.block_cols);
    printf("# data %u\n", size);
    printf("# input %u\n", G->getinputsize());
    printf("# output %u\n", G->getoutputsize());
    for (uint i = 0u; i <= searchbound; ++i) {
        chunksize = 1 << i;
        for (uint j = 0; j < uint(val[i]+0.00001); ++j) {
            printInst(sche+i, offset, chunksize);
            offset += chunksize;
            // latency += sche[i].latency;
            // energy += sche[i].energy;
            // printf("latency: %lld, energy: %lf\n", sche[i].latency, sche[i].energy);
        }
    }


    // if SIMD
    // uint ms = blocknums * pcfg.block_cols;
    // size = ((size + ms - 1) / ms) * ms;
    // Bsize = (size + pcfg.block_cols - 1) / pcfg.block_cols;
    // if (searchbound < LOG2(blocknums)) {
    //     searchbound = LOG2(blocknums);
    //     sche[searchbound] = rankuHEFTSchedule(G, blocknums);
    //     cost[searchbound] = sche[searchbound].latency;
    // }
    
    // for (uint j = 0; j < Bsize/blocknums; ++j) {
    //     // printInst(sche+i, offset, chunksize);
    //     // offset += chunksize;

    //     // simdlatency += sche[searchbound].latency;
    //     // simdenergy += sche[searchbound].energy;
    //     // printf("latency: %lld, energy: %lf\n", sche[i].latency, sche[i].energy);
    // }

    // printf("%lld,%lf,%lld,%lf\n", latency / 1000ll, energy / 1000.0, simdlatency / 1000ll, simdenergy / 1000.0);
    // printf("final latency: %lld ns, energy: %lf nJ\n", latency / 1000ll, energy / 1000.0);
    // printf("simd  latency: %lld ns, energy: %lf nJ\n", simdlatency / 1000ll, simdenergy / 1000.0);

    delete[] cost;
    delete[] sche;
    return 0;
}


