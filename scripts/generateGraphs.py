import os
import operator
import numpy as np
import matplotlib.pyplot as plt

# key is number of thread and the value is the mean execution time
affinityLoop1=dict()
affinityLoop2=dict()
guidedLoop1=dict()
dynamicLoop2=dict()

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

def GenerateLineChart(dict1,dict2,schedule1_type,schedule2_type,fileName):
	x1,y1 = GetAxis(dict1)
	x2,y2 = GetAxis(dict2)

	fig, ax = plt.subplots()
	plt.plot(x1,y1,marker="x")
	plt.plot(x2,y2,marker="x")
	ax.set(xlabel="number of threads", ylabel="total time in secs")	
	ax.grid()
	plt.legend([schedule1_type, schedule2_type], loc=1)
	#plt.show()
	path="../results/graphs/"
	plt.savefig((path+fileName), format='eps', dpi=1000)
	
	
		

def main():
	Init()	
	GenerateLineChart(affinityLoop1,guidedLoop1,"affinity","guided,1","loop1.eps")
	GenerateLineChart(affinityLoop2,dynamicLoop2,"affinity","dynamic,16","loop2.eps")
	

if __name__ == "__main__":
	main()

