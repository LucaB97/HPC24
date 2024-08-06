#/usr/bin/bash

##get info for the execution
node=$1
cpus=$2
repetitions=$3
operation=$4

ALGS=(0 1 2 3 4)
MAPPING=("core" "L3cache" "numa" "socket")
ITERATIONS=(10000 20000)
WARMUP=(200 1000)

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
size_iters=${#ITERATIONS[@]}
size_warmup=${#WARMUP[@]}

cd ../operations/
module purge
module load openMPI/4.1.5/gnu


printf "\nOperation: ${operation}\nNode: ${node}" > "${operation}_all_results_${node}.txt"

# For-loop on the used algorithms
for R in $(seq 1 $repetitions)
do
    alg_idx=$(($RANDOM % $size_algs))
    numprocs_idx=$(($RANDOM % $size_numprocs))
    iters_idx=$(($RANDOM % $size_iters))
    warmup_idx=$(($RANDOM % $size_warmup))
    
    curr_alg=${ALGS[$alg_idx]}
    curr_numprocs=${NUMPROCS[$numprocs_idx]}
    curr_iters=${ITERATIONS[$iters_idx]}
    curr_warmup=${WARMUP[$warmup_idx]}

    printf "\n\n--------------------------------------------
Test: $R\n
Algorithm: $curr_alg
Procs: $curr_numprocs
Iterations: $curr_iters
Warmup: $curr_warmup
--------------------------------------------\n" >> "${operation}_all_results_${node}.txt"

    for M in "${MAPPING[@]}"
    do
        printf "\nMapping: $M" >> "${operation}_all_results_${node}.txt"
        mpirun -np $curr_numprocs --map-by $M --mca coll_tuned_use_dynamic_rules true --mca coll_tuned_${operation}_algorithm $curr_alg osu_${operation} -f -i $curr_iters -x $curr_warmup > "curr_results_${node}.txt"
        cat "curr_results_${node}.txt" >> "${operation}_all_results_${node}.txt"
    done
done
    
printf "\n-------------------------------------------\n" >> "${operation}_all_results_${node}.txt"

##rm summarize
rm curr_results_${node}.txt
X=$(date +%m-%d--%H-%M)
mv ${operation}_all_results_${node}.txt ../tuning/mapping/$X.txt
