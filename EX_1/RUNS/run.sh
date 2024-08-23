#!/bin/bash

##get info for the execution
NODE=$1
CPUS=$2
REPETITIONS=$3
OPERATION=$4
ALGS=("${@:5}")

X=$(date +%m-%d--%H-%M)
mkdir results/$NODE/$X

##create array of values for number of processes (np) for mpirun
k=2
i=0
while [[ $k -le $CPUS ]] ; do
num_procs[$i]=$k
k=$((k+2))
i=$((i+1))
done

cp summary.py ../OPERATIONS/
cd ../OPERATIONS/
module purge
module load openMPI/4.1.6/gnu


# For-loop on the used algorithms
for A in "${ALGS[@]}"
do
    printf "\n\n\n--------------------------------------------\nAlgorithm: $A
--------------------------------------------\n" >> "${OPERATION}__all.txt"
    printf "\nAlgorithm: $A\n" >> "${OPERATION}__${A}.txt"

    # For-loop on the number of processes
    for P in "${num_procs[@]}"
    do        
        printf "\n\nProcs: $P\n" >> "${OPERATION}__all.txt"
        printf "\nProcs: $P\n" >> "${OPERATION}__${A}.txt"        

        # Running some repetitions for each parameter configuration
        for R in $(seq 1 $REPETITIONS)
        do
            printf "\nTest: $R" >> "${OPERATION}__all.txt"
            printf "\nTest: $R" >> "${OPERATION}__${A}.txt"
            mpirun -np $P --mca coll_tuned_use_dynamic_rules true --mca coll_tuned_${OPERATION}_algorithm $A osu_${OPERATION} -f -z > "curr_results.txt"
            cat "curr_results.txt" >> "${OPERATION}__${A}.txt"
            cat "curr_results.txt" >> "${OPERATION}__all.txt"
        done
        
	# Summarize the data of each parameter configuration (compute mean and standard deviation of latency measures)
        mv ${OPERATION}__${A}.txt ../RUNS/results/$NODE/$X/
    done
done

rm curr_results.txt
python3 summary.py ${OPERATION}__all.txt ${OPERATION}__summarized.csv
rm summary.py
mv ${OPERATION}__all.txt ../RUNS/results/$NODE/$X/
mv ${OPERATION}__summarized.csv ../RUNS/results/$NODE/$X/
