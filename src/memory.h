/**  
 * @file    memory.h
 * @brief   #define basic information of memory limitation
 * @author  Chenu Tang
 * @version 2.4
 * @date    2022-11-20
 * @note    
 */

#ifndef __MEMORY__
#define __MEMORY__
#include <climits>
// #include "config.h"

#ifndef BANKNUM
#define BANKNUM     4
#endif

#define BIGINT_MAX  0x7fffffffffffffffll
#define BIGINT_MIN  0x8000000000000000ll
typedef unsigned int uint;
typedef long long bigint;

#ifdef RRAM
// ./nvsim 65 0 512 256 2 2 1 1 2 2 1 1 0
#define MESHSIZE    (16*BANKNUM)     ///< number of mesh
#define BLOCKROW    256   ///< number of rows in a block - NWL
#define BLOCKCOL    1024   ///< number of lines in a block - NBL

#define COMPUTELATENCY  1352ll   // ps
#define COMPUTEENERGY   270.716 // pJ


#define BANKREADLATENCY (1418ll*4ll)
#define MATREADLATENCY  (1233ll*4ll)
#define SUBARRAYREADLATENCY (968ll*4ll)

#define BANKWRITELATENCY    (20803ll*4ll)
#define MATWRITELATENCY     (20710ll*4ll)
#define SUBARRAYWRITELATENCY    (20445ll*4ll)

#define BANKREADENERGY (204.613*4)
#define MATREADENERGY  (30.8506*4)
#define SUBARRAYREADENERGY (29.629*4)

#define BANKWRITEENERGY (369.567*4)
#define MATWRITEENERGY  (195.805*4)
#define SUBARRAYWRITEENERGY (194.583*4)

#define LEACKAGEENERGY  0.0883617    // W


#else   // SRAM
// ./nvsim 65 1 128 256 2 2 1 1 2 2 1 1 0
#define MESHSIZE    (16*BANKNUM)     ///< number of mesh
#define BLOCKROW    256    ///< number of rows in a block - NWL
#define BLOCKCOL    256    ///< number of lines in a block - NBL

#define COMPUTELATENCY  776ll   // ps
#define COMPUTEENERGY   16.4608 // pJ


#define BANKREADLATENCY 613ll       // ps
#define MATREADLATENCY  594ll
#define SUBARRAYREADLATENCY 391ll

#define BANKWRITELATENCY    604ll
#define MATWRITELATENCY     594ll
#define SUBARRAYWRITELATENCY    391ll

#define BANKREADENERGY 70.8212
#define MATREADENERGY  8.71092
#define SUBARRAYREADENERGY 8.28003

#define BANKWRITEENERGY 69.5843
#define MATWRITEENERGY  7.474
#define SUBARRAYWRITEENERGY 7.04311

#define LEACKAGEENERGY  0.279421    // W
#endif


#define LOADLATENCY     BANKWRITELATENCY
#define STORELATENCY    BANKREADLATENCY

#define LOADENERGY      BANKWRITEENERGY
#define STOREENERGY     BANKREADENERGY

#define OPLATENCY   (COMPUTELATENCY+SUBARRAYWRITELATENCY)
#define OPENERGY    (COMPUTEENERGY+SUBARRAYWRITEENERGY)
#define OPWEIGHT    (COMPUTELATENCY+SUBARRAYWRITELATENCY)

#define COMMWEIGHT      (BANKREADLATENCY+BANKWRITELATENCY)
#define MESHADDR(K,ROW) (K*BLOCKROW+ROW)

const int MemLevel = 4;
const bigint CommWeight[MemLevel+1] = {0, SUBARRAYREADLATENCY+SUBARRAYWRITELATENCY, MATREADLATENCY+MATWRITELATENCY, BANKREADLATENCY+BANKWRITELATENCY, COMMWEIGHT};
const double Readenergy[MemLevel+1] = {0.0, SUBARRAYREADENERGY, MATREADENERGY, BANKREADENERGY, LOADENERGY};
const double WriteEnergy[MemLevel+1] = {0.0, SUBARRAYWRITEENERGY, MATWRITEENERGY, BANKWRITEENERGY, STOREENERGY};
const uint LevelSize[MemLevel] = {1, 4, 4, BANKNUM};
const uint MaxCopyThread[MemLevel] = {MESHSIZE/LevelSize[0], MESHSIZE/LevelSize[0]/LevelSize[1], MESHSIZE/LevelSize[0]/LevelSize[1]/LevelSize[2], MESHSIZE/LevelSize[0]/LevelSize[1]/LevelSize[2]/LevelSize[3]};

inline int getCommLevel(uint totalpnum, uint p1, uint p2)
{
    // TODO: Only support situation for 2^n yet
    if (totalpnum > MESHSIZE || (totalpnum > 1 && totalpnum % 2)) {
        return -1;
    }
    if (p1 >= totalpnum || p2 >= totalpnum) {
        return MemLevel;
    }

    int level = 0;
    int size = LevelSize[level];
    while (level < MemLevel) {
        if (p1 / size == p2 / size) {
            return level;
        }
        size *= LevelSize[++level];
    }
    return -1;
}

#endif