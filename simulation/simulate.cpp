#include "../src/memory.h"
#include "../src/booleandag.h"
#include "../src/scheduler.h"
#include "../src/importdag.h"
#include "../ILP/ILP.h"
#include <cstdio>

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
    double totalsubarrays = MESHSIZE * BLOCKROW;
    uint maxsubarrs = 0u;
    for (std::map<bigint, uint>::const_iterator it = util.begin(); it != util.end(); ++it) {
        maxsubarrs = maxsubarrs > it->second ? maxsubarrs : it->second;
    }
    return (double)(maxsubarrs) / totalsubarrays;
}

double avgutil(const std::map<bigint, uint> &util)
{
    double totalsubarrays = MESHSIZE * BLOCKROW;
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
    const char *inputfile = argv[1];
    uint testnum = atoi(argv[2]);

    BooleanDag *G = v2booleandag(inputfile);
    bigint opcnt = G->getsize() - G->getinputsize() - 1;
    char *benchmark = getBenchmarkName(inputfile);
    uint searchbound = LOG2(MESHSIZE);
    // printf("Bsize:%d, searchbound:%d\n", Bsize, searchbound);
    
    Schedule *sche = new Schedule[searchbound+1];
    double *cost = new double[searchbound+1];

    for (uint i = 0u; i <= searchbound; ++i) {
        sche[i] = rankuDynamicWeightsSchedule(G, NPOWEROF2(i));
        cost[i] = sche[i].latency;
    }

    for (uint i = 0; i < testnum; ++i) {
        uint size = atoi(argv[3+i]);
        uint Bsize = (size + BLOCKCOL - 1) / BLOCKCOL;
        CutSolver *solver = new CutSolver(Bsize, searchbound+1, cost);

        bool optimal = solver->isOptimal();
        const double *val = solver->getSolution();
        double res = solver->getObjValue();
        // printf("cost: %g %g %g %g %g %g %g\n", cost[0], cost[1], cost[2], cost[3], cost[4], cost[5], cost[6]);
        // printf("Solution res: %g, %g %g %g %g %g %g %g %d\n", res, val[0], val[1], val[2], val[3], val[4], val[5], val[6], optimal ? 1 : 0);

        uint offset = 0;
        uint chunksize;
        bigint latency = 0;
        bigint oplatency = 0;
        double energy = 0.0;
        bigint simdlatency = 0;
        bigint simdoplatency = 0;
        double simdenergy = 0.0;
        bigint operations = (bigint)(size)*opcnt;
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

        for (uint i = 0u; i <= searchbound; ++i) {
            chunksize = 1 << i;
            for (uint j = 0; j < uint(val[i]+0.000001); ++j) {
                // printInst(sche+i, offset, chunksize);
                // offset += chunksize;
                spatialutil.insert(std::make_pair(latency, 0u));
                sche[i].p->getTime2SpatialUtil(spatialutil, latency);
                latency += sche[i].latency;
                oplatency += sche[i].oplatency;
                energy += sche[i].energy;
                // printf("latency: %lld, energy: %lf\n", sche[i].latency, sche[i].energy);
            }
        }


        // if SIMD
        uint ms = MESHSIZE * BLOCKCOL;
        size = ((size + ms - 1) / ms) * ms;
        Bsize = (size + BLOCKCOL - 1) / BLOCKCOL;
        if (searchbound < LOG2(MESHSIZE)) {
            searchbound = LOG2(MESHSIZE);
            sche[searchbound] = rankuHEFTSchedule(G, MESHSIZE);
            cost[searchbound] = sche[searchbound].latency;
        }
        
        for (uint j = 0; j < Bsize/MESHSIZE; ++j) {
            // printInst(sche+i, offset, chunksize);
            // offset += chunksize;
            simdspatialutil.insert(std::make_pair(simdlatency, 0u));
            sche[searchbound].p->getTime2SpatialUtil(simdspatialutil, simdlatency);
            simdlatency += sche[searchbound].latency;
            simdoplatency += sche[searchbound].oplatency;
            simdenergy += sche[searchbound].energy;
            // printf("latency: %lld, energy: %lf\n", sche[i].latency, sche[i].energy);
        }
        throughput = (double)(operations*1000ll)/(double)(latency);
        simdthroughput = (double)(operations*1000ll)/(double)(simdlatency);
        efficiency = (double)(operations)*(double)(latency)/(energy*1000.0);
        simdefficiency = (double)(operations)*(double)(simdlatency)/(simdenergy*1000.0);
        temporalutil = (double)(oplatency)/(double)(latency);
        simdtemporalutil = (double)(simdoplatency)/(double)(simdlatency);
        maxspatialutil = maxutil(spatialutil);
        avgspatialutil = avgutil(spatialutil);
        simdmaxspatialutil = maxutil(simdspatialutil);
        simdavgspatialutil = avgutil(simdspatialutil);

#ifdef RRAM
        printf("RRAM,");
#else
        printf("SRAM,");
#endif
        printf("%d*4*4*%d*%d,%s,%s,%lf,%lf,%lf,%lf,%lf,%lf,%lld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n", 
            BANKNUM, BLOCKROW, BLOCKCOL, 
            benchmark, argv[3+i], 
            (double)(latency) / 1000.0, energy / 1000.0, 
            (double)(simdlatency) / 1000.0, simdenergy / 1000.0, 
            (double)(oplatency) / 1000.0, (double)(simdoplatency) / 1000ll, 
            operations, throughput, simdthroughput, 
            efficiency, simdefficiency, 
            temporalutil, simdtemporalutil, 
            maxspatialutil, avgspatialutil, 
            simdmaxspatialutil, simdavgspatialutil);
        // printf("final latency: %lld ns, energy: %lf nJ\n", latency / 1000ll, energy / 1000.0);
        // printf("simd  latency: %lld ns, energy: %lf nJ\n", simdlatency / 1000ll, simdenergy / 1000.0);
        delete solver;
    }

    delete[] benchmark;
    delete[] cost;
    delete[] sche;
    return 0;
}


