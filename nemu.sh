#!/bin/bash

make nemu
cp test/$1.out ~/NEMU/testfile
cd ~/NEMU
xxd -i testfile src/elf/testfile.c
make run
cd ~/NCC


exit 0
