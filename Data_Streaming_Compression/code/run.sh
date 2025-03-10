#!/bin/bash


WORK_DIR="$(pwd)"

g++ -I "$WORK_DIR/lib" --std=c++11 main.cpp -o exe
./exe
rm -rf exe

exit 0