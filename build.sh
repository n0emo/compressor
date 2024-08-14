#!/bin/sh

CC='gcc -std=c23 -g -Wall -Wextra'

mkdir -p build

set -x
set -e

$CC -c compressor.c -o build/compressor.o
$CC -c plugin.c -o build/plugin.o

$CC build/*.o \
    -lpthread \
    -shared -o ./build/compressor.clap
