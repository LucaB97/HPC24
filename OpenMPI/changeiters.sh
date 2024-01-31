#/usr/bin/bash

##get info for the execution
node=$1
cpus=$2
operation=$3
repetitions=$4

ALGS=(0 1 2 3 4)
MAPPING=("core" "L3cache" "numa" "socket")
ITERATIONS=(10000 20000 50000 100000)
WARMUP=(200 1000 2000 5000)

##create array of values for number of processes (np) for mpirun
k=2
i=0
while [[ $k -le $cpus ]] ; do
NUMPROCS[$i]=$k
if [ $k -le $((16)) ]; then
	k=$((2*k))
else
	k=$((k+16))
fi
i=$((i+1))
done

size_algs=${#ALGS[@]}
size_numprocs=${#NUMPROCS[@]}
size_mapping=${#MAPPING[@]}
size_warmup=${#WARMUP[@]}

###c++ summarize.cpp -o summarize
###mv summarize operations/
cd operations/
module purge
module load openMPI/4.1.5/gnu


printf "\nOperation: ${operation}  , Node: ${node}" > "${operation}_all_results_${node}.txt"

# For-loop on the used algorithms
for R in $(seq 1 $repetitions)
do
    alg_idx=$(($RANDOM % $size_algs))
    numprocs_idx=$(($RANDOM % $size_numprocs))
    mapping_idx=$(($RANDOM % $size_mapping))
    warmup_idx=$(($RANDOM % $size_warmup))
    
    curr_alg=${ALGS[$alg_idx]}
    curr_numprocs=${NUMPROCS[$numprocs_idx]}
    curr_mapping=${MAPPING[$mapping_idx]}
    curr_warmup=${WARMUP[$warmup_idx]}

    printf "\n\n--------------------------------------------\n
Test: $R\n
Algorithm: $curr_alg
Procs: $curr_numprocs
Mapping: $curr_mapping
Warmup: $curr_warmup
--------------------------------------------\n" >> "${operation}_all_results_${node}.txt"

    for I in "${ITERATIONS[@]}"
    do
        printf "\nIterations: $I\n" >> "${operation}_all_results_${node}.txt"
        mpirun -np $curr_numprocs --map-by $curr_mapping --mca coll_tuned_use_dynamic_rules true --mca coll_tuned_${operation}_algorithm $curr_alg osu_${operation} -f -z -i $I -x $curr_warmup > "curr_results_${node}.txt"
        cat "curr_results_${node}.txt" >> "${operation}_all_results_${node}.txt"
    done
done
    
printf "\n-------------------------------------------\n" >> "${operation}_all_results_${node}.txt"

##rm summarize
rm curr_results_${node}.txt
X=$(date +%m-%d--%H-%M)
mv ${operation}_all_results_${node}.txt ../results/hypo1/$X-all.txt
