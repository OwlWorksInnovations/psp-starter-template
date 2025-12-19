# 3D Spinning Cube Demo for PSP

A demonstration of 3D graphics on the PlayStation Portable, featuring a wireframe cube rotating in 3D space.

**Created by Claude Code (Anthropic)**

## Features

- Real-time 3D wireframe rendering
- Smooth multi-axis rotation (X, Y, and Z)
- Perspective projection
- 60 FPS animation
- SDL2-based rendering

## What it does

Displays a rotating 3D wireframe cube with vertex highlighting. The cube continuously spins on all three axes, demonstrating basic 3D math including:
- 3D rotation matrices
- Perspective projection
- Wireframe edge rendering

Press START button to exit.

## Building

### Development Build & Deploy
```bash
./dist.sh
```

### Release Build Only
```bash
./release.sh
```

## Technical Details

This demo implements:
- 3D vector rotation using rotation matrices
- Perspective projection to convert 3D coordinates to 2D screen space
- Real-time animation loop with 16ms frame time (~60 FPS)
- Wireframe rendering with highlighted vertices

The cube is defined by 8 vertices and 12 edges, rotating continuously on all three axes at different speeds to create an interesting visual effect.

## Requirements

- PSP with custom firmware
- pspdev SDK installed
- PSPDEV environment variable set

## License

See LICENSE file in project root.
