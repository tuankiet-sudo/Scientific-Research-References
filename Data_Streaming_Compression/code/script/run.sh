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
echo "Compressing file: $FILE-$ALGO-$ID.bin"
echo "Compressing profile: $FILE-$ALGO-$ID.csv"
touch data/output/compress/$FILE-$ALGO-$ID.bin data/monitor/compress/$FILE-$ALGO-$ID.csv

echo "Start compressing..."
bin/compress $1 "data/output/compress/$FILE-$ALGO-$ID.bin" "data/monitor/compress/$FILE-$ALGO-$ID.csv" $2 "${@:4}"


# Decompressing phase
echo -e "\n-------------------------"
echo "Decompressing file: $FILE-$ALGO-$ID.csv"
echo "Decompressing profile: $FILE-$ALGO-$ID.csv"
touch data/output/decompress/$FILE-$ALGO-$ID.csv data/monitor/decompress/$FILE-$ALGO-$ID.csv

echo "Start decompressing..."
bin/decompress "data/output/compress/$FILE-$ALGO-$ID.bin" "data/output/decompress/$FILE-$ALGO-$ID.csv" "data/monitor/decompress/$FILE-$ALGO-$ID.csv" $3 $4

# Statistic phase
echo -e "\n-------------------------"
echo "Start statisticizing..."
python3 "src/python/statistics.py" $1 "data/output/decompress/$FILE-$ALGO-$ID.csv" "data/output/compress/$FILE-$ALGO-$ID.bin"

exit 0