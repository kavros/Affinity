//B145772
#!/bin/bash
AFFINITY_EXE=../build/bin/loops2
AFFINITY_OUTPUT_FOLDER=../results/affinity
AFFINITY_ERRORS_FOLDER=../errors/affinity

# run affinity scheduler with diferent number of threads
# and save log files in output folders
for i in 1 2 4 6 8 12 16 
do
	qsub -q R380254 -o "$AFFINITY_OUTPUT_FOLDER/loops2_$i.txt" \
     -e "$AFFINITY_ERRORS_FOLDER/loops_$i.txt" \
     -v executable="$AFFINITY_EXE",numberOfThreads=$i loops.pbs		
done


#-q R380254
