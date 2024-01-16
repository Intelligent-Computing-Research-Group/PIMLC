#include "../src/pimconfig.h"
#include "../src/booleandag.h"
#include "../src/scheduler.h"
#include "../src/importdag.h"
#include "../ILP/ILP.h"
#include <cstdio>
#include <ctime>

using namespace std;

/**
 * @brief main
 * 
 * @param argc arg count
 * @param argv 0:executable file name, 1:inputfile(*.v) 2:workloadsize
 * @return int 
 */


double maxutil(const std::map<bigint, uint> &util)
{
    static uint blocknums = pcfg.block_nums;
    double totalsubarrays = blocknums * pcfg.block_rows;
    uint maxsubarrs = 0u;
    for (std::map<bigint, uint>::const_iterator it = util.begin(); it != util.end(); ++it) {
        maxsubarrs = maxsubarrs > it->second ? maxsubarrs : it->second;
    }
    return (double)(maxsubarrs) / totalsubarrays;
}

double avgutil(const std::map<bigint, uint> &util)
{
    static uint blocknums = pcfg.block_nums;
    double totalsubarrays = blocknums * pcfg.block_rows;
    bigint timeprodsubarrs = 0ll;
    std::map<bigint, uint>::const_iterator it = util.begin();
    bigint lasttime = it->first;
    uint lastsubarrs = it->second;
    for (++it; it != util.end(); ++it) {
        bigint dt = it->first - lasttime;
        if (dt < 0ll) dt = -dt;
        timeprodsubarrs += (lastsubarrs + it->second) * dt / 2;
        lasttime = it->first;
        lastsubarrs = it->second;
    }

    return (timeprodsubarrs / totalsubarrays) / (double)(lasttime);
}


char* getBenchmarkName(const char *filepath)
{
    const char *start;
    const char *end = filepath;
    while ((*end) != '\0') ++end;
    while ((*end) != '.') --end;
    start = end;
    while (start!=filepath && *(start-1)!='/' && *(start-1)!='\\') --start;
    char *res = new char[end-start+1];
    char *p=res;
    while (start!=end) {
        *(p++)=*(start++);
    }
    *p = '\0';
    return res;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        return 0;
    }
    pcfg.setGlobalPIMConf(argv[2]);
    srand((unsigned)time(NULL));
    clock_t begintime;
    begintime = clock();
    const char *inputfile = argv[1];
    uint testnum = atoi(argv[3]);
    uint blocknums = pcfg.block_nums;
    uint max_pimthreads = pcfg.max_threads;

    BooleanDag *G = v2booleandag(inputfile);
    bigint opcnt = G->getsize() - G->getinputsize() - 1;
    char *benchmark = getBenchmarkName(inputfile);
    uint searchbound = LOG2(blocknums);
    // printf("Bsize:%d, searchbound:%d\n", Bsize, searchbound);
    
    Schedule *sche = new Schedule[searchbound+1];
    double *cost = new double[searchbound+1];
    clock_t *endtime = new clock_t[searchbound+1];

    for (uint i = 0u; i <= searchbound; ++i) {
        if (blocknums/NPOWEROF2(i) > max_pimthreads) {
            cost[i] = 1e20;
            endtime[i] = clock();
            continue;
        }
        sche[i] = rankuCPDynamicWeightsSchedule(G, NPOWEROF2(i));
        cost[i] = sche[i].latency;
        endtime[i] = clock();
    }

    for (uint i = 0; i < testnum; ++i) {
        clock_t ilpbegintime = clock();
        uint workload = atoi(argv[4+i]);
        uint size = (workload+pcfg.chip_num-1) / pcfg.chip_num;
        uint Bsize = (size + pcfg.block_cols - 1) / pcfg.block_cols;
        uint top = (Bsize < blocknums ? Bsize : blocknums);
        CutSolver *solver = new CutSolver(Bsize, searchbound+1, cost);

        bool optimal = solver->isOptimal();
        const double *val = solver->getSolution();
        double res = solver->getObjValue();
        clock_t ilpendtime = clock();
        // printf("cost: %g %g %g %g %g %g %g\n", cost[0], cost[1], cost[2], cost[3], cost[4], cost[5], cost[6]);
        // printf("Solution res: %g, %g %g %g %g %g %g %g %d\n", res, val[0], val[1], val[2], val[3], val[4], val[5], val[6], optimal ? 1 : 0);

        uint offset = 0;
        uint chunksize;
        bigint latency = 0; // ps
        bigint iolatency = 0;
        bigint oplatency = 0;
        double energy = 0.0;    // pJ
        double ioenergy = 0.0;    // pJ
        bigint simdlatency = 0;
        bigint simdiolatency = 0;
        bigint simdoplatency = 0;
        double simdenergy = 0.0;
        double simdioenergy = 0.0;
        bigint operations = (bigint)(workload);
        double throughput;
        double simdthroughput;
        double efficiency;
        double simdefficiency;


        std::map<bigint, uint> spatialutil, simdspatialutil;
        double temporalutil = 0.0;
        double simdtemporalutil = 0.0;
        double maxspatialutil = 0.0;
        double avgspatialutil = 0.0;
        double simdmaxspatialutil = 0.0;
        double simdavgspatialutil = 0.0;

        for (uint k = 0u; k <= searchbound; ++k) {
            chunksize = 1 << k;
            for (uint j = 0; j < uint(val[k]+0.000001); ++j) {
                // printInst(sche+k, offset, chunksize);
                // offset += chunksize;
                spatialutil.insert(std::make_pair(latency, 0u));
                sche[k].p->getTime2SpatialUtil(spatialutil, latency);
                latency += sche[k].latency;
                iolatency += sche[k].iolatency;
                oplatency += sche[k].oplatency;
                energy += sche[k].energy;
                ioenergy += sche[k].ioenergy;
                // printf("latency: %lld, energy: %lf\n", sche[k].latency, sche[k].energy);
            }
        }

        // if SIMD
        uint ms = blocknums * pcfg.block_cols;
        size = ((size + ms - 1) / ms) * ms;
        Bsize = (size + pcfg.block_cols - 1) / pcfg.block_cols;
        if (searchbound < LOG2(blocknums)) {
            searchbound = LOG2(blocknums);
            sche[searchbound] = rankuCPDynamicWeightsSchedule(G, blocknums);
            cost[searchbound] = sche[searchbound].latency;
        }
        for (uint j = 0; j < Bsize/blocknums; ++j) {
            // printInst(sche+i, offset, chunksize);
            // offset += chunksize;
            simdspatialutil.insert(std::make_pair(simdlatency, 0u));
            sche[searchbound].p->getTime2SpatialUtil(simdspatialutil, simdlatency);
            simdlatency += sche[searchbound].latency;
            simdiolatency += sche[searchbound].iolatency;
            simdoplatency += sche[searchbound].oplatency;
            simdenergy += sche[searchbound].energy;
            simdioenergy += sche[searchbound].ioenergy;
            // printf("latency: %lld, energy: %lf\n", sche[i].latency, sche[i].energy);
        }

        energy *= pcfg.chip_num;
        simdenergy *= pcfg.chip_num;

        throughput = (double)(operations*1000ll)/(double)(latency); // OP/ns = GOPS
        simdthroughput = (double)(operations*1000ll)/(double)(simdlatency);
        efficiency = (double)(operations)/(energy);    // OP/pJ = TOP/J = TOPS/W
        simdefficiency = (double)(operations)/(simdenergy);
        temporalutil = (double)(oplatency)/(double)(latency);
        simdtemporalutil = (double)(simdoplatency)/(double)(simdlatency);
        maxspatialutil = maxutil(spatialutil);
        avgspatialutil = avgutil(spatialutil);
        simdmaxspatialutil = maxutil(simdspatialutil);
        simdavgspatialutil = avgutil(simdspatialutil);

        printf("%d*%s,", pcfg.chip_num, pcfg.name.c_str());
        printf("%d*4*4*%d*%d,%s,%s,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%ld\n", 
            pcfg.level_size[pcfg.levels-1], pcfg.block_rows, pcfg.block_cols, 
            benchmark, argv[4+i], 
            (double)(latency) / 1000.0, energy / 1000.0, 
            (double)(simdlatency) / 1000.0, simdenergy / 1000.0, 
            (double)(iolatency) / 1000.0, (double)(simdiolatency) / 1000.0, 
            (double)(oplatency) / 1000.0, (double)(simdoplatency) / 1000ll, 
            (double)(ioenergy) / 1000.0, (double)(simdioenergy) / 1000.0, 
            operations, throughput, simdthroughput, 
            efficiency, simdefficiency, 
            temporalutil, simdtemporalutil, 
            maxspatialutil, avgspatialutil, 
            simdmaxspatialutil, simdavgspatialutil, endtime[LOG2(top)]-begintime+ilpendtime-ilpbegintime);

        // printf("%s,%lf,%lf\n",argv[4+i],(double)(latency)/1000.0,oldcost[i]/(double)(latency)*1000.0);
        // printf("final latency: %lld ns, energy: %lf nJ\n", latency / 1000ll, energy / 1000.0);
        // printf("simd  latency: %lld ns, energy: %lf nJ\n", simdlatency / 1000ll, simdenergy / 1000.0);
        delete solver;
    }

    delete[] benchmark;
    delete[] cost;
    delete[] sche;
    delete[] endtime;
    return 0;
}
