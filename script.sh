#!/bin/bash

set -e  # Exit on error

echo "[INFO] Waiting for camera device..."

for i in {1..30}; do
    for dev in /dev/video0 /dev/video1; do
        if [ -e "$dev" ]; then
            CAMERA="$dev"
            echo "[INFO] Found camera at $CAMERA"
            break 2  # Exit both loops
        fi
    done
    sleep 1
done

if [ -z "$CAMERA" ]; then
    echo "[ERROR] No camera device found after 30 seconds." >&2
    exit 1
fi

# Optionally export the camera path if capture uses it
export CAMERA

echo "[INFO] Starting capture program..."
./capture

