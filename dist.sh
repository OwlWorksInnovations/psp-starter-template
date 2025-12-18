#!/bin/bash
set -euo pipefail

# Get script directory (project root)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
OUTPUT_DIR="${SCRIPT_DIR}/helloworld"

# PSP mount point (override with PSP_MOUNT environment variable)
PSP_MOUNT="${PSP_MOUNT:-/media/$(whoami)/disk}"
PSP_GAME_DIR="${PSP_MOUNT}/PSP/GAME/helloworld"

echo "Building project..."
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found. Run cmake first."
    exit 1
fi

cd "$BUILD_DIR"
make clean && make

echo "Copying files to output directory..."
mkdir -p "$OUTPUT_DIR"
cp "$BUILD_DIR/EBOOT.PBP" "$OUTPUT_DIR/"

# Copy font from project root directory
if [ -f "$SCRIPT_DIR/Orbitron-Regular.ttf" ]; then
    cp "$SCRIPT_DIR/Orbitron-Regular.ttf" "$OUTPUT_DIR/"
else
    echo "Warning: Font file not found at $SCRIPT_DIR/Orbitron-Regular.ttf"
fi

echo "Deploying to PSP..."
if [ -d "${PSP_MOUNT}/PSP/GAME" ]; then
    mkdir -p "$PSP_GAME_DIR"
    cp -r "$OUTPUT_DIR"/* "$PSP_GAME_DIR/"
    echo "Successfully deployed to PSP at $PSP_GAME_DIR"
else
    echo "Warning: PSP device not mounted at $PSP_MOUNT"
    echo "Skipping PSP deployment."
    echo "Tip: Set PSP_MOUNT environment variable if your PSP is mounted elsewhere"
    echo "     Example: PSP_MOUNT=/media/mydisk ./dist.sh"
fi

echo "Build and distribution complete!"

