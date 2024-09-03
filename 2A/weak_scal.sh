#!/bin/bash

NODES=$1
SIZE=$2
REPETITIONS=$3
NUMPROCS=64
#NUMPROCS=(1 2 4 8 16 32 64 128 256)
# Start monitoring memory usage in the background and log to a file

( while true; do echo "$(date +'%a %b %e %H:%M:%S.%6N %Z %Y'):" >> output.log; free -m >> output.log; sleep 0.0000001; done ) &
# ( while true; do echo "$(date):" >> output.log; free -m >> output.log; sleep 0.0000001; done ) &
# ( while true; do free -m >> output.log; sleep 1; done ) &

srun --nodes=$NODES --ntasks-per-node=1 free > "memory"
free_memory=$(grep Mem memory | awk '{print $4}' | sort -n | head -n 1)  #in kibibytes (KiB), where 1 KiB = 1024 bytes
gcc -o memory_req memory_req.c
data_size=$(./memory_req)  # Size of each data entry in bytes
max_segment_size=$(echo "scale=0; 0.5 * 1024 * $free_memory / ($data_size * $NUMPROCS)" | bc)
printf "max segment size: $max_segment_size\n"

if [ $SIZE -ge $max_segment_size ]; then
    SIZE=$max_segment_size
fi

##load modules & create executable for the algorithm
module purge
module load openMPI/4.1.6/gnu
mpicc -O3 hybrid_qsort.c -o qsort

##perform strong scalability test
printf "Test\tProcesses\tSize\t\tTotal\t\tCommunication\tSorting\t\tMerging\n" > "Weak.txt"

for P in "${NUMPROCS[@]}"
do
    for R in $(seq 1 $REPETITIONS)
    do
        printf "$R\t" >> "Weak.txt"
        mpirun -np $P ./qsort $((P*SIZE)) >> "Weak.txt"
    done
done


python3 filter_output_log.py
mv Weak.txt results/
mv *log results/
module purge
rm qsort
rm memory
rm memory_req

