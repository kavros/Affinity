# Build and Run
* Build project using ```make```
* Run project on fronted of cirrus using:
 ```
export OMP_NUM_THREADS=4
make run
 ```
* Run on the backend of Cirrus and validate the results using:
 ```
 cd scripts
./run.sh
python validation.py
 ```
* The results are saved inside folder results/affinity

# Generate Graphs
* Run the following commands to generate 

 ```
cd scripts
python generateGraphs.py

 ```
* The graphs are saved inside folder results/graphs
