#!/bin/bash

FLAG=$(python3 src/python/verify.py "${@:4}")

if [[ $FLAG == 1 ]]; then
    echo "Algorithm $4 is not supported."
    exit 1
elif [[ $FLAG == 2 ]]; then
    echo "Missing required parameter for $4 algorithm."
    exit 2
elif [[ $FLAG == 3 ]]; then
    echo "Invalid parameter for $4 algorithm."
    exit 3
fi

mkdir -p out/compress
mkdir -p out/decompress

ALGO=$4
DATA=$(echo $1 | rev | cut -d '/' -f 2 | rev)
FILE=$(basename $1 .csv)
ID=$(uuidgen | cut -c 1-8)

# Compressing phase
echo "-------------------------"
echo "Compressing file: $DATA"_"$FILE"_"$ALGO"_"$ID.bin"
echo "Compressing profile: $DATA"_"$FILE"_"$ALGO"_"$ID.mon"
touch out/compress/$DATA"_"$FILE"_"$ALGO"_"$ID.bin out/compress/$DATA"_"$FILE"_"$ALGO"_"$ID.mon

echo "Start compressing..."
bin/compress $1 out/compress/$DATA"_"$FILE"_"$ALGO"_"$ID.bin out/compress/$DATA"_"$FILE"_"$ALGO"_"$ID.mon $2 "${@:4}"


# Decompressing phase
echo -e "\n-------------------------"
echo "Decompressing file: $DATA"_"$FILE"_"$ALGO"_"$ID.csv"
echo "Decompressing profile: $DATA"_"$FILE"_"$ALGO"_"$ID.mon"
touch out/decompress/$DATA"_"$FILE"_"$ALGO"_"$ID.csv out/decompress/$DATA"_"$FILE"_"$ALGO"_"$ID.mon

echo "Start decompressing..."
bin/decompress out/compress/$DATA"_"$FILE"_"$ALGO"_"$ID.bin out/decompress/$DATA"_"$FILE"_"$ALGO"_"$ID.csv out/decompress/$DATA"_"$FILE"_"$ALGO"_"$ID.mon $3 $4 $2

# # Statistic phase
# echo -e "\n-------------------------"
# echo "Start statisticizing..."
# python3 src/python/statistics.py $1 out/decompress/$DATA"_"$FILE"_"$ALGO"_"$ID.csv out/compress/$DATA"_"$FILE"_"$ALGO"_"$ID.mon out/decompress/$DATA"_"$FILE"_"$ALGO"_"$ID.mon out/compress/$DATA"_"$FILE"_"$ALGO"_"$ID.bin

# exit 0