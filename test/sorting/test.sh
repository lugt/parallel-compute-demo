#!/usr/bin/env bash
set -x;
g++ -fopenmp -O3 wanshu.cxx -o wanshu.out -lomp -lm

time ./wanshu.out 1 
time ./wanshu.out 2
time ./wanshu.out 4
time ./wanshu.out 8
time ./wanshu.out 16
time ./wanshu.out 32

