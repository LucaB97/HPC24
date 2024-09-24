#usr/bin/bash

operation=$1
algorithms=("${@:2}")

c++ createCSV.cpp -o createCSV
mv createCSV results/$operation/
cp plot_generator.py results/$operation/
cd results/$operation/
mkdir plots/

for A in "${algorithms[@]}"
    do
        ./createCSV "alg__$A.txt" "alg__$A.csv" 
        python3 plot_generator.py "alg__$A"
        rm "alg__$A.csv"
    done

mv *png plots/
rm createCSV
rm plot_generator.py