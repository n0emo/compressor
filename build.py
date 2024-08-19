#!/bin/python3

from functools import reduce
import os
import pathlib
import subprocess

SOURCE_DIR = 'src'
BUILD_DIR = 'build'
INCLUDE_DIR = 'include'

cc = ['clang', '-std=c23']
flags = ['-g', '-Wall', '-Wextra', '-fPIC', '-O3']
includes = [f'-I{dir}' for dir, _, _ in os.walk(INCLUDE_DIR)]
libs = ['-lpthread', '-lm']

def compile_object(source, obj):
    return cc + flags + ['-c', str(source), '-o', str(obj)] + includes

def compile_shared(files, exe):
    return cc + \
        flags + \
        [str(file[1]) for file in files] + \
        libs + \
        ['-shared', '-o', os.path.join(BUILD_DIR, exe)]

def log_cmd(cmd):
    print(f"  {' '.join(cmd)}")

sources = reduce(
    lambda acc, l: acc + l, 
    (
        [pathlib.Path(dir, file) for file in files]
        for dir, _, files in os.walk(SOURCE_DIR)
    ),
)

objects = [
    pathlib.Path(BUILD_DIR, file.name).with_suffix('.o') for file in sources
]

files = list(zip(sources, objects))

os.makedirs(BUILD_DIR, exist_ok=True)

print("Compiling objects")

processes = []
for file in files:
    cmd = compile_object(file[0], file[1])
    log_cmd(cmd)
    process = subprocess.Popen(cmd)
    processes.append(process)

success = True
for process in processes:
    process.wait()
    if process.returncode != 0:
        success = False

if not success:
    print("Error compiling objects")

print("Compiling CLAP plugin")
cmd = compile_shared(files, 'compressor.clap')
log_cmd(cmd)
process = subprocess.run(cmd)

if process.returncode != 0:
    print("Errog compiling CLAP plugin")

