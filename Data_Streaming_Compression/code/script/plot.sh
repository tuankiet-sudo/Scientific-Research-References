#!/bin/bash

WORK_DIR="$(pwd)"
CONFIG="$WORK_DIR/config/plot.json"

python3 "$WORK_DIR/plot.py" "$CONFIG"