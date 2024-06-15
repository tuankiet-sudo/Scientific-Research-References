#!/bin/bash

WORK_DIR="$(pwd)"
PLOT_CONFIG="$WORK_DIR/config/plot.json"
ALGO_CONFIG="$WORK_DIR/config/algo.json"

### Parsing json config
INPUT=$(python3 python/parse_algo.py $ALGO_CONFIG input)
OUTPUT=$(python3 python/parse_algo.py $ALGO_CONFIG output)
METHODS=$(python3 python/parse_algo.py $ALGO_CONFIG methods)

$WORK_DIR/bin/_exe $INPUT $OUTPUT $METHODS

### Plotting all results
#python3 "$WORK_DIR/plot.py" "$CONFIG"