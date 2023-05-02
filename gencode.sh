workload=(1 256 4096 200000)
benchmark=(adder cavlc dec div int2float log2 sin)
baknums=(4)
type=(SRAM)

rm -rf trace
mkdir trace

g++ simulation/offchip-trace.cpp -o gentrace

for m in ${type[@]}
    do
    for t in ${baknums[@]}
    do
        g++ main.cpp ./src/*.cpp ./ILP/ILP.cpp -DBANKNUM=$t -D$m -lOsiClp -lCbc -lClp -o gencode
        for j in ${benchmark[@]}
        do
            for w in ${workload[@]}
            do
                ./gencode ./benchmark/$j.v $w > ./trace/${m}_${t}_${j}_${w}.asm
                ./gentrace ./trace/${m}_${t}_${j}_${w}.asm ./trace/${m}_${t}_${j}_${w}.trace 1024
                echo "${m}_${t}_${j}_${w}.trace generated"
                rm -rf ./trace/${m}_${t}_${j}_${w}.asm
            done
        done
    done
done
rm -f gencode gentrace
