#!/bin/bash

echo "Start compiling..."

WORK_DIR="$(pwd)"
rm -rf "$WORK_DIR"/bin/*

mkdir -p "$WORK_DIR/bin/obj/"
cd "$WORK_DIR/bin/obj/"
for file in $(find "$WORK_DIR/src/c++/" -type f); do
    g++ -I "$WORK_DIR/include" -I "$WORK_DIR/lib" --std=c++11 -c "$file"
done

cd "$WORK_DIR"/bin/
g++ -I "$WORK_DIR/include" -I "$WORK_DIR/lib" --std=c++11 -c "$WORK_DIR/src/c++/compress.cpp"
g++ -I "$WORK_DIR/include" -I "$WORK_DIR/lib" --std=c++11 -c "$WORK_DIR/src/c++/decompress.cpp"

rm -f "$WORK_DIR"/bin/obj/compress.o "$WORK_DIR"/bin/obj/decompress.o 
g++ "$WORK_DIR"/bin/obj/*.o "$WORK_DIR"/bin/compress.o -o compress
g++ "$WORK_DIR"/bin/obj/*.o "$WORK_DIR"/bin/decompress.o -o decompress

exit 0