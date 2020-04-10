#!/bin/bash

echo "nasm" && nasm -g -felf64 output.asm && echo "gcc" && gcc -g -no-pie -o output output.o && echo "Run" && echo && ./output
