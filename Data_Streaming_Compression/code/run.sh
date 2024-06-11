#!/bin/bash

WORK_DIR="$(pwd)"

rm -rf "$WORK_DIR"/bin/*
g++ -I "$WORK_DIR/include" --std=c++11 -c "$WORK_DIR/main.cpp" -o bin/main.o

cd "$WORK_DIR/bin"
for file in $(find "$WORK_DIR/src/" -type f); do
    g++ -I "$WORK_DIR/include" --std=c++11 -c "$file"
done

g++ "$WORK_DIR"/bin/*.o -o _exe
"$WORK_DIR/bin/_exe"

exit 0