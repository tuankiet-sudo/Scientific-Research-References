#!/bin/bash

if [[ $1 = "compress" ]]; then
    echo "Start compressing..."
    
    mkdir -p data/output/compress
    INPUT=$(find data/input -name $2*)
    OUTPUT=data/output/compress/$3_$(basename $INPUT .csv)
    touch $OUTPUT

    bin/compress $INPUT $OUTPUT "${@:3}"

elif [[ $1 = "decompress" ]]; then
    echo "Start decompressing..."

    mkdir -p data/output/decompress
    INPUT=$(find data/output/compress -name $2*)
    OUTPUT=data/output/decompress/$(basename $INPUT).csv
    touch $OUTPUT
    
    bin/decompress $INPUT $OUTPUT "${@:3}"
fi

exit 0