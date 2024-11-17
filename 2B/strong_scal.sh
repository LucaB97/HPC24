#!/bin/bash

NODES=$1
SIZE=$2
NUMPROCS=("${@:3}")

srun --nodes=$NODES --ntasks-per-node=1 free > "memory"
free_memory=$(grep Mem memory | awk '{print $4}' | sort -n | head -n 1)  #in kibibytes (KiB), where 1 KiB = 1024 bytes

# Evaluate the size of each data entry in bytes
gcc -o data_unit_memory data_unit_memory.c
data_size=$(./data_unit_memory)

# Find the highest number of CPUs requested
P=$(printf "%s\n" "${NUMPROCS[@]}" | sort -n | tail -n 1)

# Evaluate the total memory requirements for the mydata and receive_buffer arrays 
log2_P=$(echo "scale=10; l($P)/l(2)" | bc -l)
memory_req=$(echo "scale=10; ($log2_P + 2)/2 + ($log2_P + 1)/4" | bc -l)

# EVALUATE MAX PROBLEM SIZE
MAX_SIZE=$(echo "scale=0; 0.70 * 1024 * $free_memory / ($memory_req * $data_size)" | bc)
printf "max size: $MAX_SIZE\n"

# If the user specified size is too big, replace it with the maximum
if [ $SIZE -ge $MAX_SIZE ]; then
    SIZE=$MAX_SIZE
fi

##load modules & create executable for the algorithm
module purge
module load openMPI/4.1.6/gnu
mpicc -O3 qsort.c -o qsort

##perform strong scalability test
printf "Test\tProcesses\tSize\t\tTotal\t\tCommunication\tSorting\t\tMerging\n" > "strong.txt"

for P in "${NUMPROCS[@]}"
do
    mpirun -np $P ./qsort $SIZE >> "strong.txt"
done

module purge
mv strong.txt results/
mv steps.log results/strong_mem_usage.log
rm qsort
rm memory
rm data_unit_memory
