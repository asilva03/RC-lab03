#!/usr/bin/env bash
gcc -c linklayer.c -o ../protocol/linklayer.o
gcc -w main.c ../protocol/*.o -o main
