
#!/bin/bash

mkdir -p dist
mkdir -p build

rm dist/* 2> /dev/null
rm build/* 2> /dev/null

OLEVEL="0"
CFLAGS="-fstrict-aliasing -Wall -Wextra -Wfloat-equal -Wno-unused-variable -pedantic -Wno-unused-parameter -g -O$OLEVEL"
CC="gcc"
OUT_EXECUTABLE="lighting"

for f in src/*.c; do
    froot=$(echo $f | awk -F '/' '{print $2}' | awk -F '.' '{print $1}')
    printf "  building $froot.o ..."
    $CC $CFLAGS -c src/$froot.c -o build/$froot.o -L/usr/local/lib/ -lSDL3 -lm
    printf " done\n"
done

printf "  building binary... "
$CC $CFLAGS build/*.o -o dist/$OUT_EXECUTABLE $LIB_ARGS -L/usr/local/lib/ -lSDL3 -lm

printf "done!\n"
