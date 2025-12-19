# PSP Development Starter Template

A minimal starter template for a PSP using SDL2 with examples

## Features

- SDL2 and SDL2_ttf integration
- Automated build scripts with PSP toolchain detection
- Auto configuration from fresh clone
- Custom font rendering example
- Ready to deploy EBOOT.PBP generation

## What the template does

Displays "Hello PSP!" on the PSP screen using the Orbitron font and has some example games

## Prerequisites

- [pspdev SDK](https://pspdev.github.io/installation.html) installed
- `PSPDEV` environment variable set (e.g., `export PSPDEV=/home/username/pspdev`)
- PSP console with custom firmware (for device testing)

## Quick Start

```bash
# Clone and build in one go
git clone https://github.com/OwlWorksInnovations/helloworld.git
cd helloworld
rm -rf .git
./dist.sh
```

That's it! The script automatically:

- Detects and configures the PSP toolchain
- Builds the project
- Copies files to `helloworld/` directory
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

Builds the project without deploying to PSP. Output is saved to `helloworld/` directory.

```bash
./release.sh
```

Perfect for CI/CD pipelines or when you don't have your PSP connected.

## Output Files

After building, you'll find in `helloworld/`:

- `EBOOT.PBP` - PSP executable
- `Orbitron-Regular.ttf` - Font file

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

### SDL2 dependency errors

The scripts automatically configure with the PSP toolchain. If you see SDL2 errors:

```bash
rm -rf build
./dist.sh
```

## Customization

- Edit `main.c` to create your game/app
- Replace `Orbitron-Regular.ttf` with your own font
- Modify `CMakeLists.txt` to add more source files or libraries

## NOTE

I am currently reading "C Programming - A Modern Approach", this means that I wont be doing work on this project until I am proficient with C **BUT** I will be using claude code to provide examples. I plan on rewriting these examples to be more clear if needed in the future.

### Progress

<details>
  <summary>Progress</summary>
  <ul>
    <li>Chapter 1 : Done</li>
    <li>Chapter 2 : Done</li>
    <li>Chapter 3 : Done</li>
    <li>Chapter 4 : Done</li>
    <li>Chapter 5 : Done</li>
    <li>Chapter 6 : In Progress</li>
    <li>Chapter 7 : Not there yet</li>
    <li>Chapter 8 : Not there yet</li>
    <li>Chapter 9 : Not there yet</li>
    <li>Chapter 10 : Not there yet</li>
    <li>Chapter 11 : Not there yet</li>
    <li>Chapter 12 : Not there yet</li>
    <li>Chapter 13 : Not there yet</li>
    <li>Chapter 14 : Not there yet</li>
    <li>Chapter 15 : Not there yet</li>
    <li>Chapter 16 : Not there yet</li>
    <li>Chapter 17 : Not there yet</li>
    <li>Chapter 18 : Not there yet</li>
    <li>Chapter 19 : Not there yet</li>
    <li>Chapter 20 : Not there yet</li>
    <li>Chapter 21 : Not there yet</li>
  </ul>
</details>

## Disappointments

- 'Maze 3D' I don't know what claude code did but it was horrible even on the opus 4.5 model used all my usage...
- 'cube3d' Almost 99% sure that this does not even use the gpu on the psp even after I told claude code with sonnet 4.5 to use it.

### What I learned

With all of these disappointments I learned.

- PSP uses PSPGL for 3d which uses opengl 1.1 if im correct (another way is with PSPGU but im not sure)
