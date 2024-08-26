#!/bin/bash

CPUS=$1
SIZE=$2
REPETITIONS=$3

##create array of values for number of processes (np) for mpirun
k=1
i=0
while [[ $k -le $CPUS ]] ; do
NUMPROCS[$i]=$k
k=$((k+2))
i=$((i+1))
done

##load modules & create executable for the algorithm
module purge
module load openMPI/4.1.6/gnu
mpicc -O3 hybrid_qsort_scatter.c -o qsort

##perform strong scalability test
printf "Test,Processes,Size,Total,Communication,Sorting,Merging\n" > "strong.csv"

for P in "${NUMPROCS[@]}"
do
    for R in $(seq 1 $REPETITIONS)
    do
        printf "$R," >> "strong.csv"
        mpirun -np $P ./qsort $SIZE >> "strong.csv"
    done
done

mv strong.csv results/
module purge
rm qsort
