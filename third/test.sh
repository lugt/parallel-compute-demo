#!/usr/bin/env bash
set -x;
# g++ -fopenmp -O0 wanshu.cxx -o wanshu.out -lomp -lm

time ./wanshu.out 1 100000000
time ./wanshu.out 2 100000000
time ./wanshu.out 4 100000000
time ./wanshu.out 8 100000000
time ./wanshu.out 16 100000000
time ./wanshu.out 32 100000000

