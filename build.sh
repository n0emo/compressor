#!/bin/sh

mkdir -p build

clang plugin.c -shared -g -Wall -Wextra -o ./build/compressor.clap
