#!/bin/bash

WORK_DIR="$(pwd)"
PLOT_CONFIG="$WORK_DIR/config/plot.json"
ALGO_CONFIG="$WORK_DIR/config/algo.json"

### Parsing json config
TEMP=$(python3 $WORK_DIR/python/parse_algo.py $ALGO_CONFIG)
$WORK_DIR/bin/_exe $WORK_DIR/$TEMP
rm -f $WORK_DIR/$TEMP

### Plotting all results
python3 "$WORK_DIR/python/plot.py" "$PLOT_CONFIG"
exit 0