#!/bin/sh

mkdir -p build

gcc -g -Wall -Wextra -c compressor.c -o build/compressor.o
gcc -g -Wall -Wextra -c plugin.c -o build/plugin.o
gcc -g -Wall -Wextra build/*.o \
    -lpthread \
    -shared -o ./build/compressor.clap
