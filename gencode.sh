workload=(1 1048576)
benchmark=(adder cavlc dec div int2float log2 max multiplier priority router sin sqrt)
algorithm=(HEFT LBCP)


rm -rf trace
mkdir -p trace bin

g++ simulation/offchip-trace.cpp -o ./bin/gentrace

g++ main.cpp ./src/*.cpp ./ILP/ILP.cpp -D$m -lOsiClp -lCbc -lClp -o ./bin/gencode-${t}
for j in ${benchmark[@]}
do
    for w in ${workload[@]}
    do
        for a in ${algorithm[@]}
        do {
            ./bin/gencode-${t} ./benchmark/$j.v $w $a > ./trace/${j}_${w}_${a}.asm
            ./bin/gentrace ./trace/${j}_${w}_${a}.asm ./trace/${j}_${w}_${a}.trace 1024
            echo "${j}_${w}_${a}.trace generated"
            # rm -rf ./trace/${j}_${w}_${a}.asm
        } &
        done
    done
done
# rm -f gencode-* gentrace
