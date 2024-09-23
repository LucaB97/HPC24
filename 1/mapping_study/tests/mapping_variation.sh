#/usr/bin/bash

##get info for the execution
node=$1
cpus=$2
operation=$3
algorithms=("${@:4}")

mkdir $node/
mkdir $node/$operation

## ITERATION ON THESE VALUES
MAPPINGS=("core" "numa" "node")

cd ../../OPERATIONS/
module purge
module load openMPI/4.1.6/gnu

# For-loop on the used algorithms
for A in "${algorithms[@]}"
do
    for M in "${MAPPINGS[@]}"
    do
        printf "\nMapping: $M" >> "${operation}__alg__${A}.txt"
        mpirun -np $cpus --map-by $M --mca coll_tuned_use_dynamic_rules true --mca coll_tuned_${operation}_algorithm $A osu_${operation} -f -z >> "${operation}__alg__${A}.txt"
    done
    
    mv "${operation}__alg__${A}.txt" ../mapping_study/tests/$node/$operation/alg__${A}.txt
done

bash results_to_plots.sh $node $operation $algorithms