#!/bin/bash

SECONDS=0
START_TIME="$(date)"

printf '0\n../data/random.bin\n1\n0\n100\n1\n' | ./assess 1000000

DURATION=$SECONDS
STOP_TIME="$(date)"

echo "       Start: $START_TIME"
echo "        Stop: $STOP_TIME"
echo "Elapsed time: $DURATION seconds"
echo " "
