#!/bin/bash

NODES=$1
SIZE=$2
REPETITIONS=$3
NUMPROCS=64

srun --nodes=$NODES --ntasks-per-node=1 free > "memory"
free_memory=$(grep Mem memory | awk '{print $4}' | sort -n | head -n 1)  #in kibibytes (KiB), where 1 KiB = 1024 bytes
gcc -o memory_req memory_req.c
data_size=$(./memory_req)  # Size of each data entry in bytes
max_segment_size=$(echo "scale=0; 0.25 * 1024 * $free_memory / (3 * $data_size * $NUMPROCS)" | bc)
printf "max segment size: $max_segment_size\n"

#if [ $SIZE -ge $max_segment_size ]; then
#	    SIZE=$max_segment_size
#fi

SIZE=$max_segment_size
printf "$SIZE\n"

##load modules & create executable for the algorithm
module purge
module load openMPI/4.1.6/gnu
mpicc -O3 hybrid_qsort.c -o qsort
touch steps.log

# ##perform strong scalability test
printf "Test\tProcesses\tSize\t\tTotal\t\tCommunication\tSorting\t\tMerging\n" > "Weak.txt"

for R in $(seq 1 $REPETITIONS)
do
    sleep 10
    printf "$R\t" >> "Weak.txt"
    echo "$(date):" >> output.log
    free >> output.log
    mpirun -np $NUMPROCS ./qsort $((NUMPROCS*SIZE)) >> "Weak.txt"
done

mv Weak.txt results/
mv *log results/
rm qsort
module purge

