#/usr/bin/bash

X=$(date +%m-%d--%H-%M)
mkdir tests/$X

##get info for the execution
node=$1
cpus=$2
# repetitions=$3

## chosen after previous test on total iterations and warmup iterations 
ITERATIONS=10000
WARMUP=200

## these parameters will be generated randomly at each repetition:
## the idea is to find out if it is possible to study the impact of **total iterations** and **warmup iterations**
## on performances independently on the specific execution 
OPERATION="bcast"
ALGS=(0 2 5 6 7)

## ITERATION ON THESE VALUES
MAPPING=("core" "L3cache" "numa" "socket" "node")

cd ../../operations/
module purge
module load openMPI/4.1.6/gnu


# For-loop on the used algorithms
for A in "${ALGS[@]}"
do
    for M in "${MAPPING[@]}"
    do
        printf "\nMapping: $M" >> "bcast__${A}.txt"
        mpirun -np $cpus --map-by $M --mca coll_tuned_use_dynamic_rules true --mca coll_tuned_${OPERATION}_algorithm $A osu_${OPERATION} -f -z > "curr_results.txt"
        cat "curr_results.txt" >> "bcast__${A}.txt"
    done
    mv "bcast__${A}.txt" ../parameters_study/mapping/tests/$X/
done

rm curr_results.txt
