#usr/bin/bash

folder=$1
repetitions=10

c++ createCSV.cpp -o createCSV
mv createCSV iterations/tests/$folder
cp plot_generator.py iterations/tests/$folder
cd iterations/tests/$folder
mkdir plots

# printf "Operation,Algorithm,Mapping,Iterations,Warmup,Size,Avg Latency(us),Min Latency(us),Max Latency(us)\n" > "results.csv"

for X in $(seq 1 $repetitions)
    do
        ./createCSV "test__$X.txt" "test__$X.csv" 
        # cat "test__$X.csv" >> "results.csv"
        python3 plot_generator.py $X
        rm "test__$X.csv"
    done

mv *png plots/
rm createCSV
rm plot_generator.py