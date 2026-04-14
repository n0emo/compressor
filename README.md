# n0emo's Compressor

A simple stereo compressor CLAP plugin in C.

## Features

- Threshold, Attack, Release, Ratio controls
- Output gain and Mix (wet/dry)
- RMS level detection
- Smooth parameter interpolation

## Platform support

- MacOS
- Linux

## Building

### Prerequisites

- CMake 3.14+
- C23 compiler (gcc, clang, or MSVC)

### macOS

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
Output: directory `build/compressor.clap/`

### Linux
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Output: single file `build/compressor.clap`

## Installation

macOS: Copy `compressor.clap` to `/Library/Audio/Plug-Ins/CLAP/` or `~/Library/Audio/Plug-Ins/CLAP/`

Linux: Copy `compressor.clap` to `~/.clap/` or `/usr/lib/clap/`
