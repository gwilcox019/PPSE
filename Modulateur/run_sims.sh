#!/bin/bash

# USAGE :
# ./run_sims.sh <f-parameter> <s-parameter>
# This script will automatically run the 5 sims in background at the same time with the inputted parameters
# Quicker to execute and super fun !

gcc simulator.c -o simulator.x -Wall -std=c99 -I/usr/include/gsl -lgsl -lgslcblas -lm -O3 -march=native

# Check usage
# if [ "$#" -ne 2 ]; then
#     echo "Usage: $0 <f> <s>"
#     exit 1
# fi

# Hard
./simulator.x -m 0 -M 14 -s 1 -e 100 -K 32 -N 8192 -D "rep-hard" -f "hard" &
./simulator.x -m 0 -M 14 -s 1 -e 100 -K 32 -N 8192 -D "rep-hard8" -f "hard8" --qf 5 --qs 7 &
./simulator.x -m 0 -M 14 -s 1 -e 100 -K 32 -N 8192 -D "rep-hard8-neon" -f "hard_neon" --qf 5 --qs 7 &

./simulator.x -m 0 -M 11 -s 1 -e 100 --src-all-zeros --mod-all-ones -K 32 -N 8192 -D "rep-soft" -f "soft" &
./simulator.x -m 0 -M 11 -s 1 -e 100 --src-all-zeros --mod-all-ones -K 32 -N 8192 -D "rep-soft8" -f "soft8" --qf 3 --qs 5 &
./simulator.x -m 0 -M 11 -s 1 -e 100 --src-all-zeros --mod-all-ones -K 32 -N 8192 -D "rep-soft8-neon" -f "soft_neon" --qf 3 --qs 5 & 
