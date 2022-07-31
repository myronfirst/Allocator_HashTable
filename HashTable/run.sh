#!/bin/bash

SERVER_THREADS=8
COMMANDS_NUMBER=10240

make
rm /dev/shm/*
(./server $SERVER_THREADS) & (./client $COMMANDS_NUMBER) && fg
wait
