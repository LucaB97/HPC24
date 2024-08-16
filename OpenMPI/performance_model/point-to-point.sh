#usr/bin/bash
CPUS=$1
MESSAGE_SIZE=$2

##create array with IDs of receivers (sender is 0)
k=1
i=0
while [[ $k -le $((CPUS-1)) ]] ; do
receivers[$i]=$k
k=$((k+1))
i=$((i+1))
done

cd ../operations/
module purge
module load openMPI/4.1.6/gnu

printf "" > "latencies_$MESSAGE_SIZE.txt"

for R in "${receivers[@]}"
do
    printf "Message to core: $R\n"
    printf "Message to core: $R\n" >> "latencies_$MESSAGE_SIZE.txt"
    mpirun -np 2 --cpu-list 0,$R osu_latency -m $MESSAGE_SIZE:$MESSAGE_SIZE > "temp.txt"
    cat "temp.txt" >> "latencies_$MESSAGE_SIZE.txt"
done

rm temp.txt
mv *txt ../performance_model/
