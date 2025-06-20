# SDL Dynamic Lighting Example


An test program that emulates dynamic lighting using SDL2

<img src="images/sdl.png" width="300">

## Installation

```bash
# debian
apt install libsdl2-dev
```

## Building

```bash
mkdir -p dist && gcc -o dist/app src/app.c -lSDL2 -lm

# or use build script

./build.sh

```

## running
```bash
./dist/app

# test with vsync
USE_VSYNC=1 ./dist/app
```

## Seizure Warning

This program displays flashing lights on the screen.

