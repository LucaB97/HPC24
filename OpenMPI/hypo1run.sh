#/usr/bin/bash

##get info for the execution
NODE=$1
CPUS=$2
OPERATION=$3
###REPETITIONS=$4
ALGS=("${@:4}")
ITERATIONS=(1 10 100 1000 10000 100000 1000000)

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


# For-loop on the used algorithms
for A in "${ALGS[@]}"
do
    printf "\n\n\n--------------------------------------------\nAlgorithm: $A
--------------------------------------------\n" >> "${OPERATION}_all_results_${NODE}.txt"

    # For-loop on the number of processes
    for P in "${num_procs[@]}"
    do
        ###printf "\nAlgorithm: $A\n" > "tmp_results_${NODE}.txt"
        ###printf "\nProcs: $P\n" >> "tmp_results_${NODE}.txt"
        printf "\n\nProcs: $P\n" >> "${OPERATION}_all_results_${NODE}.txt"

        # Running some repetitions for each parameter configuration
        ###for X in $(seq 1 $REPETITIONS)
        for X in "${ITERATIONS[@]}"
        do
            printf "\nIter: $X" >> "${OPERATION}_all_results_${NODE}.txt"
            printf "\nIter: $X" >> "tmp_results_${NODE}.txt"
            mpirun -np $P --map-by core --mca coll_tuned_use_dynamic_rules true --mca coll_tuned_${OPERATION}_algorithm $A osu_${OPERATION} -f -z -i $X > "curr_results_${NODE}.txt"
            ###cat "curr_results_${NODE}.txt" >> "tmp_results_${NODE}.txt"
            cat "curr_results_${NODE}.txt" >> "${OPERATION}_all_results_${NODE}.txt"
        done
	# Summarize the data of each parameter configuration (compute mean and standard deviation of latency measures)
        ###./summarize tmp_results_${NODE}.txt ${OPERATION}_summarized_results_${NODE}.csv
    done
done
printf "\n-------------------------------------------\n" >> "${OPERATION}_all_results_${NODE}.txt"

##rm summarize
rm curr_results_${NODE}.txt
###rm tmp_results_${NODE}.txt
X=$(date +%m-%d--%H-%M)
mv ${OPERATION}_all_results_${NODE}.txt ../results/hypo1/$OPERATION/$NODE/$X-all.txt
###mkdir ../results/$OPERATION/$NODE/$X
###mv ${OPERATION}_all_results_${NODE}.txt ../results/$OPERATION/$NODE/$X/all.txt
###mv ${OPERATION}_summarized_results_${NODE}.csv ../results/$OPERATION/$NODE/$X/summarized.csv
