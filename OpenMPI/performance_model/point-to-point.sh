#usr/bin/bash
CPUS=$1
MESSAGE_SIZE=$2

cd ../operations/

##create array with IDs of receivers (sender is 0)
k=1
i=0
while [[ $k < $CPUS ]] ; do
receivers[$i]=$k
k=$((k+1))
i=$((i+1))
done

for R in "${receivers[@]}"
do
    printf "Message to core: $R\n" >> "latencies.txt"
    mpirun -np 2 --cpu-list 0,$R osu_latency -m $MESSAGE_SIZE:$MESSAGE_SIZE > "temp.txt"
    cat "temp.txt" >> "latencies.txt"
done

rm temp.txt
mv *txt ../performance_model/
