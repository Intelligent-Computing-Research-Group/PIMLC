/**  
 * @file    iofile.h
 * @brief   Basic implementation of boolean dag
 * @author  Chenyu Tang
 * @version 4.5
 * @date    2024-01-11
 * @note    
 */


#ifndef _IOFILE_
#define _IOFILE_

#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <algorithm>
#define SKIPSPACES(p)  while (*(p)==' ' || *(p)=='\t' || *(p) == '\n' || *(p) == '\r') ++(p)
#define SKIPSTRINGSPACES(s,t)  while (((s)[(t)])==' ' || ((s)[(t)])=='\t' || ((s)[(t)])=='\r' || ((s)[(t)])=='\n') ++(t)

typedef struct Node {
    std::string name;
    bool leaf = false;      // false: intermediate nodes; true: leaf/root nodes
    bool inv = false;       // false: origin; true: need an inverter
} node;

std::string &removespace(std::string &s);

int parseline(std::string s, std::string &lhs, std::string &rhs);

void CheckFile(bool iFile, const std::string& filename);

int CountOperands(const std::string& rhs);

void CheckOperand(node& a);

#endif
