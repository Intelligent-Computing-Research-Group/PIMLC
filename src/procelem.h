/**  
 * @file    procelem.h
 * @brief   Processing Element Definition
 * @author  Chenu Tang
 * @version 2.3
 * @date    2022-11-18
 * @note    
 */
#ifndef _PROCELEM_
#define _PROCELEM_
#include <map>
#include <vector>
#include <deque>
#include <cstdio>
#include "instruction.h"
#include "booleandag.h"


class StageProcessors;
typedef struct Assignment {
    uint pid;   ///< pe id
    uint tid;   ///< task(vertice) id
    bigint starttime;      ///< starttime
    bigint finishtime;     ///< starttime
    Assignment() : pid(0), tid(0), starttime(0), finishtime(0) {};
} Assignment;


int checkPEAvail(); ///< If PE state allow a newly assigned task
int updatePEState();    ///< update PE state


///< @brief record real-time state of a single PE
typedef struct _PE {
    uint id;            ///< unique id
    std::vector<Assignment*>    tasks;  ///< Assigned tasks
    bigint opeft;            ///< earlist finish time
    std::map<uint, uint> cache;     ///< <taskid, index> record cached id of nodes
    uint line[BLOCKROW];            ///< index -> taskid
    uint smallestfreeidx;
    uint overwriteflag;
    _PE() : id(0), opeft(0), smallestfreeidx(0), overwriteflag(0) {for(uint i=0;i<BLOCKROW;++i)line[i]=UINT_MAX;};
} ProcessElem;



class StageProcessors {
    uint pnum;      ///< number of Processing Elements
    std::map<uint, Assignment> schedule;    ///< <taskid, Assignment>
    ProcessElem *PE;        ///< the array of Processing Elements (= grid rows)
    bigint *loadlatency;    ///< the load latency of each PE
    bigint *oplatency;      ///< the operation latency of each PE
    bigint *midlatency;     ///< the operation+copy latency of each PE
    bigint *storelatency;   ///< the store latency of each PE
    double *loadenergy;     ///< the load energy of each PE
    double *midenergy;      ///< the operation+copy energy of each PE
    double *storeenergy;    ///< the store energy of each PE
    std::deque<InstructionNameSpace::Instruction> inst;    ///< instruction lists

public:
    StageProcessors *next;  ///< next stage
    StageProcessors *prior; ///< prior stage
    /* Constructor & Destructor */
    StageProcessors();
    ~StageProcessors();

    /* Construction related */
    int init(uint n);  // unlimited memory for DAG with certain number of tasks
    int clean();

    /* TaskAssignment */
    
    /**
     * @brief int StageProcessors::checkPlaceable(BooleanDag *G, uint peid, uint taskid)
     * @details check if the task `taskid` can be assigned to the `peid` Processsing Element
     *            we shall check every single PE before we assign a task to it. 
     * @retval 1 OK
     * @retval 0 Wrong
     */
    int checkPlaceable(BooleanDag *G, uint peid, uint taskid);

    /**
     * @brief int StageProcessors::assignTask(BooleanDag* g, uint taskid, uint PEid, bigint starttime, bigint finishtime)
     * @details assign the task `taskid` to the PE `PEid`, the state of PE will be updated 
     *            and new instructions will be inserted into the inst list
     * @retval 1 OK
     * @retval 0 Wrong
     */
    int assignTask(BooleanDag* g, uint taskid, uint PEid, bigint starttime, bigint finishtime);
    int assignTaskSeek(BooleanDag* G, uint taskid, uint PEid, std::vector<InstructionNameSpace::Instruction> *newinst);
    int dynamicWeightsAssignTask(BooleanDag* g, uint taskid, uint PEid);

    /**
     * @brief int StageProcessors::releaseMem
     * @details release the useless data in a PE after the task `taskid` is assigned
     * @retval 1 OK
     * @retval 0 Wrong
     */
    int releaseMem(BooleanDag* g, uint taskid, bigint *priority);
    int releaseMem(BooleanDag* g, uint taskid, bool *assigned);
    int assignFinish();
    int storeCache();
    int calcLatency();
    int calcEnergy();

    /* Setters */
    int removeStoreInst(uint taskid);

    /* Visitors */
    uint getTaskNum() const;
    bigint getLatency() const;
    bigint getOPLatency() const;
    double getEnergy() const;
    uint const& getpnum() const;
    bigint const& getPEMidLatency(const uint &id) const;
    bigint* const& getMidLatency() const;
    ProcessElem* getPE(uint id);
    StageProcessors* getNext();
    StageProcessors* getPrior();
    uint getPEByTask(uint taskid);
    uint getTaskByPE(uint PEid, int n = -1);
    uint getLine(uint taskid);
    uint const& getOverwritepos(uint peid) const;
    Assignment* getAssignmentByTask(uint taskid);
    void getTime2SpatialUtil(std::map<bigint, uint> &res, bigint offset = 0);

    /* Printers */
    void printScheduleByTasks();
    void printScheduleByPE();
    void printInstructions(int stage=0);
};

#endif