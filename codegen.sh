benchmarks=(adder cavlc dec div int2float log2 max multiplier priority router sin sqrt)
g++ main.cpp src/*.cpp ILP/ILP.cpp -lOsiClp -lCbc -lClp -o main
for b in ${benchmarks[@]}
do
    echo $b
    ./main benchmark/${b}.v $1 > benchmark/${b}.asm
done
