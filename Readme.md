# Build and Run
* Build project using ```make```
* Run project on fronted of cirrus using:
 ```
export OMP_NUM_THREADS=4
make run
 ```
* Run on the backend of Cirrus using:
 ```
 cd scripts
./run.sh
 ```
* The results are in folder results/affinity

# Generate Graphs
* Run the following commands to generate 

 ```
cd scripts
python generateGraphs.py

 ```
* The results are in folder results/graphs
