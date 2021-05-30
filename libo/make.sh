#!/bin/sh
arm-none-eabi-gcc -Wall -nostdlib -Tlibo.ld libo.S -o libo.elf
arm-none-eabi-objcopy -O binary libo.elf libo.bin
