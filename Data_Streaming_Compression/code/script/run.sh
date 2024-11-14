#!/bin/bash

if [[ $1 = "compress" ]]; then
    echo "Start compressing..."
    
    mkdir -p data/output/compress
    mkdir -p data/monitor/compress

    OUTPUT=data/output/compress/$3_$(basename $2 .csv)
    MONITOR=data/monitor/compress/$3_$(basename $2 .csv).csv
    touch $OUTPUT $MONITOR

    bin/compress $2 $OUTPUT $MONITOR "${@:3}"

elif [[ $1 = "decompress" ]]; then
    echo "Start decompressing..."

    mkdir -p data/output/decompress
    mkdir -p data/monitor/decompress

    OUTPUT=data/output/decompress/$(basename $2).csv
    MONITOR=data/monitor/decompress/$(basename $2).csv
    touch $OUTPUT $MONITOR
    
    bin/decompress $2 $OUTPUT $MONITOR "${@:3}"
fi

exit 0