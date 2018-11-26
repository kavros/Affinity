#B145772
import os
import operator
import numpy as np
import matplotlib.pyplot as plt

# key is number of thread and the value is the mean execution time
affinityLoop1=dict()
affinityLoop2=dict()
guidedLoop1=dict()
dynamicLoop2=dict()

# calculates the average time for loop 1 and loop 2 using the files
# that qsub produce
def GetAverageTime(lines,target):
	sumT=0.0;
	cnt=0;
	for line in lines:
		if(target in line):
			loop1Time =float(line.split("=")[1].strip());
			sumT=sumT+loop1Time
			cnt = cnt +1
	averageTime =  sumT/cnt
	return averageTime

# read a log files and initialize dictionaries
def Init():
	path = "../results/affinity/"
	for fileName in os.listdir(path):
		f = open(path+fileName,"r")
		lines =  f.readlines();
		nThreads = int(fileName.split(".")[0].split("_")[1].strip());
		
		affinityLoop1[nThreads] = GetAverageTime(lines,"loop 1 =")
		affinityLoop2[nThreads] = GetAverageTime(lines,"loop 2 =")
		
	
	path ="../results/part1_best_loop1/"
	for fileName in os.listdir(path):
		f = open(path+fileName,"r")
		lines =  f.readlines();
		nThreads = int(fileName.split(".")[0].split("_")[2]);
		guidedLoop1[nThreads] = GetAverageTime(lines,"loop 1 =")
	
	path ="../results/part1_best_loop2/"
	for fileName in os.listdir(path):
		f = open(path+fileName,"r")
		lines =  f.readlines();
		nThreads = int(fileName.split(".")[0].split("_")[2]);
		dynamicLoop2[nThreads] = GetAverageTime(lines,"loop 2 =")	
	
def GetAxis(dictionary):
	xAxisContent=[]
	yAxisContent=[]
	for key in sorted(dictionary.iterkeys()):
		xAxisContent.append(key)
		yAxisContent.append(dictionary[key])
	return xAxisContent,yAxisContent

# generate speedup and total time graphs
def GenerateLineChart(dict1,dict2,schedule1_type,schedule2_type,fileName):
	x1,y1 = GetAxis(dict1)
	x2,y2 = GetAxis(dict2)
	fig, ax = plt.subplots()
	addExpectedSpeedup(fileName,plt,dict1)	
	plt.plot(x1,y1,marker="x")
	plt.plot(x2,y2,marker="x")
	ax.set(xlabel="number of threads", ylabel="total time in secs")	
	ax.grid()
	if("speedup" not in fileName):
		plt.legend([schedule1_type, schedule2_type], loc=1)
	else:
		plt.legend(["ideal",schedule1_type, schedule2_type], loc=2)
	#plt.show()
	path="../results/graphs/"
	plt.savefig((path+fileName), format='eps', dpi=1000)

# helper function for calculation of speedup.
def GetSpeedup(target,src):
	for key in src.keys():
		target[key] = src[1]/src[key]


# adds expected speedip to speedup graph
def addExpectedSpeedup(fileName,plt,dict1):
	if("speedup" not in fileName):
		return
	print fileName
	expectedSpeedup=dict()
	for key in dict1.keys():
		expectedSpeedup[key] = key
	x3,y3 = GetAxis(expectedSpeedup)
	plt.plot(x3,y3,marker="x")


def main():
	# initalize dictionaries with total time
	Init()	

	# generate total time line charts
	GenerateLineChart(affinityLoop1,guidedLoop1,"affinity","guided,1","loop1.eps")
	GenerateLineChart(affinityLoop2,dynamicLoop2,"affinity","dynamic,16","loop2.eps")
	
	affinityLoop1Speedup=dict()
	affinityLoop2Speedup=dict()
	guidedLoop1Speedup=dict()
	dynamicLoop2Speedup=dict()


	# initialize dictionaries with speedup
	GetSpeedup(affinityLoop1Speedup,affinityLoop1)
	GetSpeedup(affinityLoop2Speedup,affinityLoop2)
	GetSpeedup(guidedLoop1Speedup,guidedLoop1)
	GetSpeedup(dynamicLoop2Speedup,dynamicLoop2)

	# generate speedup line charts for speedup
	GenerateLineChart(affinityLoop1Speedup,guidedLoop1Speedup,"affinity","guided,1","loop1_speedup.eps")
	GenerateLineChart(affinityLoop2Speedup,dynamicLoop2Speedup,"affinity","dynamic,16","loop2_speedup.eps")

	print affinityLoop1Speedup
	print guidedLoop1Speedup

	
if __name__ == "__main__":
	main()

