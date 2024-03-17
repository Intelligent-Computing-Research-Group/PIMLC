import sys
import json
import math
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from scipy.stats.mstats import gmean  

import matplotlib as mpl
from matplotlib.ticker import FuncFormatter

# mpl.rcParams.update(
# {
# 'text.usetex': False,
# 'font.family': 'stixgeneral',
# 'mathtext.fontset': 'stix',
# }
# )
mpl.rcParams["font.sans-serif"] = ["SimHei"]
# mpl.rcParams['axes.unicode_minus'] = False ## 设置正常显示符号
mpl.rcParams['font.family'] = 'SimHei' # 作用：解决图上汉字显示为方框的问题 在图上显示汉字
mpl.rcParams['axes.unicode_minus']=False # 作用：解决坐标轴为负时 负号显示为方框的问题

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
memcolors = ["#F2D4C9","#DF8D8F","#BF8A7E","#F20505","#A60303","#D1E1E9","#A6BACC","#7A96AB","#526F84","#3E3540","#3E3540"]
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
    name = row['memorytype']
    benchmark = row['benchmark']

    if (size==1024):
        if (name=="2MB-SRAM"):
            smallsramthroughput[benchmark] = row['throughput(GOPS)']
            smallsimdsramthroughput[benchmark] = row['simdthroughput(GOPS)']
            smallsramefficiency[benchmark] = row['operations(Ops)']/(row['energy(nJ)'])       #sramefficiency[benchmark] = row['efficiency(TOPS/W)']
            smallsimdsramefficiency[benchmark] = row['operations(Ops)']/(row['simdenergy(nJ)'])    #simdsramefficiency[benchmark] = row['simdefficiency(TOPS/W)']
        else:
            smallrramthroughput[benchmark] = row['throughput(GOPS)']
            smallsimdrramthroughput[benchmark] = row['simdthroughput(GOPS)']
            smallrramefficiency[benchmark] = row['operations(Ops)']/(row['energy(nJ)'])       # rramefficiency[benchmark] = row['efficiency(TOPS/W)']
            smallsimdrramefficiency[benchmark] = row['operations(Ops)']/(row['simdenergy(nJ)'])    # simdrramefficiency[benchmark] = row['simdefficiency(TOPS/W)']

    if (size==1048576):
        if (name=="2MB-SRAM"):
            sramthroughput[benchmark] = row['throughput(GOPS)']
            simdsramthroughput[benchmark] = row['simdthroughput(GOPS)']
            sramefficiency[benchmark] = row['operations(Ops)']/(row['energy(nJ)'])    #sramefficiency[benchmark] = row['efficiency(TOPS/W)']
            simdsramefficiency[benchmark] = row['operations(Ops)']/(row['simdenergy(nJ)'])    #simdsramefficiency[benchmark] = row['simdefficiency(TOPS/W)']
        else:
            rramthroughput[benchmark] = row['throughput(GOPS)']
            simdrramthroughput[benchmark] = row['simdthroughput(GOPS)']
            rramefficiency[benchmark] = row['operations(Ops)']/(row['energy(nJ)'])    # rramefficiency[benchmark] = row['efficiency(TOPS/W)']
            simdrramefficiency[benchmark] = row['operations(Ops)']/(row['simdenergy(nJ)'])    # simdrramefficiency[benchmark] = row['simdefficiency(TOPS/W)']

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


plt.figure(figsize=(20,5.2))

ax = plt.subplot(121)
plt.tick_params(direction="in",labelsize=27)
plt.subplots_adjust(bottom=0.2)
plt.yticks(position=(0.1,0))
coloridx=0
ax.set_ylim(0.5, 19.5)
for key in speedup:
    plt.plot(workload, speedup[key], color=memcolors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+2
plt.semilogx()
plt.xlabel("计算负载",fontsize=38,labelpad=-1.1)
plt.ylabel("加速比",fontsize=40)
plt.title("(a) 不同PIM规格下的速度提升", size=32)
plt.legend(fontsize=17,ncol=1,loc='upper right',frameon=False)


ax = plt.subplot(122)
plt.tick_params(direction="in",labelsize=27)
plt.yticks(position=(0.1,0))
coloridx=0
# ax.set_ylim(-1, 62)
for key in benchmarkspeedup:
    plt.plot(workload, benchmarkspeedup[key], color=colors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+1
plt.semilogx()
plt.xlabel("计算负载",fontsize=38,labelpad=-1.1)
# plt.ylabel("加速比",fontsize=25)
plt.title("(b) 不同基准测试下的速度提升", size=32)
plt.legend(fontsize=18,ncol=2,loc='upper right',frameon=False)
plt.subplots_adjust(wspace=0.05)
plt.savefig('figures/PIM_speedup.pdf',bbox_inches='tight')



plt.figure(figsize=(26,4.8))
plt.subplot(141)
plt.tick_params(labelsize=22)
coloridx=0
for key in avgspatialutil:
    plt.plot(workload, avgspatialutil[key], color=memcolors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+2
plt.semilogx()
plt.xlabel("计算负载",fontsize=36,labelpad=-1.1)
plt.ylabel("利用率",fontsize=42)
plt.title("(a) 单内存配置下\n平均空间利用率", size=30)
plt.legend(fontsize=18,ncol=1,loc='upper left',frameon=False)

plt.subplot(142)
plt.tick_params(labelsize=22)
coloridx=0
for key in maxspatialutil:
    plt.plot(workload, maxspatialutil[key], color=memcolors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+2
plt.semilogx()
plt.xlabel("计算负载",fontsize=36,labelpad=-1.1)
# plt.ylabel("Utilization",fontsize=24)
plt.title("(b) 单内存配置下\n最大空间利用率", size=30)
plt.legend(fontsize=18,ncol=1,loc='upper left',frameon=False)

plt.subplot(143)
plt.tick_params(labelsize=22)
coloridx=0
for key in benchmarkavgspatialutil:
    plt.plot(workload, benchmarkavgspatialutil[key], color=colors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+1
plt.semilogx()
plt.xlabel("计算负载",fontsize=36,labelpad=-1.1)
# plt.ylabel("Utilization",fontsize=24)
plt.title("(c) 单基准下\n平均空间利用率", size=30)
plt.legend(fontsize=14,ncol=2,loc='upper left',frameon=False)


plt.subplot(144)
plt.tick_params(labelsize=22)
coloridx=0
for key in benchmarkmaxspatialutil:
    plt.plot(workload, benchmarkmaxspatialutil[key], color=colors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+1
plt.semilogx()
plt.xlabel("计算负载",fontsize=36,labelpad=-1.1)
# plt.ylabel("Utilization",fontsize=24)
plt.title("(d) 单基准下\n最大空间利用率", size=30)
plt.legend(fontsize=14,ncol=2,loc='upper left',frameon=False)
plt.subplots_adjust(wspace=0.19, bottom=0.18, top=0.82)
plt.savefig('figures/PIM_spatialutil.pdf',bbox_inches='tight')


plt.figure(figsize=(13,4.8))
plt.subplot(121)
plt.tick_params(labelsize=22)
coloridx=0
for key in temporalutil:
    plt.plot(workload, temporalutil[key], color=memcolors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+2
plt.semilogx()
plt.xlabel("计算负载",fontsize=36,labelpad=-1.1)
plt.ylabel("利用率",fontsize=42)
plt.title("(a) 单内存配置下\n时间利用率", size=30)
plt.legend(fontsize=18,ncol=1,loc='upper left',frameon=False)


plt.subplot(122)
plt.tick_params(labelsize=22)
coloridx=0
for key in benchmarktemporalutil:
    plt.plot(workload, benchmarktemporalutil[key], color=colors[coloridx], linewidth=2, label=key)
    coloridx=coloridx+1
plt.semilogx()
plt.xlabel("计算负载",fontsize=36,labelpad=-1.1)
plt.title("(b) 单基准下\n时间利用率", size=30)
plt.legend(fontsize=14,ncol=2,loc='upper left',frameon=False)
plt.subplots_adjust(wspace=0.19, bottom=0.18, top=0.82)
plt.savefig('figures/PIM_temporalutil.pdf',bbox_inches='tight')


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
total_width, n = 1, 6
width = total_width / n
xbar = xbar - (total_width - width) / 2


fig=plt.figure(figsize=(18,14))
plt.subplots_adjust(bottom=0.153, top=0.847, wspace=0.16,hspace=0.52)

ax=plt.subplot(411)
plt.tick_params(labelsize=21)
plt.xticks(xbar+1.5*width,benchmarks,position=(0,0.04))
plt.bar(xbar+0*width, np.array(smallsimdsramefficiency), width=width, label="SIMD下的SRAM-PIM")
plt.bar(xbar+1*width, np.array(smallsramefficiency), width=width, label="MIMD下的SRAM-PIM")
plt.bar(xbar+2*width, np.array(smallsimdrramefficiency), width=width, label="SIMD下的ReRAM-PIM")
plt.bar(xbar+3*width, np.array(smallrramefficiency), width=width, label="MIMD下的ReRAM-PIM")
plt.title("Efficiency of workload parallism $n=1024$",fontsize=24)
plt.ylabel("效率\n(GOPS/W)",fontsize=22)
plt.semilogy()
plt.yticks(fontproperties = 'DejaVu Sans', size = 20)
for tick in ax.get_xticklabels():
    tick.set_rotation(17.5)

ax=plt.subplot(412)
plt.tick_params(labelsize=21)
plt.xticks(xbar+1.5*width,benchmarks,position=(0,0.04))
plt.bar(xbar+0*width, np.array(smallsimdsramthroughput), width=width, label="SIMD下的SRAM-PIM")
plt.bar(xbar+1*width, np.array(smallsramthroughput), width=width, label="MIMD下的SRAM-PIM")
plt.bar(xbar+2*width, np.array(smallsimdrramthroughput), width=width, label="SIMD下的ReRAM-PIM")
plt.bar(xbar+3*width, np.array(smallrramthroughput), width=width, label="MIMD下的ReRAM-PIM")
plt.title("Throughput of workload parallism $n =1024$",fontsize=24)
plt.ylabel("吞吐率\n(GOPS)",fontsize=23)
plt.semilogy()
plt.yticks(fontproperties = 'DejaVu Sans', size = 20)
for tick in ax.get_xticklabels():
    tick.set_rotation(17.5)

ax=plt.subplot(413)
plt.tick_params(labelsize=21)
plt.xticks(xbar+1.5*width,benchmarks,position=(0,0.04))
plt.bar(xbar+0*width, np.array(simdsramefficiency), width=width, label="SIMD下的SRAM-PIM")
plt.bar(xbar+1*width, np.array(sramefficiency), width=width, label="MIMD下的SRAM-PIM")
plt.bar(xbar+2*width, np.array(simdrramefficiency), width=width, label="SIMD下的ReRAM-PIM")
plt.bar(xbar+3*width, np.array(rramefficiency), width=width, label="MIMD下的ReRAM-PIM")
plt.title("Efficiency of workload parallism $n= 1048576$",fontsize=24)
plt.ylabel("效率\n(GOPS/W)",fontsize=22)
# ax.set_ylim(0.01,150)
plt.semilogy()
plt.yticks(fontproperties = 'DejaVu Sans', size = 20)
for tick in ax.get_xticklabels():
    tick.set_rotation(17.5)

ax=plt.subplot(414)
plt.tick_params(labelsize=21)
plt.xticks(xbar+1.5*width,benchmarks,position=(0,0.04))
plt.bar(xbar+0*width, np.array(simdsramthroughput), width=width, label="SIMD下的SRAM-PIM")
plt.bar(xbar+1*width, np.array(sramthroughput), width=width, label="MIMD下的SRAM-PIM")
plt.bar(xbar+2*width, np.array(simdrramthroughput), width=width, label="SIMD下的ReRAM-PIM")
plt.bar(xbar+3*width, np.array(rramthroughput), width=width, label="MIMD下的ReRAM-PIM")
plt.title("Throughput of workload parallism $n=1048576$",fontsize=24)
plt.ylabel("吞吐率\n(GOPS)",fontsize=23)
plt.semilogy()
plt.yticks(fontproperties = 'DejaVu Sans', size = 20)
for tick in ax.get_xticklabels():
    tick.set_rotation(17.5)


lines_labels = ax.get_legend_handles_labels()
lines, labels = [sum(lol, []) for lol in zip(lines_labels)]
fig.legend(lines,labels,fontsize=21,ncol=4,bbox_to_anchor=(0.5,0.91),loc='upper center',frameon=False)

plt.savefig('figures/PIM_performance_comparison.pdf', bbox_inches='tight')



testname = ["add32", "add16", "add8", "mul32", "mul16", "mul8", "div32", "div16", "div8"]

srampimlatency = [72998.896, 36426.144, 18146.432, 779023.04, 52313.12, 20865.344, 150767.776, 55045.36, 21638.368]
srampimenergy = [432839.7397, 216315.652, 108063.0825, 663368.6419, 238902.5095, 111928.6185, 543405.4391, 242786.9942, 113027.6434]

rrampimlatency = [2489491.808, 1244195.904, 621597.952, 20621991.81, 1363395.904, 641997.952, 3072991.808, 1383895.904, 647797.952]
rrampimenergy = [570988.9652, 284307.8352, 141075.1472, 7319953.507, 541486.6985, 185088.9795, 1829914.022, 585716.2849, 197602.7162]

cpulatency = [5548658.75, 5476784, 5104411.25, 5559826.75, 5485236.75, 5197111.75, 5473174.25, 5464983.25, 5078152.5]
cpuenergy = [196051258.8, 163386645.4, 148743115.3, 212356346.8, 205740783.6, 153102440.7, 176661651.1, 164425856.2, 148884225.9]

srampimspeedup = cpulatency[:]
rrampimspeedup = cpulatency[:]
srampimefficiencyup = cpuenergy[:]
rrampimefficiencyup = cpuenergy[:]


xbar = np.arange(9)
total_width, n = 1.6, 6
width = total_width / n
xbar = xbar - (total_width - width) / 2

for i in range(9):
    srampimspeedup[i] = srampimspeedup[i] / srampimlatency[i]
    rrampimspeedup[i] = rrampimspeedup[i] / rrampimlatency[i]
    srampimefficiencyup[i] = srampimefficiencyup[i] / srampimenergy[i]
    rrampimefficiencyup[i] = rrampimefficiencyup[i] / rrampimenergy[i]

fig=plt.figure(figsize=(10,6))

ax=plt.subplot(111)
plt.tick_params(labelsize=19)
plt.xticks(xbar+0.5*width,testname,fontsize=18,position=(0,0.02))
srampimspeedup=np.array(srampimspeedup)
rrampimspeedup=np.array(rrampimspeedup)
srampimefficiencyup=-np.array(srampimefficiencyup)
rrampimefficiencyup=-np.array(rrampimefficiencyup)

bar1 = ax.bar(xbar+0*width, srampimspeedup, color=colors[7], width=width, label="SRAM-PIM")
bar2 = ax.bar(xbar+1*width, rrampimspeedup, color=colors[8], width=width, label="ReRAM-PIM")
bar3 = ax.bar(xbar+0*width, srampimefficiencyup, color=colors[9], width=width, label="SRAM-PIM")
bar4 = ax.bar(xbar+1*width, rrampimefficiencyup, color=colors[10], width=width, label="ReRAM-PIM")
ax.set_ylim(-30000, 5000)
ax.axhline(y=0, color='black', linestyle='--', linewidth=1)
ax.set_yscale('symlog')
# plt.title("Throughput/Efficiency Improvement vs. CPU",fontsize=23)
plt.ylabel("性能提升比",fontsize=20,labelpad=0)
# 创建上半区的图例
legend_upper = plt.legend(handles=[bar1[0], bar2[0]], labels=['SRAM-PIM', 'ReRAM-PIM'],frameon=False,
                           loc='upper center', ncol=2, bbox_to_anchor=(0.5, 1.043), prop={'size': 12})

# 创建下半区的图例
legend_lower = plt.legend(handles=[bar3[0], bar4[0]], labels=['SRAM-PIM', 'ReRAM-PIM'],frameon=False,
                           loc='lower center', ncol=2, bbox_to_anchor=(0.5, -0.043), prop={'size': 12})


# legend_upper.set_frame_on(False)
# legend_lower.set_frame_on(False)
# legend_upper.get_frame().set_alpha(0.25)
for lh in legend_upper.legendHandles:
    lh.set_alpha(0.8)  # 设置每个图例项的透明度
# legend_lower.get_frame().set_alpha(0.25)
for lh in legend_lower.legendHandles:
    lh.set_alpha(0.8)  # 设置每个图例项的透明度

legend_upper.get_texts()[0].set_alpha(0.9)  # 设置第一个图例项的透明度
legend_upper.get_texts()[1].set_alpha(0.9)  # 设置第二个图例项的透明度

legend_lower.get_texts()[0].set_alpha(0.9)  # 设置第一个图例项的透明度
legend_lower.get_texts()[1].set_alpha(0.9)  # 设置第二个图例项的透明度

# 添加图例到子图
ax.add_artist(legend_upper)
ax.add_artist(legend_lower)

def abs_formatter(x, pos):
    num = int(math.log(abs(int(x))+0.0001,10))
    res = f"$10^{num}$"
    if (x==0):
        res = f"0"

    return res
ax.yaxis.set_major_formatter(FuncFormatter(abs_formatter))

for bar in bar1:
    yval = bar.get_height()
    ax.text(bar.get_x() + bar.get_width()/2, yval, round(yval, 1), ha='center', va='bottom', color='black')

for bar in bar2:
    yval = bar.get_height()
    ax.text(bar.get_x() + bar.get_width()/2, yval, round(yval, 1), ha='center', va='bottom', color='black')

for bar in bar3:
    yval = bar.get_height()
    ax.text(bar.get_x() + bar.get_width()/2, yval*1.5, -round(yval, 1), ha='center', va='top', color='black')

for bar in bar4:
    yval = bar.get_height()
    ax.text(bar.get_x() + bar.get_width()/2, yval, -round(yval, 1), ha='center', va='top', color='black')


for tick in ax.get_xticklabels():
    tick.set_rotation(15)

plt.yticks(position=(0.015,0),ha='left',fontsize=13,alpha=0.7)
plt.subplots_adjust(left=0.05, right=0.98, bottom=0.28, top=0.73)

# ax=plt.subplot(121)
# plt.tick_params(labelsize=19)
# plt.xticks(xbar+0.5*width,testname)
# plt.bar(xbar+0*width, np.array(srampimspeedup), color=colors[7], width=width, label="2MB SRAM-PIM")
# plt.bar(xbar+1*width, np.array(rrampimspeedup), color=colors[8], width=width, label="8MB ReRAM-PIM")
# plt.title("Speedup",fontsize=24)
# plt.ylabel("Rate",fontsize=24)
# plt.semilogy()
# for tick in ax.get_xticklabels():
#     tick.set_rotation(20)

# ax=plt.subplot(122)
# plt.tick_params(labelsize=19)
# plt.xticks(xbar+0.5*width,testname)
# plt.bar(xbar+0*width, np.array(srampimefficiencyup), color=colors[7], width=width, label="2MB SRAM-PIM")
# plt.bar(xbar+1*width, np.array(rrampimefficiencyup), color=colors[8], width=width, label="8MB ReRAM-PIM")
# plt.title("Efficiency Improvement",fontsize=24)
# plt.ylabel("Rate",fontsize=24)
# plt.semilogy()
# for tick in ax.get_xticklabels():
#     tick.set_rotation(20)
# plt.subplots_adjust(left=0.05, right=0.98, bottom=0.2, top=0.78, wspace=0.15)



# lines_labels = ax.get_legend_handles_labels()
# lines, labels = [sum(lol, []) for lol in zip(lines_labels)]
# fig.legend(lines, labels,fontsize=24,ncol=6,bbox_to_anchor=(0.5,1.01),loc='upper center',frameon=False)

plt.savefig('figures/PIM_CPU_comparison.pdf')



print("2MB SRAM n=1048576 geomean throughput boost: ",sramthroughput[12]/simdsramthroughput[12])
print("2MB SRAM n=1048576 geomean efficiency boost: ",sramefficiency[12]/simdsramefficiency[12])
print("8MB ReRAM n=1048576 geomean throughput boost: ",rramthroughput[12]/simdrramthroughput[12])
print("8MB ReRAM n=1048576 geomean efficiency boost: ",rramefficiency[12]/simdrramefficiency[12])
print("2MB SRAM n=1024 geomean throughput boost: ",smallsramthroughput[12]/smallsimdsramthroughput[12])
print("2MB SRAM n=1024 geomean efficiency boost: ",smallsramefficiency[12]/smallsimdsramefficiency[12])
print("8MB ReRAM n=1024 geomean throughput boost: ",smallrramthroughput[12]/smallsimdrramthroughput[12])
print("8MB ReRAM n=1024 geomean efficiency boost: ",smallrramefficiency[12]/smallsimdrramefficiency[12])

print("2MB SRAM speedup: ",speedup["2MB-SRAM"][1])
print("8MB ReRAM speedup: ",speedup["8MB-ReRAM"][1])
