workload=(1 256 4096 200000)
benchmark=(adder cavlc dec div int2float log2 sin)
baknums=(4)
algorithm=(HEFT LBCP)
type=(SRAM)

rm -rf trace
mkdir -p trace bin

g++ simulation/offchip-trace.cpp -o ./bin/gentrace

for m in ${type[@]}
    do
    for t in ${baknums[@]}
    do
        g++ main.cpp ./src/*.cpp ./ILP/ILP.cpp -DBANKNUM=$t -D$m -lOsiClp -lCbc -lClp -o ./bin/gencode-${t}
        for j in ${benchmark[@]}
        do
            for w in ${workload[@]}
            do
                for a in ${algorithm[@]}
                do {
                    ./bin/gencode-${t} ./benchmark/$j.v $w $a > ./trace/${m}_${t}_${j}_${w}_${a}.asm
                    ./bin/gentrace ./trace/${m}_${t}_${j}_${w}_${a}.asm ./trace/${m}_${t}_${j}_${w}_${a}.trace 1024
                    echo "${m}_${t}_${j}_${w}_${a}.trace generated"
                    rm -rf ./trace/${m}_${t}_${j}_${w}_${a}.asm
                } &
                done
            done
        done
    done
done
# rm -f gencode-* gentrace
