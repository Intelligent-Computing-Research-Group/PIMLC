workload=(1 2 4 8 16 32 64 100 128 256 365 512 800 1000 1024 1200 1500 1800 2048 2560 3000 3600 4096 5000 6000 8192 10000 12000 14000 16384 20000 24000 28765 32768 36789 42000 51200 65536 80000 102400 131072 160000 200000 262144 360000 460000 524288 720000 1000000 1048576)
benchmark=(adder cavlc dec div int2float log2 max multiplier priority router sin sqrt)
baknums=(1 2 4 8 16)
type=(SRAM RRAM)

rm -f $1
touch $1
echo "memorytype,memorysize,benchmark,workload,latency(ns),energy(nJ),simdlatency(ns),simdenergy(nJ),oplatency(ns),simdoplatency(ns),opearations(Ops),throughput(GOPS),simdthroughput(GOPS),efficiency(TOPS/W),simdefficiency(TOPS/W),temporalutil,simdtemporalutil,maxspatialutil,avgspatialutil,simdmaxspatialutil,simdavgspatialutil" >> $1
for m in ${type[@]}
    do
    for t in ${baknums[@]}
    do
        for j in ${benchmark[@]}
        do
            g++ simulate.cpp ../src/*.cpp ../ILP/ILP.cpp -DBANKNUM=$t -D$m -lOsiClp -lCbc -lClp -o simulate
            ./simulate ../benchmark/$j.v ${#workload[@]} ${workload[@]} >> $1
            echo "$m $t $j finish"
        done
    done
done
rm -f simulate
