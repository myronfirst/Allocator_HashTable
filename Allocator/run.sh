#!/bin/bash

THREADS=8

make
./main $THREADS
wait
