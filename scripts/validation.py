//B145772
import os
import filecmp

sumLoop1="-343021.474766"
sumLoop2="-25242644.603147"

# read all files inside the directory affinity and validate
# that sums are correct
path="../results/affinity/"
for fileName in os.listdir(path):
	#print fileName
	f = open(path+fileName,"r")
	lines =  f.readlines();		
	for line in lines:
		if("Sum of a" in line):
			currSumLoop1=line.split(" ")[7].strip()
			assert(currSumLoop1 == sumLoop1)
		if("Sum of c" in line):
			currSumLoop2=line.split(" ")[7].strip()
			assert(currSumLoop2 == sumLoop2)

print "Tests passed successfully!"