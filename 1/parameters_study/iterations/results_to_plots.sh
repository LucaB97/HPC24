#usr/bin/bash

folder=$1
repetitions=10

c++ createCSV.cpp -o createCSV
mv createCSV $folder/
cp plot_generator.py $folder/
cd $folder/

for X in $(seq 1 $repetitions)
    do
        ./createCSV "test__$X.txt" "test__$X.csv" 
        python3 plot_generator.py $X
        rm "test__$X.csv"
    done

rm createCSV
rm plot_generator.py
mkdir plots
mv *png plots/
