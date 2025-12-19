# PSP Audio Demo

An interactive audio example for PSP using SDL2_mixer, showcasing sound effects and music playback.

## Features

- SDL2, SDL2_ttf, and SDL2_mixer integration
- Procedurally generated sound effects and music
- Interactive audio playback controls
- Volume control
- Real-time audio status display
- Automated build scripts with PSP toolchain detection

## What the demo does

This example demonstrates PSP audio capabilities by:
- Generating three different sine wave tones (440 Hz, 880 Hz, and 523 Hz) at runtime
- Playing sound effects on button press
- Playing/pausing/stopping background music
- Adjusting volume with shoulder buttons
- Displaying current audio status on screen

## Controls

- **X Button** - Play Beep 1 (440 Hz - A4 note)
- **O Button** - Play Beep 2 (880 Hz - A5 note)
- **□ Button** - Play/Pause Background Music
- **△ Button** - Stop Background Music
- **L Trigger** - Decrease Volume
- **R Trigger** - Increase Volume
- **START** - Exit Demo

## Prerequisites

- [pspdev SDK](https://pspdev.github.io/installation.html) installed
- `PSPDEV` environment variable set (e.g., `export PSPDEV=/home/username/pspdev`)
- PSP console with custom firmware (for device testing)

## Quick Start

```bash
# Navigate to the audio example directory
cd examples/audio

# Build and deploy
./dist.sh
```

The script automatically:
- Detects and configures the PSP toolchain
- Builds the project
- Copies files to `audio/` directory
- Deploys to your PSP (if connected)

## Build Scripts

### `./dist.sh` - Development Build & Deploy
Builds the project and deploys to PSP if connected via USB in USB mode.

**Custom PSP mount point:**
```bash
PSP_MOUNT=/media/mydisk ./dist.sh
```

Default mount point: `/media/$(whoami)/disk`

### `./release.sh` - Release Build Only
Builds the project without deploying to PSP. Output is saved to `audio/` directory.

```bash
./release.sh
```

Perfect for CI/CD pipelines or when you don't have your PSP connected.

## Output Files

After building, you'll find in `audio/`:
- `EBOOT.PBP` - PSP executable
- `Orbitron-Regular.ttf` - Font file

The audio files (`beep.wav`, `beep2.wav`, `music.wav`) are generated at runtime.

## Technical Details

### Audio Generation
The demo uses a sine wave generator to create three different tones:
- **Beep 1**: 440 Hz (A4 note) - 200ms duration
- **Beep 2**: 880 Hz (A5 note) - 150ms duration
- **Music**: 523 Hz (C5 note) - 1000ms duration (loops)

All sounds include fade-in/fade-out envelopes to prevent audio clicking.

### Audio Format
- Sample Rate: 22050 Hz
- Format: 16-bit signed PCM
- Channels: Mono (sound effects) / Stereo output (mixer)

### Libraries Used
- **SDL2**: Core functionality and window management
- **SDL2_ttf**: Font rendering
- **SDL2_mixer**: Audio playback and mixing

## Troubleshooting

### "PSPDEV environment variable is not set"
Make sure you've installed the PSP toolchain and set the environment variable:
```bash
export PSPDEV=/path/to/your/pspdev
```

Add it to your `~/.bashrc` or `~/.zshrc` to make it permanent.

### "PSP device not mounted"
Ensure your PSP is:
1. Connected via USB
2. In USB mode (Settings → USB Connection)
3. Mounted at `/media/$(whoami)/disk` or set `PSP_MOUNT` variable

### SDL2_mixer dependency errors
The scripts automatically configure with the PSP toolchain. If you see SDL2_mixer errors:
```bash
rm -rf build
./dist.sh
```

Make sure SDL2_mixer is installed in your PSP toolchain.

## Customization

- Edit `main.c` to:
  - Change audio frequencies and durations
  - Add more sound effects
  - Load external audio files instead of generating them
  - Implement more complex audio interactions
- Replace `Orbitron-Regular.ttf` with your own font
- Modify `CMakeLists.txt` to add more source files or libraries

## Learning More

This example is a great starting point for:
- Game sound effects
- Menu navigation sounds
- Background music systems
- Audio-based games (rhythm games, audio puzzles, etc.)

For more complex audio needs, explore:
- Loading MP3/OGG files with SDL2_mixer
- Multiple simultaneous sound channels
- Audio effects and filters
- Dynamic audio generation based on game events
