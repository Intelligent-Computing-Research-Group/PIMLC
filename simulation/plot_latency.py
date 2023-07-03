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

colors = ["#FAAA89", "#FFDC7E", "#7ED0F8", "#DF8D8F", "#BF947B", "#FCD3B5",
          "#7EA2ED", "#C497B2", "#B9CBB2", "#9E94B6", "#A5AEBE", "#ADCED7", "#F7B7B5"]
memcolors = ["#F2D4C9", "#DF8D8F", "#BF8A7E", "#F20505",
             "#A60303", "#D1E1E9", "#A6BACC", "#7A96AB", "#526F84", "#3E3540"]
benchmarks = ["adder", "cavlc", "dec", "div", "int2float", "log2",
              "max", "multiplier", "priority", "router", "sin", "sqrt"]
workload = [1, 4, 16, 256, 800, 1024, 2048, 8192, 12000,
            16384, 51200, 65536, 102400, 200000, 524288]
memorysize = ['1*4*4*256*256', '2*4*4*256*256', '4*4*4*256*256', '8*4*4*256*256', '16*4*4*256*256']
banknum = [1, 2, 4, 8, 16]

# read data
csvdata = dict()
csvdata['RD'] = pd.read_csv('RD202305182036.csv')
csvdata['HEFT'] = pd.read_csv('HEFT202305162217.csv')
csvdata['CP'] = pd.read_csv('CP202305170042.csv')

fig = plt.figure(figsize=(38, 10))

# plot workload
workload_RD = []
workload_HEFT = []
workload_CP = []

for i in workload:
    q = 'memorytype == \'SRAM\' & workload == ' + \
        str(i) + ' & memorysize == \'4*4*4*256*256\''
    # print(q)
    # print(gmean(csvdata['RD'].query(q)['latency(ns)']))
    workload_RD.append(gmean(csvdata['RD'].query(q)['latency(ns)']))
    workload_HEFT.append(gmean(csvdata['HEFT'].query(q)['latency(ns)']))
    workload_CP.append(gmean(csvdata['CP'].query(q)['latency(ns)']))

workloadspeedup_RD = []
workloadspeedup_HEFT = []
workloadspeedup_CP = []

for i in range(len(workload)):
    workloadspeedup_RD.append(workload_RD[i] / workload_RD[i])
    workloadspeedup_HEFT.append(workload_RD[i] / workload_HEFT[i])
    workloadspeedup_CP.append(workload_RD[i] / workload_CP[i])

xbar = np.arange(len(workload))
total_width, n = 0.8, 2
width = total_width / n
xbar = xbar - (total_width - width) / 2
ax = plt.subplot(131)

plt.tick_params(labelsize=19)
plt.xticks(xbar+1.5*width, workload, fontsize=14)
ax.set_ylim(0.6, 1.7)
# plt.bar(xbar, np.array(workloadspeedup_RD), width=width, label='RD')
plt.bar(xbar+width, np.array(workloadspeedup_HEFT), width=width, label='HEFT')
plt.bar(xbar+2*width, np.array(workloadspeedup_CP), width=width, label='LBCP')
plt.title("Speedup w.r.t different workload parallelism", fontsize=24)
plt.xlabel("workload parallelism $n$", fontsize=24)
plt.ylabel("speedup", fontsize=25)

# plt.semilogy()
for tick in ax.get_xticklabels():
    tick.set_rotation(45)

# plot benchmark
benchmark_RD = []
benchmark_HEFT = []
benchmark_CP = []

for i in benchmarks:
    q = 'memorytype == \'SRAM\' & benchmark == \'' + \
        i + '\' & memorysize == \'4*4*4*256*256\''
    # print(q)
    # print(gmean(csvdata['RD'].query(q)['latency(ns)']))
    benchmark_RD.append(gmean(csvdata['RD'].query(q)['latency(ns)']))
    benchmark_HEFT.append(gmean(csvdata['HEFT'].query(q)['latency(ns)']))
    benchmark_CP.append(gmean(csvdata['CP'].query(q)['latency(ns)']))

benchmarkspeedup_RD = []
benchmarkspeedup_HEFT = []
benchmarkspeedup_CP = []

for i in range(len(benchmarks)):
    benchmarkspeedup_RD.append(benchmark_RD[i] / benchmark_RD[i])
    benchmarkspeedup_HEFT.append(benchmark_RD[i] / benchmark_HEFT[i])
    benchmarkspeedup_CP.append(benchmark_RD[i] / benchmark_CP[i])

xbar = np.arange(len(benchmarks))
total_width, n = 0.8, 2
width = total_width / n
xbar = xbar - (total_width - width) / 2
ax = plt.subplot(132)

plt.tick_params(labelsize=19)
plt.xticks(xbar+1.5*width, benchmarks, fontsize=16)
# plt.bar(xbar, np.array(benchmarkspeedup_RD), width=width, label='RD')
plt.bar(xbar+width, np.array(benchmarkspeedup_HEFT),
        width=width, label='HEFT')
plt.bar(xbar+2*width, np.array(benchmarkspeedup_CP), width=width, label='LBCP')
plt.title("Speedup w.r.t different benchmarks", fontsize=24)
plt.xlabel("benchmark", fontsize=25)
plt.ylabel("speedup", fontsize=25)
# plt.semilogy()
for tick in ax.get_xticklabels():
    tick.set_rotation(15)
plt.subplots_adjust(wspace=0.15, hspace=0.4)

# plot memorysize

memorysize_RD = []
memorysize_HEFT = []
memorysize_CP = []

for i in memorysize:
    q = 'memorytype == \'SRAM\' & memorysize == \'' + i + '\''
    # print(q)
    # print(gmean(csvdata['RD'].query(q)['latency(ns)']))
    memorysize_RD.append(gmean(csvdata['RD'].query(q)['latency(ns)']))
    memorysize_HEFT.append(gmean(csvdata['HEFT'].query(q)['latency(ns)']))
    memorysize_CP.append(gmean(csvdata['CP'].query(q)['latency(ns)']))

memorysizespeedup_RD = []
memorysizespeedup_HEFT = []
memorysizespeedup_CP = []

for i in range(len(memorysize)):
    memorysizespeedup_RD.append(memorysize_RD[i] / memorysize_RD[i])
    memorysizespeedup_HEFT.append(memorysize_RD[i] / memorysize_HEFT[i])
    memorysizespeedup_CP.append(memorysize_RD[i] / memorysize_CP[i])

xbar = np.arange(len(memorysize))
total_width, n = 0.8, 2
width = total_width / n
xbar = xbar - (total_width - width) / 2
ax = plt.subplot(133)

plt.tick_params(labelsize=19)
plt.xticks(xbar+1.5*width, banknum, fontsize=25)
ax.set_ylim(0.6, 1.4)
# plt.bar(xbar, np.array(memorysizespeedup_RD), width=width, label='RD')
plt.bar(xbar+width, np.array(memorysizespeedup_HEFT),
        width=width, label='HEFT')
plt.bar(xbar+2*width, np.array(memorysizespeedup_CP), width=width, label='LBCP')
plt.title("Speedup w.r.t different memory size", fontsize=24)
plt.xlabel("bank number", fontsize=25)
plt.ylabel("speedup", fontsize=25)
# plt.semilogy()
# for tick in ax.get_xticklabels():
#     tick.set_rotation(15)
plt.subplots_adjust(bottom=0.25, top=0.75, wspace=0.15, hspace=0.5)


lines_labels = ax.get_legend_handles_labels()
lines, labels = [sum(lol, []) for lol in zip(lines_labels)]
fig.legend(lines, labels, fontsize=24, ncol=6, bbox_to_anchor=(
    0.5, 0.85), loc='upper center', frameon=False)


plt.savefig('figures/PIM_schedulecomp.pdf')
