/**  
 * @file    importdag.cpp
 * @brief   Import a DAG in other forms
 * @author  Chenyu Tang
 * @version 2.0
 * @date    2022-11-09
 * @note    
 */

#include "importdag.h"
#include "booleandag.h"
#include "pimconfig.h"
#include "iofile.h"

#define SRCWEIGHT   0

using namespace std;

/**
 * @brief verilog to booleandag
 * 
 * @param filename verilog file name
 * @return BooleanDag* 
 * @details 1'b0 -> No.0
 *          x0, x1, ..., xt -> No.1, No.2, ..., No.t+1
 *          nk, nk+1 ... -> No.t+2, No.t+3, ...
 */
BooleanDag *v2booleandag(const std::string &filename)
{
    BooleanDag *G = new BooleanDag;
    ifstream iVerilog;
    iVerilog.open(filename);
    CheckFile((bool) iVerilog, filename);
    std::string line;

    uint x_num = 0;
    uint y_num = 0;
    uint n_shift = 0;
    uint n_num = 0;

    while (getline(iVerilog, line)) {
        if (line.find("output") != std::string::npos) {
            y_num = count(line.begin(), line.end(), 'y');
            continue;
        } else if (line.find("input") != std::string::npos) {
            x_num = count(line.begin(), line.end(), 'x');
            continue;
        } else if (line.find("wire") != std::string::npos) {
            n_num = count(line.begin(), line.end(), 'n');
            G->init(n_num + x_num + 1, x_num, y_num);

            /* Add 1'b0 and xi to DAG */
            G->addVertice(0, SRCWEIGHT, "1'b0");
            for (uint i = 1u; i <= x_num; ++i) {
                std::string x = "x";
                x += std::to_string(i-1);
                G->addVertice(i, SRCWEIGHT, x);
            }
            int pos = line.find_first_of('n');
            pos++;
            std::string num_string;
            while (line[pos] != ' ' && line[pos] != ',') {
                num_string += line[pos];
                pos++;
            }
            n_shift = atoi(num_string.c_str());
            continue;
        }
        if (line.find("assign") == std::string::npos) continue; // invalid line
        size_t partition = line.find_first_of('=');
        std::string lhs = line.substr(0, partition - 1);
        std::string rhs = line.substr(partition+1);

        int temp = lhs.find_first_of("assign") + 6;
        while (lhs[temp] == ' ') ++temp;
        if (lhs[temp] != 'n') continue; // xi or yi (not ni, skip)

        node operand[3];    // 3 at most
        node dest;
        while (temp < lhs.length() && lhs[temp] != ' ' && lhs[temp] != '=') {
            dest.name += lhs[temp];
            temp++;
        }
        uint destid = atoi(dest.name.c_str()+1) - n_shift + x_num + 1;

        int num_operands = CountOperands(rhs);
        // case 1: assign a = (~)b
        int inv = 0;
        if (num_operands == 0) {
            temp = rhs.find_first_not_of(' ');
            while (rhs[temp] != ' ') {
                operand[0].name += rhs[temp];
                temp++;
            }
            CheckOperand(operand[0]);   // set inv and name(remove '~' if exist)
            inv |= operand[0].inv ? (1<<0) : 0;
            G->addVertice(destid, pcfg.compute_latency, VINV, inv, dest.name);

            // rhs is a constant
            if (operand[0].name[0] == 'n') {
                G->addEdge(atoi(operand[0].name.c_str()+1) - n_shift + x_num + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[0].name[0] == 'x') {
                G->addEdge(atoi(operand[0].name.c_str()+1) + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[0].name.find("1'b") != std::string::npos) {
                G->addEdge(0, destid, pcfg.commWeight[pcfg.levels]);
            }
        }

        // case 2: 2-input gate
        else if (num_operands == 1) {
            temp = rhs.find_first_not_of(' ');
            while (rhs[temp] != ' ') {
                operand[0].name += rhs[temp];
                temp++;
            }
            CheckOperand(operand[0]);
            inv |= operand[0].inv ? (1<<0) : 0;

            temp += 3;
            while (rhs[temp] != ' ' && rhs[temp] != ';') {
                operand[1].name += rhs[temp];
                temp++;
            }
            CheckOperand(operand[1]);
            inv |= operand[1].inv ? (1<<1) : 0;

            if (line.find('&') != std::string::npos) {
                G->addVertice(destid, pcfg.compute_latency, VAND, inv, dest.name);
            } else if (line.find('^') != std::string::npos) {
                G->addVertice(destid, pcfg.compute_latency, VXOR, inv, dest.name);
            } else if (line.find('|') != std::string::npos) {
                G->addVertice(destid, pcfg.compute_latency, VOR, inv, dest.name);
            }

            if (operand[0].name[0] == 'n') {
                G->addEdge(atoi(operand[0].name.c_str()+1) - n_shift + x_num + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[0].name[0] == 'x') {
                G->addEdge(atoi(operand[0].name.c_str()+1) + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[0].name.find("1'b") != std::string::npos) {
                G->addEdge(0, destid, pcfg.commWeight[pcfg.levels]);
            }

            if (operand[1].name[0] == 'n') {
                G->addEdge(atoi(operand[1].name.c_str()+1) - n_shift + x_num + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[1].name[0] == 'x') {
                G->addEdge(atoi(operand[1].name.c_str()+1) + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[1].name.find("1'b") != std::string::npos) {
                G->addEdge(0, destid, pcfg.commWeight[pcfg.levels]);
            }
        }

        // case 3: 3 input XOR gate
        else if (num_operands == 2 && line.find('^') != std::string::npos) {
            temp = rhs.find_first_not_of(' ');
            while (rhs[temp] != ' ') {
                operand[0].name += rhs[temp];
                temp++;
            }
            CheckOperand(operand[0]);
            inv |= operand[0].inv ? (1<<0) : 0;

            temp += 3;
            while (rhs[temp] != ' ' && rhs[temp] != ';') {
                operand[1].name += rhs[temp];
                temp++;
            }
            CheckOperand(operand[1]);
            inv |= operand[1].inv ? (1<<1) : 0;

            temp += 3;
            while (rhs[temp] != ' ' && rhs[temp] != ';') {
                operand[2].name += rhs[temp];
                temp++;
            }
            CheckOperand(operand[2]);
            inv |= operand[2].inv ? (1<<2) : 0;

            G->addVertice(destid, pcfg.compute_latency, VXOR3, inv, dest.name);

            // the third one is a constant
            if (operand[0].name[0] == 'n') {
                G->addEdge(atoi(operand[0].name.c_str()+1) - n_shift + x_num + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[0].name[0] == 'x') {
                G->addEdge(atoi(operand[0].name.c_str()+1) + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[0].name.find("1'b") != std::string::npos) {
                G->addEdge(0, destid, pcfg.commWeight[pcfg.levels]);
            }

            if (operand[1].name[0] == 'n') {
                G->addEdge(atoi(operand[1].name.c_str()+1) - n_shift + x_num + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[1].name[0] == 'x') {
                G->addEdge(atoi(operand[1].name.c_str()+1) + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[1].name.find("1'b") != std::string::npos) {
                G->addEdge(0, destid, pcfg.commWeight[pcfg.levels]);
            }

            if (operand[2].name[0] == 'n') {
                G->addEdge(atoi(operand[2].name.c_str()+1) - n_shift + x_num + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[2].name[0] == 'x') {
                G->addEdge(atoi(operand[2].name.c_str()+1) + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[2].name.find("1'b") != std::string::npos) {
                G->addEdge(0, destid, pcfg.commWeight[pcfg.levels]);
            }
        }
        // Otherwise: majority gate, doesn't need conversion
        else {
            int temp = 0;
            while (rhs[temp] == ' ' || rhs[temp] == '(') {
                temp++;
            }
            while (rhs[temp] != ' ') {
                operand[0].name += rhs[temp];
                temp++;
            }
            CheckOperand(operand[0]);
            inv |= operand[0].inv ? (1<<0) : 0;

            while (rhs[temp] == ' ' || rhs[temp] == '&') {
                temp++;
            }
            while (rhs[temp] != ' ') {
                operand[1].name += rhs[temp];
                temp++;
            }
            CheckOperand(operand[1]);
            inv |= operand[1].inv ? (1<<1) : 0;

            while (rhs[temp] != '&') {
                temp++;
            }
            temp+=2;
            while (rhs[temp] != ' ') {
                operand[2].name += rhs[temp];
                temp++;
            }
            CheckOperand(operand[2]);
            inv |= operand[2].inv ? (1<<2) : 0;

            G->addVertice(destid, pcfg.compute_latency, VMAJ, inv, dest.name);

            if (operand[0].name[0] == 'n') {
                G->addEdge(atoi(operand[0].name.c_str()+1) - n_shift + x_num + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[0].name[0] == 'x') {
                G->addEdge(atoi(operand[0].name.c_str()+1) + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[0].name.find("1'b") != std::string::npos) {
                G->addEdge(0, destid, pcfg.commWeight[pcfg.levels]);
            }

            if (operand[1].name[0] == 'n') {
                G->addEdge(atoi(operand[1].name.c_str()+1) - n_shift + x_num + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[1].name[0] == 'x') {
                G->addEdge(atoi(operand[1].name.c_str()+1) + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[1].name.find("1'b") != std::string::npos) {
                G->addEdge(0, destid, pcfg.commWeight[pcfg.levels]);
            }

            if (operand[2].name[0] == 'n') {
                G->addEdge(atoi(operand[2].name.c_str()+1) - n_shift + x_num + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[2].name[0] == 'x') {
                G->addEdge(atoi(operand[2].name.c_str()+1) + 1, destid, pcfg.commWeight[pcfg.levels]);
            }
            else if (operand[2].name.find("1'b") != std::string::npos) {
                G->addEdge(0, destid, pcfg.commWeight[pcfg.levels]);
            }
        }
    }

    iVerilog.close();
    G->linkDAG();
    return G;
}

