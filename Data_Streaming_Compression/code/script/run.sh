#!/bin/bash

FLAG=$(python3 src/python/verify.py "${@:4}")

if [[ $FLAG == 1 ]]; then
    echo "Algorithm $4 is not supported."
    exit 1
elif [[ $FLAG == 2 ]]; then
    echo "Missing required parameter for $4 algorithm."
    exit 2
fi

mkdir -p data/output/compress
mkdir -p data/output/decompress
mkdir -p data/monitor/compress
mkdir -p data/monitor/decompress

ALGO=$4
FILE=$(basename $1 .csv)
ID=$(uuidgen | cut -c 1-8)

# Compressing phase
echo "-------------------------"
echo "Compressing file: $ALGO-$FILE-$ID.bin"
echo "Compressing profile: $ALGO-$FILE-$ID.csv"
touch data/output/compress/$ALGO-$FILE-$ID.bin data/monitor/compress/$ALGO-$FILE-$ID.csv

echo "Start compressing..."
bin/compress $1 "data/output/compress/$ALGO-$FILE-$ID.bin" "data/monitor/compress/$ALGO-$FILE-$ID.csv" $2 "${@:4}"


# Decompressing phase
echo -e "\n-------------------------"
echo "Decompressing file: $ALGO-$FILE-$ID.csv"
echo "Decompressing profile: $ALGO-$FILE-$ID.csv"
touch data/output/decompress/$ALGO-$FILE-$ID.csv data/monitor/decompress/$ALGO-$FILE-$ID.csv

echo "Start decompressing..."
bin/decompress "data/output/compress/$ALGO-$FILE-$ID.bin" "data/output/decompress/$ALGO-$FILE-$ID.csv" "data/monitor/decompress/$ALGO-$FILE-$ID.csv" $3 $4

# Statistic phase
echo -e "\n-------------------------"
echo "Start statisticizing..."
python3 "src/python/statistics.py" $1 "data/output/decompress/$ALGO-$FILE-$ID.csv" "data/output/compress/$ALGO-$FILE-$ID.bin"

exit 0