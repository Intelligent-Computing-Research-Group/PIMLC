/**  
 * @file    instruction.h
 * @brief   instruction details
 * @author  Chenu Tang
 * @version 2.0
 * @date    2022-11-09
 * @note    
 */

#ifndef __INSTRUCTION__
#define __INSTRUCTION__

#include "memory.h"

namespace InstructionNameSpace {
enum InstructionType {
    NOP, COPY, STORE, LOAD, SYNC, INV, AND, OR, XOR, XOR3, MAJ, 
};

const char* const instname[] = {"NOP", "COPY", "STORE", "LOAD", "SYNC", "INV", "AND", "OR", "XOR", "XOR3", "MAJ"};

typedef int Address;

typedef struct Instruction {

    uint    taskid;
    InstructionType op;
    Address src[3];
    Address dest;
    bool    invflag[3];

    Instruction() : taskid(UINT_MAX),op(NOP),src{0,0,0},dest(0),invflag{false,false,false}{};
} Instruction;

}




#endif