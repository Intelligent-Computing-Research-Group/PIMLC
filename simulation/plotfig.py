import sys
import json
import math
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from scipy.stats.mstats import gmean  

import matplotlib as mpl

mpl.rcParams.update(
{
'text.usetex': False,
'font.family': 'stixgeneral',
'mathtext.fontset': 'stix',
}
)

'''
memorytype-memorysize
x - n(mimd/simd)
y - latency/energy/speedup/temporalutil/maxspatialutil/avgspatialutil
geomean(benchmark*16)
'''

'''
benchmarks
x - n
y - latency/energy/speedup/temporalutil/maxspatialutil/avgspatialutil
geomean(benchmark*12)
'''

colors = ["#FAAA89","#FFDC7E","#7ED0F8","#DF8D8F","#BF947B","#FCD3B5","#7EA2ED","#C497B2","#B9CBB2","#9E94B6","#A5AEBE","#ADCED7","#F7B7B5"]
memcolors = ["#F2D4C9","#DF8D8F","#BF8A7E","#F20505","#A60303","#D1E1E9","#A6BACC","#7A96AB","#526F84","#3E3540"]
benchmarks = ["adder","cavlc","dec","div","int2float","log2","max","multiplier","priority","router","sin","sqrt"]

sramthroughput = dict()
rramthroughput = dict()
simdsramthroughput = dict()
simdrramthroughput = dict()
sramefficiency = dict()
rramefficiency = dict()
simdsramefficiency = dict()
simdrramefficiency = dict()

smallsramthroughput = dict()
smallrramthroughput = dict()
smallsimdsramthroughput = dict()
smallsimdrramthroughput = dict()
smallsramefficiency = dict()
smallrramefficiency = dict()
smallsimdsramefficiency = dict()
smallsimdrramefficiency = dict()

cputhroughput = [0.000003071253,0.000004472271,0.000004789272,0.000000031641,0.000012360939,0.000000114728,0.000000863408,0.000000139134,0.000002858776,0.00001283697,0.000000712656,0.000000099373]
asicthroughput = [0.2785008826,1.268602217,1.138547673,0.01783742868,1.441987486,0.1505786171,0.167380356,0.2139346084,0.5894157414,1.34974948,0.31987494,0.01193747772]
cpuefficiency = [116777,170048,182101,1203,469997,4362,32829,5290,108698,488097,27097,3778]
asicefficiency = [105828.7715,212472.7306,144862.1371,22587.66428,212526.8462,89291.71756,68549.13749,114430.3215,212397.0085,212534.7653,197666.0815,8976.512739]

cpuefficiency = [e/1000000000000 for e in cpuefficiency]    # TOPS/W
asicefficiency = [e/1000000000000 for e in asicefficiency]  # TOPS/W

csvdata = pd.read_csv(sys.argv[1])

workload = [1,2,4,8,16,32,64,100,128,256,365,512,800,1000,1024,1200,1500,1800,2048,2560,3000,3600,4096,5000,6000,8192,10000,12000,14000,16384,20000,24000,28765,32768,36789,42000,51200,65536,80000,102400,131072,160000,200000,262144,360000,460000,524288,720000,1000000,1048576]

# workload = [math.log(i) for i in workload]

speedup = dict()
temporalutil = dict()
maxspatialutil = dict()
avgspatialutil = dict()


benchmarkspeedup = dict()
benchmarktemporalutil = dict()
benchmarkmaxspatialutil = dict()
benchmarkavgspatialutil = dict()



for index, row in csvdata.iterrows():
    # speedup[]
    size = row['workload']
    name = row['memorytype']+"#"+row['memorysize']
    name = name.replace("*4*4*256*256","")
    name = name.replace("*4*4*256*1024","")
    benchmark = row['benchmark']

    if (size==1024):
        if (name=="SRAM#4"):
            smallsramthroughput[benchmark] = row['throughput(GOPS)']/1000
            smallsimdsramthroughput[benchmark] = row['simdthroughput(GOPS)']/1000
            smallsramefficiency[benchmark] = row['opearations(Ops)']/(row['energy(nJ)']*1000)       #sramefficiency[benchmark] = row['efficiency(TOPS/W)']
            smallsimdsramefficiency[benchmark] = row['opearations(Ops)']/(row['simdenergy(nJ)']*1000)    #simdsramefficiency[benchmark] = row['simdefficiency(TOPS/W)']
        else:
            smallrramthroughput[benchmark] = row['throughput(GOPS)']/1000
            smallsimdrramthroughput[benchmark] = row['simdthroughput(GOPS)']/1000
            smallrramefficiency[benchmark] = row['opearations(Ops)']/(row['energy(nJ)']*1000)       # rramefficiency[benchmark] = row['efficiency(TOPS/W)']
            smallsimdrramefficiency[benchmark] = row['opearations(Ops)']/(row['simdenergy(nJ)']*1000)    # simdrramefficiency[benchmark] = row['simdefficiency(TOPS/W)']

    if (size==1048576):
        if (name=="SRAM#4"):
            sramthroughput[benchmark] = row['throughput(GOPS)']/1000
            simdsramthroughput[benchmark] = row['simdthroughput(GOPS)']/1000
            sramefficiency[benchmark] = row['opearations(Ops)']/(row['energy(nJ)']*1000)    #sramefficiency[benchmark] = row['efficiency(TOPS/W)']
            simdsramefficiency[benchmark] = row['opearations(Ops)']/(row['simdenergy(nJ)']*1000)    #simdsramefficiency[benchmark] = row['simdefficiency(TOPS/W)']
        else:
            rramthroughput[benchmark] = row['throughput(GOPS)']/1000
            simdrramthroughput[benchmark] = row['simdthroughput(GOPS)']/1000
            rramefficiency[benchmark] = row['opearations(Ops)']/(row['energy(nJ)']*1000)    # rramefficiency[benchmark] = row['efficiency(TOPS/W)']
            simdrramefficiency[benchmark] = row['opearations(Ops)']/(row['simdenergy(nJ)']*1000)    # simdrramefficiency[benchmark] = row['simdefficiency(TOPS/W)']

    if name not in speedup:
        speedup[name] = dict()
        temporalutil[name] = dict()
        maxspatialutil[name] = dict()
        avgspatialutil[name] = dict()
    if benchmark not in benchmarkspeedup:
        benchmarkspeedup[benchmark] = dict()
        benchmarktemporalutil[benchmark] = dict()
        benchmarkmaxspatialutil[benchmark] = dict()
        benchmarkavgspatialutil[benchmark] = dict()

    if size in speedup[name]:
        speedup[name][size].append(row['simdlatency(ns)']/row['latency(ns)'])
        temporalutil[name][size].append(row['temporalutil'])
        maxspatialutil[name][size].append(row['maxspatialutil'])
        avgspatialutil[name][size].append(row['avgspatialutil'])
    else:
        speedup[name][size] = [row['simdlatency(ns)']/row['latency(ns)']]
        temporalutil[name][size] = [row['temporalutil']]
        maxspatialutil[name][size] = [row['maxspatialutil']]
        avgspatialutil[name][size] = [row['avgspatialutil']]
    
    if size in benchmarkspeedup[benchmark]:
        benchmarkspeedup[benchmark][size].append(row['simdlatency(ns)']/row['latency(ns)'])
        benchmarktemporalutil[benchmark][size].append(row['temporalutil'])
        benchmarkmaxspatialutil[benchmark][size].append(row['maxspatialutil'])
        benchmarkavgspatialutil[benchmark][size].append(row['avgspatialutil'])
    else:
        benchmarkspeedup[benchmark][size] = [row['simdlatency(ns)']/row['latency(ns)']]
        benchmarktemporalutil[benchmark][size] = [row['temporalutil']]
        benchmarkmaxspatialutil[benchmark][size] = [row['maxspatialutil']]
        benchmarkavgspatialutil[benchmark][size] = [row['avgspatialutil']]



for mem in speedup:
    l1 = list()
    l2 = list()
    l3 = list()
    l4 = list()
    for wl in speedup[mem]:
        l1.append(gmean(speedup[mem][wl]))
        l2.append(gmean(temporalutil[mem][wl]))
        l3.append(gmean(maxspatialutil[mem][wl]))
        l4.append(gmean(avgspatialutil[mem][wl]))

    speedup[mem] = l1
    temporalutil[mem] = l2
    maxspatialutil[mem] = l3
    avgspatialutil[mem] = l4

for bm in benchmarkspeedup:
    l1 = list()
    l2 = list()
    l3 = list()
    l4 = list()
    for wl in benchmarkspeedup[bm]:
        l1.append(gmean(benchmarkspeedup[bm][wl]))
        l2.append(gmean(benchmarktemporalutil[bm][wl]))
        l3.append(gmean(benchmarkmaxspatialutil[bm][wl]))
        l4.append(gmean(benchmarkavgspatialutil[bm][wl]))

    benchmarkspeedup[bm] = l1
    benchmarktemporalutil[bm] = l2
    benchmarkmaxspatialutil[bm] = l3
    benchmarkavgspatialutil[bm] = l4


plt.figure(figsize=(20,6))

plt.subplot(121)
plt.tick_params(labelsize=18)
coloridx=0
for key in speedup:
    plt.plot(workload, speedup[key], color=memcolors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+1
plt.semilogx()
plt.xlabel("Workload",fontsize=20)
plt.ylabel("Speedup",fontsize=25)
plt.title("PIMLC Speedup per Memory Spec", size=25)
plt.legend(fontsize=14,ncol=2,frameon=False)


plt.subplot(122)
plt.tick_params(labelsize=18)
coloridx=0
for key in benchmarkspeedup:
    plt.plot(workload, benchmarkspeedup[key], color=colors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+1
plt.semilogx()
plt.xlabel("Workload",fontsize=22)
plt.ylabel("Speedup",fontsize=25)
plt.title("PIMLC Speedup per Benchmark", size=25)
plt.legend(fontsize=15,ncol=2,frameon=False)
plt.subplots_adjust(wspace=0.15)
plt.savefig('figures/PIM_speedup.pdf')


plt.figure(figsize=(20,6))
plt.subplot(121)
plt.tick_params(labelsize=18)
coloridx=0
for key in temporalutil:
    plt.plot(workload, temporalutil[key], color=memcolors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+1
plt.semilogx()
plt.xlabel("Workload",fontsize=22)
plt.ylabel("Utilization",fontsize=25)
plt.title("Temporal Utilization per Memory Spec", size=25)
plt.legend(fontsize=14,ncol=2,frameon=False)


plt.subplot(122)
plt.tick_params(labelsize=18)
coloridx=0
for key in benchmarktemporalutil:
    plt.plot(workload, benchmarktemporalutil[key], color=colors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+1
plt.semilogx()
plt.xlabel("Workload",fontsize=22)
plt.ylabel("Utilization",fontsize=25)
plt.title("Temporal Utilization per Benchmark", size=25)
plt.legend(fontsize=16,ncol=2,frameon=False)
plt.subplots_adjust(wspace=0.15)
plt.savefig('figures/PIM_temporalutil.pdf')



plt.figure(figsize=(32,6))
plt.subplot(141)
plt.tick_params(labelsize=16)
coloridx=0
for key in avgspatialutil:
    plt.plot(workload, avgspatialutil[key], color=memcolors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+1
plt.semilogx()
plt.xlabel("Workload",fontsize=22)
plt.ylabel("Utilization",fontsize=24)
plt.title("Average Spatial Utilization per Memory Spec", size=20)
plt.legend(fontsize=12,ncol=2,frameon=False)

plt.subplot(142)
plt.tick_params(labelsize=16)
coloridx=0
for key in maxspatialutil:
    plt.plot(workload, maxspatialutil[key], color=memcolors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+1
plt.semilogx()
plt.xlabel("Workload",fontsize=20)
plt.ylabel("Utilization",fontsize=24)
plt.title("Peak Spatial Utilization per Memory Spec", size=20)
plt.legend(fontsize=12,ncol=2,frameon=False)

plt.subplot(143)
plt.tick_params(labelsize=16)
coloridx=0
for key in benchmarkavgspatialutil:
    plt.plot(workload, benchmarkavgspatialutil[key], color=colors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+1
plt.semilogx()
plt.xlabel("Workload",fontsize=20)
plt.ylabel("Utilization",fontsize=24)
plt.title("Average Spatial Utilization per Benchmark", size=20)
plt.legend(fontsize=11,ncol=2,frameon=False)


plt.subplot(144)
plt.tick_params(labelsize=16)
coloridx=0
for key in benchmarkmaxspatialutil:
    plt.plot(workload, benchmarkmaxspatialutil[key], color=colors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+1
plt.semilogx()
plt.xlabel("Workload",fontsize=20)
plt.ylabel("Utilization",fontsize=24)
plt.title("Peak Spatial Utilization per Benchmark", size=20)
plt.legend(fontsize=11,ncol=2,frameon=False)
plt.subplots_adjust(wspace=0.19)
plt.savefig('figures/PIM_spatialutil.pdf')





cputhroughput.append(gmean(cputhroughput))
asicthroughput.append(gmean(asicthroughput))
cpuefficiency.append(gmean(cpuefficiency))
asicefficiency.append(gmean(asicefficiency))


sramthroughput = [sramthroughput[t] for t in benchmarks]
sramthroughput.append(gmean(sramthroughput))
rramthroughput = [rramthroughput[t] for t in benchmarks]
rramthroughput.append(gmean(rramthroughput))
sramefficiency = [sramefficiency[t] for t in benchmarks]
sramefficiency.append(gmean(sramefficiency))
rramefficiency = [rramefficiency[t] for t in benchmarks]
rramefficiency.append(gmean(rramefficiency))
smallsramthroughput = [smallsramthroughput[t] for t in benchmarks]
smallsramthroughput.append(gmean(smallsramthroughput))
smallrramthroughput = [smallrramthroughput[t] for t in benchmarks]
smallrramthroughput.append(gmean(smallrramthroughput))
smallsramefficiency = [smallsramefficiency[t] for t in benchmarks]
smallsramefficiency.append(gmean(smallsramefficiency))
smallrramefficiency = [smallrramefficiency[t] for t in benchmarks]
smallrramefficiency.append(gmean(smallrramefficiency))

simdsramthroughput = [simdsramthroughput[t] for t in benchmarks]
simdsramthroughput.append(gmean(simdsramthroughput))
simdrramthroughput = [simdrramthroughput[t] for t in benchmarks]
simdrramthroughput.append(gmean(simdrramthroughput))
simdsramefficiency = [simdsramefficiency[t] for t in benchmarks]
simdsramefficiency.append(gmean(simdsramefficiency))
simdrramefficiency = [simdrramefficiency[t] for t in benchmarks]
simdrramefficiency.append(gmean(simdrramefficiency))
smallsimdsramthroughput = [smallsimdsramthroughput[t] for t in benchmarks]
smallsimdsramthroughput.append(gmean(smallsimdsramthroughput))
smallsimdrramthroughput = [smallsimdrramthroughput[t] for t in benchmarks]
smallsimdrramthroughput.append(gmean(smallsimdrramthroughput))
smallsimdsramefficiency = [smallsimdsramefficiency[t] for t in benchmarks]
smallsimdsramefficiency.append(gmean(smallsimdsramefficiency))
smallsimdrramefficiency = [smallsimdrramefficiency[t] for t in benchmarks]
smallsimdrramefficiency.append(gmean(smallsimdrramefficiency))

benchmarks.append("geomean")
xbar = np.arange(13)
total_width, n = 0.8, 6
width = total_width / n
xbar = xbar - (total_width - width) / 2

fig=plt.figure(figsize=(30,12))

ax=plt.subplot(221)
plt.tick_params(labelsize=19)
plt.xticks(xbar+2.5*width,benchmarks)
plt.bar(xbar, np.array(cpuefficiency), width=width, label="CPU")
plt.bar(xbar+width, np.array(asicefficiency), width=width, label="ASIC")
plt.bar(xbar+2*width, np.array(smallsimdsramefficiency), width=width, label="PIM SRAM w/o WRAS")
plt.bar(xbar+3*width, np.array(smallsramefficiency), width=width, label="PIM SRAM w/ WRAS")
plt.bar(xbar+4*width, np.array(smallsimdrramefficiency), width=width, label="PIM RRAM w/o WRAS")
plt.bar(xbar+5*width, np.array(smallrramefficiency), width=width, label="PIM RRAM w/ WRAS")
plt.title("Efficiency(workload parallism=1024)",fontsize=24)
plt.ylabel("Efficiency(TOPS/W)",fontsize=24)
plt.semilogy()
for tick in ax.get_xticklabels():
    tick.set_rotation(15)
plt.subplots_adjust(wspace=0.15,hspace=0.3)

ax=plt.subplot(222)
plt.tick_params(labelsize=19)
plt.xticks(xbar+2.5*width,benchmarks)
plt.bar(xbar, np.array(cputhroughput), width=width, label="CPU")
plt.bar(xbar+width, np.array(asicthroughput), width=width, label="ASIC")
plt.bar(xbar+2*width, np.array(simdsramthroughput), width=width, label="PIM SRAM w/o WRAS")
plt.bar(xbar+3*width, np.array(sramthroughput), width=width, label="PIM SRAM w/ WRAS")
plt.bar(xbar+4*width, np.array(simdrramthroughput), width=width, label="PIM RRAM w/o WRAS")
plt.bar(xbar+5*width, np.array(rramthroughput), width=width, label="PIM RRAM w/ WRAS")
plt.title("Throughput(workload parallism=1024)",fontsize=24)
plt.ylabel("Throughput(TOPS)",fontsize=24)
plt.semilogy()
for tick in ax.get_xticklabels():
    tick.set_rotation(15)

ax=plt.subplot(223)
plt.tick_params(labelsize=19)
plt.xticks(xbar+2.5*width,benchmarks)
plt.bar(xbar, np.array(cpuefficiency), width=width, label="CPU")
plt.bar(xbar+width, np.array(asicefficiency), width=width, label="ASIC")
plt.bar(xbar+2*width, np.array(simdsramefficiency), width=width, label="PIM SRAM w/o WRAS")
plt.bar(xbar+3*width, np.array(sramefficiency), width=width, label="PIM SRAM w/ WRAS")
plt.bar(xbar+4*width, np.array(simdrramefficiency), width=width, label="PIM RRAM w/o WRAS")
plt.bar(xbar+5*width, np.array(rramefficiency), width=width, label="PIM RRAM w/ WRAS")
plt.title("Efficiency(workload parallism=1048576)",fontsize=24)
plt.ylabel("Efficiency(TOPS/W)",fontsize=24)
plt.semilogy()
for tick in ax.get_xticklabels():
    tick.set_rotation(15)

ax=plt.subplot(224)
plt.tick_params(labelsize=19)
plt.xticks(xbar+2.5*width,benchmarks)
plt.bar(xbar, np.array(cputhroughput), width=width, label="CPU")
plt.bar(xbar+width, np.array(asicthroughput), width=width, label="ASIC")
plt.bar(xbar+2*width, np.array(simdsramthroughput), width=width, label="PIM SRAM w/o WRAS")
plt.bar(xbar+3*width, np.array(sramthroughput), width=width, label="PIM SRAM w/ WRAS")
plt.bar(xbar+4*width, np.array(simdrramthroughput), width=width, label="PIM RRAM w/o WRAS")
plt.bar(xbar+5*width, np.array(rramthroughput), width=width, label="PIM RRAM w/ WRAS")
plt.title("Throughput(workload parallism=1048576)",fontsize=24)
plt.ylabel("Throughput(TOPS)",fontsize=24)
plt.semilogy()
for tick in ax.get_xticklabels():
    tick.set_rotation(15)


lines_labels = ax.get_legend_handles_labels()
lines, labels = [sum(lol, []) for lol in zip(lines_labels)]
fig.legend(lines, labels,fontsize=20,ncol=6,bbox_to_anchor=(0.5,0.96),loc='upper center',frameon=False)

plt.savefig('figures/PIM_performance_comparison.pdf')


print("SRAM 1048576 geomean throughput boost: ",sramthroughput[12]/simdsramthroughput[12])
print("SRAM 1048576 geomean efficiency boost: ",sramefficiency[12]/simdsramefficiency[12])
print("ReRAM 1048576 geomean throughput boost: ",rramthroughput[12]/simdrramthroughput[12])
print("ReRAM 1048576 geomean efficiency boost: ",rramefficiency[12]/simdrramefficiency[12])
print("SRAM 1024 geomean throughput boost: ",smallsramthroughput[12]/smallsimdsramthroughput[12])
print("SRAM 1024 geomean efficiency boost: ",smallsramefficiency[12]/smallsimdsramefficiency[12])
print("ReRAM 1024 geomean throughput boost: ",smallrramthroughput[12]/smallsimdrramthroughput[12])
print("ReRAM 1024 geomean efficiency boost: ",smallrramefficiency[12]/smallsimdrramefficiency[12])


print("16 banks SRAM speedup: ",speedup["SRAM#16"][1])
print("16 banks RRAM speedup: ",speedup["RRAM#16"][1])

