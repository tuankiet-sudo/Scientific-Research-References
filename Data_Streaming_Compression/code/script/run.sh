#!/bin/bash

if [[ $1 = "compress" ]]; then
    echo "Start compressing..."
    bin/compress "${@:2}"

elif [[ $1 = "decompress" ]]; then
    echo "Start decompressing..."
    bin/decompress "${@:2}"
fi

exit 0