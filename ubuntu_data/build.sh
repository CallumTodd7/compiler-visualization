#!/bin/bash

echo "nasm" && nasm -felf64 output.asm && echo "gcc" && gcc -no-pie -o output output.o && echo "Run" && echo && ./output
