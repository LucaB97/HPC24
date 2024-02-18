#!/bin/bash

REPETITIONS=$1

for X in $(seq 1 $REPETITIONS)
do
	sbatch iter_test.job $X
done
