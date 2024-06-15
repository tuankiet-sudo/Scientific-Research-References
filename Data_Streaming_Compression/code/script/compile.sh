#!/bin/bash

WORK_DIR="$(pwd)"
rm -rf "$WORK_DIR"/bin/*

cd "$WORK_DIR/bin"
for file in $(find "$WORK_DIR/src/" -type f); do
    g++ -I "$WORK_DIR/include" -I "$WORK_DIR/lib" --std=c++11 -c "$file"
done
g++ -I "$WORK_DIR/include" -I "$WORK_DIR/lib" --std=c++11 -c "$WORK_DIR/main.cpp"

g++ "$WORK_DIR"/bin/*.o -o _exe
exit 0