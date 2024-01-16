#include "src/pimconfig.h"
#include <cstdio>

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("argc < 2\n");
    }
    pcfg.setGlobalPIMConf(argv[1]);
    pcfg.printPIMConf();
    return 0;
}
