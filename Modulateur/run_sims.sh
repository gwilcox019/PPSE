#!/bin/bash

# USAGE :
# ./run_sims.sh <f-parameter> <s-parameter>
# This script will automatically run the 5 sims in background at the same time with the inputted parameters
# Quicker to execute and super fun !
# Results will be stored in the file : "sim_<no>_<f>_<s>.csv"

gcc simulator.c -o simulator.x -Wall -std=c99 -I/usr/include/gsl -lgsl -lgslcblas -lm

# Check usage
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <f> <s>"
    exit 1
fi

# Task1
./simulator.x -m 0 -M 10 -s 1 -e 100 -K 32 -N 128 -D "rep-hard8" -f "1_$1_$2" --qf $1 --qs $2 &

# Task 2
./simulator.x -m 0 -M  9 -s 1 -e 100 -K 32 -N 128 -D "rep-soft8" -f "2_$1_$2" --qf $1 --qs $2 &

# Task 3
./simulator.x -m 0 -M  9 -s 1 -e 100 -K 32 -N  96 -D "rep-soft8" -f "3_$1_$2" --qf $1 --qs $2 &

# Task 4
./simulator.x -m 0 -M  9 -s 1 -e 100 -K 32 -N  96 -D "rep-soft8" -f "4_$1_$2" --qf $1 --qs $2 &

# Task 5
./simulator.x -m 0 -M  9 -s 1 -e 100 -K 32 -N  96 -D "rep-soft8" -f "5_$1_$2" --qf $1 --qs $2 &