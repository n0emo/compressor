#!/bin/sh

CC='gcc -std=c23 -g -Wall -Wextra -I include'

mkdir -p build

set -x
set -e

$CC -c src/compressor.c -o build/compressor.o
$CC -c src/plugin.c -o build/plugin.o

$CC build/*.o \
    -lpthread \
    -shared -o ./build/compressor.clap
