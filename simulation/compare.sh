workload=(1 256 1024 4096 16384 200000)
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
