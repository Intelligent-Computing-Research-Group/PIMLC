workload=(1 256 1024 4096 16384 200000)
benchmark=(adder cavlc dec div int2float log2 max multiplier priority router sin sqrt)

rm -f $1
touch $1
echo "memorytype,memorysize,benchmark,workload,latency(ns),energy(nJ),simdlatency(ns),simdenergy(nJ),oplatency(ns),simdoplatency(ns),opearations(Ops),throughput(GOPS),simdthroughput(GOPS),efficiency(TOPS/W),simdefficiency(TOPS/W),temporalutil,simdtemporalutil,maxspatialutil,avgspatialutil,simdmaxspatialutil,simdavgspatialutil" >> $1

for j in ${benchmark[@]}
do
    g++ simulate.cpp ../src/*.cpp ../ILP/ILP.cpp -lOsiClp -lCbc -lClp -o simulate
    ./simulate ../benchmark/$j.v ${#workload[@]} ${workload[@]} >> $1
    echo "$j finish"
done

rm -f simulate
