/**  
 * @file    pimconfig.h
 * @brief   #define basic information of PIM settings
 * @author  Chenyu Tang
 * @version 4.6
 * @date    2024-1-11
 * @note    
 */

#ifndef __PIMCONFIG__
#define __PIMCONFIG__
#include <climits>
#include <string>
#include <functional>
#include <unordered_map>

#define MAXPIMLEVEL 4


#define BIGINT_MAX  0x7fffffffffffffffll
#define BIGINT_MIN  0x8000000000000000ll
typedef unsigned int uint;
typedef long long bigint;

#define roundup(x, y) ((((x)+((y)-1))/(y))*(y))

class PIMConf {
private:
    
public:
    static inline PIMConf& getInstance() {
        static PIMConf _ins;
        return _ins;
    }
    void setGlobalPIMConf(const char *configfile);
    void printPIMConf();
    void test() { printf("test!\n");}
private:
    ///< function caller, init in initFunctionMap()
    std::unordered_map<std::string, std::function<void(std::string&)>> options;


    /* The 0-level is the smallest unit with bit cols and rows in the PIM */
    std::string _name;  ///< PIM name

    int _chip_num;  ///< number of PIM chips

    double _frequency;  ///< clock frequency (GHz)
    bigint _cycle_time; ///< clock cycle (ps), calculated in setGlobalPIMConf()

    bool _latency_roundup;  ///< should roundup latency or not

    int _levels;   ///< the level number of PIM, not larger than 
                        ///<   MAXPIMLEVEL, block is the lowest level

    int _level_size[MAXPIMLEVEL];   ///< the number of level k-1 units
                                    ///<   in level k, _level_size[0]=1

    std::string _level_name[MAXPIMLEVEL];

    int _schedule_level;    ///< The smallest level to be scheduled to 
                            ///<  form the MIMD system

    int _max_threads;       ///< Max MIMD instruction threads

    int _level0_cols;
    int _level0_rows;


    /* Block related data are calculated in setGlobalPIMConf() */
    int _block_cols;    ///< the degree of parallelism
    int &_block_parallelism = _block_cols;

    int _block_rows;    ///< the size of bits of a single col
    int &_block_size = _block_rows;

    int _block_nums;    ///< total blocks contained in the PIM

    bigint _compute_latency;    ///< single block compute latency
    double _compute_energy;     ///< single block compute energy

    /* read/write happens in the PIM */
    bigint _read_latency[MAXPIMLEVEL];
    bigint _write_latency[MAXPIMLEVEL];
    double _read_energy[MAXPIMLEVEL];
    double _write_energy[MAXPIMLEVEL];
    uint _maxCopyThread[MAXPIMLEVEL];    ///< calculated in setGlobalPIMConf()

    /* load/store happens between DRAM and PIM */

    bigint _load_latency;
    bigint _store_latency;
    double _load_energy;
    double _store_energy;

    double _leackage_energy;

    /* This is for schedule, should we place it here? */
    bigint _commWeight[MAXPIMLEVEL+1];  ///< calculated in setGlobalPIMConf()
public:
    PIMConf(const PIMConf&) = delete;
    PIMConf(PIMConf&&) = delete;
    PIMConf& operator=(const PIMConf&) = delete;
    PIMConf& operator=(PIMConf&&) = delete;
private:
    virtual void initFunctionMap();
    PIMConf();
    ~PIMConf() {};
public:
    /* read-only reference for direct acesss */
    const std::string &name = _name;  ///< PIM name
    const int &chip_num = _chip_num;
    const double &frequency = _frequency;
    const bigint &cycle_time = _cycle_time;
    const bool &latency_roundup = _latency_roundup;
    const int &levels = _levels;
    const int (&level_size)[MAXPIMLEVEL] = _level_size;
    const std::string (&level_name)[MAXPIMLEVEL] = _level_name;
    const int &schedule_level = _schedule_level;
    const int &max_threads = _max_threads;
    const int &level0_cols = _level0_cols;
    const int &level0_rows = _level0_rows;
    const int &block_cols = _block_cols;
    const int &block_rows = _block_rows;
    const int &block_nums = _block_nums;
    const bigint &compute_latency = _compute_latency;
    const double &compute_energy = _compute_energy;
    const bigint (&read_latency)[MAXPIMLEVEL] = _read_latency;
    const bigint (&write_latency)[MAXPIMLEVEL] = _write_latency;
    const double (&read_energy)[MAXPIMLEVEL] = _read_energy;
    const double (&write_energy)[MAXPIMLEVEL] = _write_energy;
    const uint (&maxCopyThread)[MAXPIMLEVEL] = _maxCopyThread;
    const bigint &load_latency = _load_latency;
    const bigint &store_latency = _store_latency;
    const double &load_energy = _load_energy;
    const double &store_energy = _store_energy;
    const double &leackage_energy = _leackage_energy;
    const bigint (&commWeight)[MAXPIMLEVEL+1] = _commWeight;
};

extern PIMConf &pcfg;

inline int getCommLevel(uint totalpnum, uint p1, uint p2)
{
    // TODO: Only support situation for 2^n yet
    static PIMConf &cfg = PIMConf::getInstance();
    static uint blocknums = cfg.block_nums;
    static int levelnums = cfg.levels;
    if (totalpnum > blocknums || (totalpnum > 1 && totalpnum % 2)) {
        return -1;
    }
    if (p1 >= totalpnum || p2 >= totalpnum) {
        return levelnums;   // off-chip communication
    }

    int level = 0;
    // e.g. levelsize = [1, 4, 4, 4], blocknums=1*4*4*4=64
    int size = cfg.level_size[0];
    while (level < levelnums) {
        if (p1 / size == p2 / size) {
            return level;   // on-chip communication
        }
        size *= cfg.level_size[++level];
    }
    return -1;
}

#endif