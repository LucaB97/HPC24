#/usr/bin/bash

##get info for the execution
NODE=$1
CPUS=$2
OPERATION=$3
REPETITIONS=$4
ALGS=("${@:5}")
MAPPING=("core" "L3cache" "numa" "socket")
ITERATIONS=(10000 30000 100000)

##create array of values for number of processes (np) for mpirun
k=2
i=0
while [[ $k -le $CPUS ]] ; do
num_procs[$i]=$k
k=$((2*k))
i=$((i+1))
done

###c++ summarize.cpp -o summarize
###mv summarize operations/
cd operations/
module purge
module load openMPI/4.1.5/gnu


printf "Operation: ${OPERATION}  , Node: ${NODE}" > "${OPERATION}_all_results_${NODE}.txt"

# For-loop on the used algorithms
for A in "${ALGS[@]}"
do
    printf "\n\n\n--------------------------------------------\nAlgorithm: $A
--------------------------------------------\n" >> "${OPERATION}_all_results_${NODE}.txt"
    for M in "${MAPPING[@]}"
    do
        printf "\n\n**Mapping: $M**" >> "${OPERATION}_all_results_${NODE}.txt"
    # For-loop on the number of processes
        for P in "${num_procs[@]}"
        do
            printf "\n\nProcs: $P\n" >> "${OPERATION}_all_results_${NODE}.txt"

            # Running some repetitions for each parameter configuration
            for X in "${ITERATIONS[@]}"
            do
                printf "\n\nIter: $X , Warmup: $((X/50))" >> "${OPERATION}_all_results_${NODE}.txt"
                for R in $(seq 1 $REPETITIONS)
                do
                    printf "\nRepetition: $R" >> "${OPERATION}_all_results_${NODE}.txt"
                    mpirun -np $P --map-by $M --mca coll_tuned_use_dynamic_rules true --mca coll_tuned_${OPERATION}_algorithm $A osu_${OPERATION} -f -z -i $X -x $((X/50)) > "curr_results_${NODE}.txt"
                    cat "curr_results_${NODE}.txt" >> "${OPERATION}_all_results_${NODE}.txt"
                done
            done
        done
    done
done
printf "\n-------------------------------------------\n" >> "${OPERATION}_all_results_${NODE}.txt"

##rm summarize
rm curr_results_${NODE}.txt
X=$(date +%m-%d--%H-%M)
mv ${OPERATION}_all_results_${NODE}.txt ../results/hypo1/$X-all.txt
