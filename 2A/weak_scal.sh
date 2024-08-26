#!/bin/bash

CPUS=$1
SIZE=$2
REPETITIONS=$3

##create array of values for number of processes (np) for mpirun
k=1
i=0
while [[ $k -le $CPUS ]] ; do
NUMPROCS[$i]=$k
k=$((2*k))
i=$((i+1))
done

##load modules & create executable for the algorithm
# module purge
# module load openMPI/4.1.6/gnu
mpicc -O3 hybrid_qsort.c -o qsort

##perform strong scalability test
printf "Test\tProcesses\tSize\t\tTotal\t\tCommunication\tSorting\t\tMerging\n" > "weak.txt"

for P in "${NUMPROCS[@]}"
do
    for R in $(seq 1 $REPETITIONS)
    do
        printf "$R\t\t" >> "weak.txt"
        mpirun -np $P ./qsort $((P*SIZE)) >> "weak.txt"
    done
done

mv weak.txt results/
# module purge
rm qsort
