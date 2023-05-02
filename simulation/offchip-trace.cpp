/**  
 * @file    offchip-trace.cpp
 * @brief   Generate ramulator memory trace file
 * @author  Chenu Tang
 * @version 2.0
 * @date    2022-11-09
 * @note    
 */
#include <fstream>
#include <cstdio>

#define LINESIZE 256
#define BYTESIZE 8
#define SKIPSPACES(p)  while (*(p)==' ' || *(p)=='\t' || *(p) == '\n' || *(p) == '\r') ++(p)

typedef unsigned int uint;
using namespace std;

int strheadstr(const char *s, const char *sub)
{
    int len = 0;
    if (!s || !sub) {
        return -1;
    }
    while (*s && *sub && (*s==*sub)) {
        ++s;
        ++sub;
        ++len;
    }
    if (*sub) {
        return -1;
    }
    return len;
}

/**
 * @brief main
 * 
 * @param argc arg count
 * @param argv 0:./offchip-trace, 1:inputfile(*.asm) 2:outputfile() 3:memory offset
 * @return int 
 */
int main(int argc, char *argv[])
{
    char codeline[LINESIZE];
    char *p;
    uint globaloffset = 0;
    uint globalbitwidth = 64u;
    if (argc < 3) {
        perror("argc must >= 3!\n");
        return -1;
    }
    if (argc > 4) {
        globaloffset = atoi(argv[4]);
    }
    if (argc > 3) {
        globalbitwidth = atoi(argv[3]);
    }
    
    uint src, dst;
    uint meshrows, meshcols, blockrows, blockcols, memoffset, data, input, output;
    ifstream infile;
    FILE *outfile;
    blockrows = 256u;
    blockcols = 256u;
    infile.open(argv[1]);
    outfile = fopen(argv[2], "w");

    while (!infile.eof()) {
        p = codeline;
        infile.getline(codeline, LINESIZE);
        SKIPSPACES(p);
        if (*p == '\0') {
            continue;
        }
        else if (*p == '#' || *p == '%') {
            ++p;
            int len;
            SKIPSPACES(p);

            if ((len = strheadstr(p, "meshrows")) > 0 && (*(p + len) == ' ')) {
                p += len;
                SKIPSPACES(p);
                meshrows = 0;
                while (*p >= '0' && *p <= '9') {
                    meshrows = meshrows * 10 + *p - '0';
                    ++p;
                }
            }
            else if ((len = strheadstr(p, "meshcols")) > 0 && (*(p + len) == ' ')) {
                p += len;
                SKIPSPACES(p);
                meshcols = 0;
                while (*p >= '0' && *p <= '9') {
                    meshcols = meshcols * 10 + *p - '0';
                    ++p;
                }
            }
            else if ((len = strheadstr(p, "blockrows")) > 0 && (*(p + len) == ' ')) {
                p += len;
                SKIPSPACES(p);
                blockrows = 0;
                while (*p >= '0' && *p <= '9') {
                    blockrows = blockrows * 10 + *p - '0';
                    ++p;
                }
            }
            else if ((len = strheadstr(p, "blockcols")) > 0 && (*(p + len) == ' ')) {
                p += len;
                SKIPSPACES(p);
                blockcols = 0;
                while (*p >= '0' && *p <= '9') {
                    blockcols = blockcols * 10 + *p - '0';
                    ++p;
                }
            }
            else if ((len = strheadstr(p, "memoffset")) > 0 && (*(p + len) == ' ')) {
                p += len;
                SKIPSPACES(p);
                memoffset = 0;
                while (*p >= '0' && *p <= '9') {
                    memoffset = memoffset * 10 + *p - '0';
                    ++p;
                }
            }
            else if ((len = strheadstr(p, "data")) > 0 && (*(p + len) == ' ')) {
                p += len;
                SKIPSPACES(p);
                data = 0;
                while (*p >= '0' && *p <= '9') {
                    data = data * 10 + *p - '0';
                    ++p;
                }
                data = (data + globalbitwidth - 1) / globalbitwidth * globalbitwidth;
            }
            else if ((len = strheadstr(p, "input")) > 0 && (*(p + len) == ' ')) {
                p += len;
                SKIPSPACES(p);
                input = 0;
                while (*p >= '0' && *p <= '9') {
                    input = input * 10 + *p - '0';
                    ++p;
                }
            }
            else if ((len = strheadstr(p, "output")) > 0 && (*(p + len) == ' ')) {
                p += len;
                SKIPSPACES(p);
                output = 0;
                while (*p >= '0' && *p <= '9') {
                    output = output * 10 + *p - '0';
                    ++p;
                }
            }
        }
        else if (p[0] == 'L' && p[1] == 'O' && p[2] == 'A' && p[3] == 'D') {
            src = 0;
            p += 4;
            SKIPSPACES(p);
            while (*p >= '0' && *p <= '9') {
                src = src*10 + *p - '0';
                ++p;
            }
            uint addr, localoffset;
            // uint len: Number of 'R' trace lines needed for current LOAD instruction
            uint len = meshcols*blockcols/globalbitwidth;
            len += (len==0);
            for (int i = 0; i < len; ++i) {
                localoffset = memoffset*(blockcols/BYTESIZE)+i*(globalbitwidth/BYTESIZE);
                if (localoffset*BYTESIZE >= data) {
                    break;
                }
                addr = (src-meshrows*blockrows)*(data/BYTESIZE) + localoffset;
                // fprintf(outfile, "%d %d %d %d %d\n", src, meshrows, blockrows, data, localoffset);
                fprintf(outfile, "0x%08x R\n", addr);
            }
        }
        else if (p[0] == 'S' && p[1] == 'T' && p[2] == 'O' && p[3] == 'R' && p[4] == 'E') {
            dst = 0;
            p += 5;
            SKIPSPACES(p);
            while (*p >= '0' && *p <= '9') ++p; // skip src1
            SKIPSPACES(p);
            while (*p >= '0' && *p <= '9') ++p; // skip src2
            SKIPSPACES(p);
            while (*p >= '0' && *p <= '9') ++p; // skip src3
            SKIPSPACES(p);
            while (*p >= '0' && *p <= '9') {
                dst = dst*10 + *p - '0';
                ++p;
            }
            uint addr, localoffset;
            // uint len: Number of 'W' trace lines needed for current STORE instruction
            uint len = meshcols*blockcols/globalbitwidth;
            len += (len==0);
            for (int i = 0; i < len; ++i) {
                localoffset = memoffset*(blockcols/BYTESIZE)+i*(globalbitwidth/BYTESIZE);
                if (localoffset*BYTESIZE >= data) {
                    break;
                }
                addr = (dst-meshrows*blockrows)*(data/BYTESIZE) + localoffset;
                fprintf(outfile, "0x%08x W\n", addr);
            }
        }
    }

    infile.close();
    fclose(outfile);
    return 0;
}