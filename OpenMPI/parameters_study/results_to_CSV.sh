#usr/bin/bash

folder=$1
repetitions=10

c++ createCSV.cpp -o createCSV
mv createCSV iterations/tests/$folder
cd iterations/tests/$folder

printf "Operation,Algorithm,Mapping,Iterations,Warmup,Size,Avg Latency(us),Min Latency(us),Max Latency(us)\n" > "results.csv"

for X in $(seq 1 $repetitions)
    do
        ./createCSV "test__$X.txt" "test__$X.csv" 
        cat "test__$X.csv" >> "results.csv"
        rm "test__$X.csv"
    done

rm createCSV