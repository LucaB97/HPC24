#!/bin/bash

##get info for the execution
NODE=$1
CPUS=$2
REPETITIONS=$3
OPERATION=$4
ALG=$5

##create array of values for number of processes (np) for mpirun
k=2
i=0
while [[ $k -le $CPUS ]] ; do
num_procs[$i]=$k
k=$((k+2))
i=$((i+1))
done

mkdir results/$NODE/$OPERATION/alg__$ALG
cp summary.py ../OPERATIONS/
cd ../OPERATIONS/
module purge
module load openMPI/4.1.6/gnu


printf "\n\n\n--------------------------------------------\nAlgorithm: $ALG
--------------------------------------------\n" >> "${OPERATION}__all.txt"
printf "\nAlgorithm: $ALG\n" >> "${OPERATION}__${ALG}.txt"

# For-loop on the number of processes
for P in "${num_procs[@]}"
do        
    printf "\n\nProcs: $P\n" >> "${OPERATION}__all.txt"
    printf "\nProcs: $P\n" >> "${OPERATION}__${ALG}.txt"        

    # Running some repetitions for each parameter configuration
    for R in $(seq 1 $REPETITIONS)
    do
        printf "\nTest: $R" >> "${OPERATION}__${ALG}.txt"
        mpirun -np $P --mca coll_tuned_use_dynamic_rules true --mca coll_tuned_${OPERATION}_algorithm $ALG osu_${OPERATION} -f -z > "curr__${OPERATION}_${ALG}.txt"
        cat "curr__${OPERATION}_${ALG}.txt" >> "${OPERATION}__${ALG}.txt"
    done
done

rm curr__${OPERATION}_${ALG}.txt
python3 summary.py ${OPERATION}__${ALG}.txt ${OPERATION}__${ALG}__summarized.csv
rm summary.py
mv ${OPERATION}__${ALG}.txt ../RUNS/results/$NODE/$OPERATION/alg__$ALG/all.txt
mv ${OPERATION}__${ALG}__summarized.csv ../RUNS/results/$NODE/$OPERATION/alg__$ALG/summarized.csv
