/**  
 * @file    booleandag.h
 * @brief   Basic implementation of boolean dag
 * @author  Chenyu Tang
 * @version 3.0
 * @date    2023-02-09
 * @note    
 */
#ifndef _BOOLEANDAG_
#define _BOOLEANDAG_
#include <cstdio>
#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include "pimconfig.h"

/// @brief Supported Boolean operation
enum VerticeType {
    NORMAL, CONSTANT, VINV, VAND, VOR, VXOR, VXOR3, VMAJ,
};
struct Edge;

/// @brief This structure has not been used yet
typedef struct SubGraph {
    uint size;      ///< number of vertices
    std::set<uint> idset;   ///< vertices set
    uint length;    ///< workload size
} SubGraph;


/**
 * @brief Vertice structure
 * Reperesent a single vertice with its unique id 
 * and other basic information
 */
typedef struct Vertice {
    uint id;                ///< unique id, start from 0
    std::string name;       ///< optional name
    VerticeType type;
    bigint weight;          ///< computation cost
    Edge **predecessors;    ///< parents lists (must before this vertice)
    Edge **successors;      ///< children lists (must after this vertice)
    uint prednum;           ///< number of predecessors
    uint succnum;           ///< number of successors
    int invflags;
    Vertice() : id(-1u), name(""), type{NORMAL}, weight(-1), \
        predecessors(NULL), successors(NULL), prednum(0u), succnum(0u), invflags(0) {};
    ~Vertice() {if(predecessors) delete[] predecessors; if (successors) delete[] successors;};
} Vertice;

/**
 * @brief Edge structure
 * Reperesent an edge connecting two vertices
 * including src, dest and its weight
 */
typedef struct Edge {
    Vertice *src;           ///< source vertice
    Vertice *dest;          ///< destination vertice
    bigint weight;             ///< communication cost
    Edge() : src(NULL), dest(NULL), weight(-1) {};
} Edge;

int destroySingleEdge(Edge *e);     ///< WIP
int cleanSingleVertice(Vertice *v); ///< WIP

/**
 * @brief BooleanDag Class
 * Reperesent a Boolean Function DAG
 */
class BooleanDag {
private:
    Vertice *V;             ///< vertice list (can also be found with src and dest in edges)
    std::vector<Edge> E;    ///< edge list (can also be found with predecessors and successors in vertices)
    uint inputsize;         /// The bit length of input of the Boolean Function, e.g. adder(x[255:0],y[128:0]) ->  256
    uint outputsize;        /// The bit length of output of the Boolean Function, e.g. adder(x[255:0],y[128:0]) -> 129
    uint size;              ///< length of the vertice list (number of the vertices)
    bigint *prio;           ///< priority list (not recommended stored inside this class, remove in the future)

public:
    /* Constructor & Destructor */
    BooleanDag();
    BooleanDag(uint n);
    ~BooleanDag();

    /* Construction related */
    int init();
    int init(uint n,uint i=0u,uint o=0u);   ///< init the DAG
    int destroy();  ///< WIP
    int addVertice(bigint weight, const std::string &s=""); ///< add new Vertice
    int addVertice(uint id, bigint weight, const std::string &s="");
    int addVertice(uint id, bigint weight, VerticeType type, int inv, const std::string &s="");
    int addEdge(uint src, uint dest, bigint weight);    ///< add new edge
    int linkDAG();      ///< must run this function after adding all vertices and edges


    /* Setters */
    int setPriority(bigint *arr);

    /* Vistors */
    uint const& getsize() const;
    uint const& getoutputsize() const;
    uint const& getinputsize() const;
    Vertice* getvertice(uint id);
    bigint getPriority(uint id);

    int getCriticalPath(std::set<uint> & res);
    int getMainCluster(std::set<uint> & res, uint depth= 1);
    
    // int getCriticalPath(std::vector<uint> &res);
    // int getCriticalPath(std::set<uint> &res);

    void traversePrint();
};




#endif