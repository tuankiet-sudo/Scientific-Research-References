#!/bin/bash

echo "Start plotting..."

ARGV=("original-data" "$1")
for FILE in ${@:2}; do
    INDEX=$(( $(basename $FILE | awk -F'_' '{print NF}') - 1))
    ALGO=$(basename $FILE | cut -d '_' -f 3-$INDEX)
    ARGV+=("$ALGO" "$FILE")
done

python3 src/python/plot.py "${ARGV[@]}"

exit 0