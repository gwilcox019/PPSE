#!/bin/bash

# USAGE :
# ./run_sims.sh <f-parameter> <s-parameter>
# This script will automatically run the 5 sims in background at the same time with the inputted parameters
# Quicker to execute and super fun !

# gcc simulator.c -o simulator.x -Wall -std=c99 -I/usr/include/gsl -lgsl -lgslcblas -lm -O3 -march=native -DENABLE_STATS

# Check usage
# if [ "$#" -ne 2 ]; then
#     echo "Usage: $0 <f> <s>"
#     exit 1
# fi

~/Documents/PPSE/Modulateur/simulator -m 0 -M 14 -s 1 -e 100 -K 32 -N 128 -D "rep-hard" --demod-neon -f "hard" & 
~/Documents/PPSE/Modulateur/simulator -m 0 -M 11 -s 1 -e 100 -K 32 -N 128 -D "rep-soft" --demod-neon -f "soft_128"  &
~/Documents/PPSE/Modulateur/simulator -m 0 -M 11 -s 1 -e 100 -K 32 -N 96  -D "rep-soft" --demod-neon -f "soft_96"  &
~/Documents/PPSE/Modulateur/simulator -m 0 -M 11 -s 1 -e 100 -K 32 -N 64  -D "rep-soft" --demod-neon -f "soft_64"  &
~/Documents/PPSE/Modulateur/simulator -m 0 -M 11 -s 1 -e 100 -K 32 -N 32  -D "rep-soft" --demod-neon -f "soft_32"  &
