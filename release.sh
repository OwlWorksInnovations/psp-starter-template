#!/bin/bash
set -euo pipefail

# Get script directory (project root)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
OUTPUT_DIR="${SCRIPT_DIR}/helloworld"

echo "Building release..."
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

echo "Release build complete!"
echo "Output directory: $OUTPUT_DIR"

