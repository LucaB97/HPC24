#/usr/bin/bash

X=$(date +%m-%d--%H-%M)
mkdir tests/$X

##get info for the execution
node=$1
cpus=$2
repetitions=$3

## chosen after previous test on total iterations and warmup iterations 
ITERATIONS=10000
WARMUP=200

## these parameters will be generated randomly at each repetition:
## the idea is to find out if it is possible to study the impact of **total iterations** and **warmup iterations**
## on performances independently on the specific execution 
OPERATIONS=("bcast" "scatter" "gather" "reduce" "barrier")
ALGS=(0 1 2 3)

size_operations=${#OPERATIONS[@]}
size_algs=${#ALGS[@]}

## ITERATION ON THESE VALUES
MAPPING=("core" "L3cache" "numa" "socket")


cd ../../operations/
module purge
module load openMPI/4.1.5/gnu


# For-loop on the used algorithms
for R in $(seq 1 $repetitions)
do  
    # printf "Test $R"
    operation_idx=$(($RANDOM % $size_operations))
    alg_idx=$(($RANDOM % $size_algs))
    
    curr_operation=${OPERATIONS[$operation_idx]}
    curr_alg=${ALGS[$alg_idx]}

    printf "Node: $node     CPUS: $cpus\nIterations: $ITERATIONS     Warmup: $WARMUP\n\n--------------------------------------------
Operation: $curr_operation
Algorithm: $curr_alg
--------------------------------------------\n" > "test__${R}.txt"

    for M in "${MAPPING[@]}"
    do
        # printf "\nMapping: $M"
        printf "\nMapping: $M" >> "test__${R}.txt"
            mpirun -np $cpus --map-by $M --mca coll_tuned_use_dynamic_rules true --mca coll_tuned_${curr_operation}_algorithm $curr_alg osu_${curr_operation} -f -i $ITERATIONS -x $WARMUP > "curr_results_{$R}.txt"
            cat "curr_results_{$R}.txt" >> "test__${R}.txt"
    done
    # printf "\n"
    mv "test__${R}.txt" ../parameters_study/mapping/tests/$X/
done

rm curr_results_{$R}.txt
