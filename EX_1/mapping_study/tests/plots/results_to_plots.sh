#usr/bin/bash

folder=$1
algorithms=(0 2 5 6 7)

c++ createCSV.cpp -o createCSV
mv createCSV ../$folder
cp plot_generator2.py ../$folder
cd ../$folder

for X in "${algorithms[@]}"
    do
        ./createCSV "bcast__$X.txt" "bcast__$X.csv" 
        python3 plot_generator2.py "bcast__$X"
        rm "bcast__$X.csv"
    done

mv *png ../plots/
rm createCSV
rm plot_generator2.py