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
size_mapping=${#MAPPING[@]}
size_iterations=${#ITERATIONS[@]}
size_warmup=${#WARMUP[@]}
size_numprocs=${#NUMPROCS[@]}

###c++ summarize.cpp -o summarize
###mv summarize operations/
cd operations/
module purge
module load openMPI/4.1.5/gnu


printf "Operation: ${operation}  , Node: ${node}" > "${operation}_all_results_${node}.txt"

# For-loop on the used algorithms
for R in $(seq 1 $REPETITIONS)
do
    curr_alg=$(($RANDOM % $size_algs))
    curr_mapping=$(($RANDOM % $size_mapping))
    curr_iters=$(($RANDOM % $size_iterations))
    curr_warmup=$(($RANDOM % $size_warmup))
    curr_numprocs=$(($RANDOM % $size_numprocs))
    
    printf "\n\n--------------------------------------------\n
Repetition: $R\n\n
Algorithm: ${ALGS[$curr_alg]}\n
Procs: ${NUMPROCS[$curr_numprocs]}\n
Mapping: ${MAPPING[$curr_mapping]}\n
Iterations: ${ITERATIONS[$curr_iters]}\n
Warmup: ${WARMUP[$curr_warmup]}\n >> "${operation}_all_results_${node}.txt"

    mpirun -np ${NUMPROCS[$curr_numprocs]} --map-by ${MAPPING[$curr_mapping]} --mca coll_tuned_use_dynamic_rules true --mca coll_tuned_${operation}_algorithm ${ALGS[$curr_alg]} osu_${operation} -f -z -i ${ITERATIONS[$curr_iters]} -x ${WARMUP[$curr_warmup]} > "curr_results_${node}.txt"
    cat "curr_results_${node}.txt" >> "${operation}_all_results_${node}.txt"
done
    
printf "\n-------------------------------------------\n" >> "${operation}_all_results_${node}.txt"

##rm summarize
rm curr_results_${node}.txt
X=$(date +%m-%d--%H-%M)
mv ${operation}_all_results_${node}.txt ../results/hypo1/$X-all.txt
