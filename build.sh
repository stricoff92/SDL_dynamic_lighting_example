
#!/bin/bash

mkdir -p dist

OLEVEL="0"
CFLAGS="-fstrict-aliasing -Wall -Wextra -Wfloat-equal -Wno-unused-variable -pedantic -Wno-unused-parameter -g -O$OLEVEL"

printf "building... "
gcc $CFLAGS -o dist/app src/app.c -lSDL2 -lm

printf "done!\n"
