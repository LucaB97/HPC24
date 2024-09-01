#!/bin/bash

# CPUS=$1
SIZE=$1
REPETITIONS=$2

##create array of values for number of processes (np) for mpirun
# k=1
# i=0
# while [[ $k -le $CPUS ]] ; do
# NUMPROCS[$i]=$k
# k=$((2*k))
# i=$((i+1))
# done
NUMPROCS=(1 2 4 8 16 32 64 128 256)

##load modules & create executable for the algorithm
# module purge
# module load openMPI/4.1.6/gnu
mpicc -O3 hybrid_qsort.c -o qsort

##perform strong scalability test
printf "Test\tProcesses\tSize\t\tTotal\t\tCommunication\tSorting\t\tMerging\n" > "strong.txt"

for P in "${NUMPROCS[@]}"
do
    for R in $(seq 1 $REPETITIONS)
    do
        printf "$R\t\t" >> "strong.txt"
        mpirun -np $P ./qsort $SIZE >> "strong.txt"
    done
done

mv strong.txt results/
# module purge
rm qsort
