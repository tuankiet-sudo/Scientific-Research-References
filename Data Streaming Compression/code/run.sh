#!/bin/bash

WORK_DIR="$(pwd)"

rm -rf "$WORK_DIR"/bin/*
g++ -I "$WORK_DIR/include" --std=c++11 -c "$WORK_DIR/main.cpp" -o bin/main.o

cd "$WORK_DIR/bin"
for file in $(ls "$WORK_DIR/src/"); do
    g++ -I "$WORK_DIR/include" --std=c++11 -c "$WORK_DIR/src/$file"
done

g++ "$WORK_DIR"/bin/*.o -o run
"$WORK_DIR/bin/run"

exit 0