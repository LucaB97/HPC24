#usr/bin/bash

node=$1
operation=$2
algorithms=("${@:3}")

c++ createCSV.cpp -o createCSV
mv createCSV ../$node/$operation/
cp plot_generator.py ../$node/$operation/
cd ../$node/$operation/
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