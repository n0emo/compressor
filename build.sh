#!/bin/sh

CC='clang -std=c23 -g -Wall -Wextra -I include -fPIC'

mkdir -p build

set -x
set -e

$CC -c src/compressor.c -o build/compressor.o
$CC -c src/plugin.c -o build/plugin.o
$CC -c src/params.c -o build/params.o
$CC -c src/util.c -o build/util.o

$CC build/*.o \
    -lpthread \
    -shared -o ./build/compressor.clap
