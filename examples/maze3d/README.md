# 3D Maze Game for PSP

A first-person 3D maze exploration game using raycasting, featuring procedurally generated mazes across 3 levels of increasing difficulty.

**Created by Claude Code (Anthropic)**

## Features

- Wolfenstein 3D-style raycasting engine
- Procedurally generated mazes using recursive backtracking
- Textured walls with brick pattern
- 3 levels with increasing maze size
- FPS-style movement with strafing
- Background music and sound effects
- Start menu and pause menu
- Distance-based shading for depth perception

## Gameplay

Navigate through 3D mazes to find the green exit. Each level features a randomly generated maze that gets progressively larger and more challenging.

### Controls

| Button | Action |
|--------|--------|
| D-pad Up / Analog Up | Move forward |
| D-pad Down / Analog Down | Move backward |
| D-pad Left | Turn left |
| D-pad Right | Turn right |
| L Trigger | Strafe left |
| R Trigger | Strafe right |
| Start | Pause game |
| X (Cross) | Confirm selection |

### Levels

- **Level 1**: 5x5 maze (small, for learning)
- **Level 2**: 8x8 maze (medium difficulty)
- **Level 3**: 12x10 maze (large, challenging)

## Technical Details

### Raycasting Engine

The game uses a DDA (Digital Differential Analyzer) raycasting algorithm to render 3D walls from a 2D map, similar to classic games like Wolfenstein 3D.

- Screen resolution: 480x272 (PSP native)
- Field of view: 60 degrees
- Texture resolution: 64x64 pixels
- Pre-calculated sin/cos lookup tables for performance

### Maze Generation

Mazes are generated using the recursive backtracking algorithm, which creates "perfect" mazes with exactly one path between any two points.

### Audio

All audio is procedurally generated at runtime:

- Background music: Simple melody loop
- Menu selection sound: Short beep
- Win sound: Ascending tone sequence

## Prerequisites

- [pspdev SDK](https://pspdev.github.io/installation.html) installed
- `PSPDEV` environment variable set
- PSP console with custom firmware (for device testing)

## Quick Start

```bash
# Navigate to the maze3d example directory
cd examples/maze3d

# Build and deploy
./dist.sh
```

## Build Scripts

### `./dist.sh` - Development Build & Deploy

Builds the project and deploys to PSP if connected via USB.

```bash
PSP_MOUNT=/media/mydisk ./dist.sh  # Custom mount point
```

### `./release.sh` - Release Build Only

Builds without deploying:

```bash
./release.sh
```

## Output Files

After building, you'll find in `maze3d/`:

- `EBOOT.PBP` - PSP executable
- `Orbitron-Regular.ttf` - Font file

Audio files are generated at runtime.

## Troubleshooting

### Performance Issues

The raycasting engine is optimized for PSP but may still run slower on complex mazes. The game uses:

- Lookup tables for trigonometry
- Maximum render distance limit
- Efficient DDA algorithm

### "PSPDEV environment variable is not set"

```bash
export PSPDEV=/path/to/your/pspdev
```

### Font not loading

Ensure `Orbitron-Regular.ttf` is in the same directory as `EBOOT.PBP`.

## How It Works

1. **Maze Generation**: Creates a logical maze using recursive backtracking, then converts it to a wall grid
2. **Raycasting**: For each vertical screen column, casts a ray to find the nearest wall and calculates its height
3. **Texturing**: Maps brick texture to walls based on hit position, with exit walls using green checkered pattern
4. **Shading**: Applies distance-based darkening and side-based shading for depth perception
5. **Collision**: Uses circle-box collision detection with wall sliding

## Learning More

This example demonstrates:

- Software 3D rendering techniques
- Procedural content generation
- Game state management
- PSP-specific optimizations

For more details on raycasting, see [Lode's Computer Graphics Tutorial](https://lodev.org/cgtutor/raycasting.html).
